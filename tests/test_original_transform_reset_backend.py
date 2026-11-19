import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_PC,
        UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<45I")
OUTPUT = struct.Struct("<32I20H")
DESCRIPTOR = 0x801E0000
BINDING = 0x801E1000
WORKING = 0x801D58F0
RENDER = 0x801D3414


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalTransformResetBackendTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_reset_backend_against_original(self):
        generator = random.Random(0x731C0)
        cases = [
            (binding, mutations, *(generator.getrandbits(32) for _ in range(43)))
            for binding, mutations in itertools.product(range(2), range(8))
            for _ in range(64)
        ]
        for mutations, transformed in itertools.product(
            range(8), itertools.product((0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF), repeat=3)
        ):
            words = [generator.getrandbits(32) for _ in range(43)]
            words[32:35] = transformed
            cases.append((1, mutations, *words))
        native_outputs = run_native_probe(
            self, [
                "src/game/func_800731C0.c", "src/game/func_80077C00.c",
                "tests/test_transform_reset_backend.c",
            ], cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def mutate_word(uc, address, mask):
            value, = struct.unpack("<I", uc.mem_read(physical(address), 4))
            uc.mem_write(physical(address), struct.pack("<I", value ^ mask))

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            if address == 0x80073324:
                self.assertEqual((current["calls"], a0), (0, BINDING))
                current["calls"] += 1
                uc.mem_write(physical(a1), struct.pack("<8I", *current["hierarchy"]))
                if current["mutations"] & 1:
                    mutate_word(uc, WORKING + 28, 0x11111111)
                    mutate_word(uc, RENDER, 0x22222222)
            elif address == 0x80077298:
                a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
                self.assertEqual(current["calls"], 1)
                current["calls"] += 1
                current["basis_before"] = (*struct.unpack("<9H", uc.mem_read(physical(a0), 18)), 0)
                current["observed_input"] = struct.unpack("<3I", uc.mem_read(physical(a1), 12))
                if current["mutations"] & 2:
                    value, = struct.unpack("<H", uc.mem_read(physical(a0), 2))
                    uc.mem_write(physical(a0), struct.pack("<H", value ^ 1))
                    mutate_word(uc, WORKING + 4, 0x33333333)
                uc.mem_write(physical(a2), struct.pack("<3I", *current["transformed"]))
                uc.reg_write(UC_MIPS_REG_V0, a2)
            elif address == 0x8007242C:
                self.assertEqual((current["calls"], a0), (2, WORKING))
                current["calls"] += 1
                current["basis_after"] = (*struct.unpack("<9H", uc.mem_read(physical(a1), 18)), 0)
                current["observed_inverse"] = struct.unpack("<3I", uc.mem_read(physical(a1 + 20), 12))
                current["working_at_composition"] = struct.unpack("<8I", uc.mem_read(physical(WORKING), 32))
                if current["mutations"] & 4:
                    mutate_word(uc, WORKING, 0x44444444)
                    mutate_word(uc, RENDER + 4, 0x55555555)
                uc.mem_write(physical(a1), struct.pack("<8I", *current["composed"]))
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            binding, mutations = entry[:2]
            descriptor, working, render = entry[2:10], entry[10:18], entry[18:26]
            hierarchy, transformed, composed = entry[26:34], entry[34:37], entry[37:45]
            current.update(
                calls=0, mutations=mutations, hierarchy=hierarchy,
                transformed=transformed, composed=composed,
                observed_input=(0,) * 3, observed_inverse=(0,) * 3,
                working_at_composition=(0,) * 8, basis_before=(0,) * 10, basis_after=(0,) * 10,
            )
            cpu.mem_write(physical(DESCRIPTOR), struct.pack("<8II", *descriptor, BINDING if binding else 0))
            cpu.mem_write(physical(WORKING), struct.pack("<8I", *working))
            cpu.mem_write(physical(RENDER), struct.pack("<8I", *render))
            cpu.reg_write(UC_MIPS_REG_A0, DESCRIPTOR)
            self.call(cpu, 0x800731C0)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, current["calls"],
                *current["observed_input"], *current["observed_inverse"],
                *current["working_at_composition"],
                *struct.unpack("<8I", cpu.mem_read(physical(WORKING), 32)),
                *struct.unpack("<8I", cpu.mem_read(physical(RENDER), 32)),
                *current["basis_before"], *current["basis_after"],
            )
            with self.subTest(index=index, binding=binding, mutations=mutations):
                self.assertEqual(native, original)
