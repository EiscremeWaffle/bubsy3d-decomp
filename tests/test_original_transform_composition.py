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


INPUT = struct.Struct("<28I")
OUTPUT = struct.Struct("<19I")
BUFFER = 0x801E0000


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalTransformCompositionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_transform_composition_against_original(self):
        generator = random.Random(0x7242C)
        cases = []
        for variant, alias, mutations in itertools.product(range(2), range(2), range(32)):
            for _ in range(24):
                cases.append((variant, alias, mutations, *(generator.getrandbits(32) for _ in range(25))))
        native_outputs = run_native_probe(
            self, [
                "src/game/func_8007242C.c", "src/game/func_800724B0.c",
                "tests/test_transform_composition.c",
            ],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def replace(uc, address, values, mask):
            uc.mem_write(physical(address + 20), struct.pack("<3I", *(value ^ mask for value in values)))

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            if address == 0x80077298:
                a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
                self.assertEqual((a0, a1), (current["first"], current["second"] + 20))
                self.assertEqual(current["calls"], 0)
                current["calls"] += 1
                current["observed"] = struct.unpack("<3I", uc.mem_read(physical(a1), 12))
                if current["mutations"] & 1:
                    replace(uc, current["first"], current["first_replacement"], 0)
                if current["mutations"] & 2:
                    replace(uc, current["second"], current["second_replacement"], 0)
                uc.mem_write(physical(a2), struct.pack("<3I", *current["transformed"]))
                current["temporary"] = a2
                uc.reg_write(UC_MIPS_REG_V0, a2)
            elif address == (0x80076890 if current["variant"] else 0x8007699C):
                self.assertEqual((a0, a1), (current["first"], current["second"]))
                self.assertEqual(current["calls"], 1)
                current["calls"] += 1
                destination = a0 if current["variant"] else a1
                uc.mem_write(physical(destination), struct.pack("<H", 0xBEEF))
                if current["mutations"] & 4:
                    replace(uc, current["first"], current["first_replacement"], 0x5A5A5A5A)
                if current["mutations"] & 8:
                    replace(uc, current["second"], current["second_replacement"], 0xA5A5A5A5)
                if current["mutations"] & 16:
                    values = struct.unpack("<3I", uc.mem_read(physical(current["temporary"]), 12))
                    uc.mem_write(physical(current["temporary"]),
                                 struct.pack("<3I", *(value ^ 0xFFFFFFFF for value in values)))
                uc.reg_write(UC_MIPS_REG_V0, destination)
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            variant, alias, mutations = entry[:3]
            transformed, first_replacement, second_replacement = entry[3:6], entry[6:9], entry[9:12]
            current.update(
                variant=variant, first=BUFFER, second=BUFFER if alias else BUFFER + 32,
                mutations=mutations,
                transformed=transformed, first_replacement=first_replacement,
                second_replacement=second_replacement, calls=0,
            )
            cpu.mem_write(physical(BUFFER), struct.pack("<16I", *entry[12:]))
            cpu.reg_write(UC_MIPS_REG_A0, current["first"])
            cpu.reg_write(UC_MIPS_REG_A1, current["second"])
            self.call(cpu, 0x800724B0 if variant else 0x8007242C)
            self.assertEqual(current["calls"], 2)
            original = (*current["observed"], *struct.unpack("<16I", cpu.mem_read(physical(BUFFER), 64)))
            with self.subTest(index=index, variant=variant, alias=alias, mutations=mutations):
                self.assertEqual(native, original)
