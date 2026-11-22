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


INPUT = struct.Struct("<9I")
OUTPUT = struct.Struct("<29I")
COMPONENT = 0x801E0000
BASIS = 0x801E0100
TRANSLATION = 0x801E0200
INITIAL_COMPONENT = (0x12345678, 0x9ABCDEF0, 0x1234ABCD, 0x12344321, 0xBEEFCAFE, 1, 2, 3)


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalComponentTargetOrientationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_target_orientation_against_original(self):
        generator = random.Random(0x5536C)
        cases = [
            (null_component, null_translation, null_basis, mutations,
             *(generator.getrandbits(32) for _ in range(3)), first_status, second_status)
            for null_component, null_translation, null_basis, mutations, first_status, second_status
            in itertools.product((0, 1), (0, 1), (0, 1), range(4), (0, 0xFFFFFFFF), (0, 7))
        ]
        native_outputs = run_native_probe(
            self, ["src/game/func_8005536C.c", "tests/test_component_target_orientation.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            if address == 0x80054C7C:
                self.assertEqual(current["calls"], 0)
                self.assertEqual(a1, 0 if current["null_basis"] else BASIS)
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A2), TRANSLATION)
                current["calls"] += 1
                current["target"] = a0
                current["initial_target"] = struct.unpack("<8I", uc.mem_read(physical(a0), 32))
                current["translation"] = struct.unpack("<3I", uc.mem_read(physical(TRANSLATION), 12))
                uc.mem_write(physical(a0 + 20), struct.pack("<3I", *current["translation"]))
                if current["mutations"] & 1:
                    value = struct.unpack("<H", uc.mem_read(physical(COMPONENT), 2))[0]
                    uc.mem_write(physical(COMPONENT), struct.pack("<H", value ^ 0x1111))
                uc.reg_write(UC_MIPS_REG_V0, current["first_status"])
            elif address == 0x800550A4:
                self.assertEqual((current["calls"], a0, a1), (1, COMPONENT, current["target"]))
                current["calls"] += 1
                current["final_target"] = struct.unpack("<8I", uc.mem_read(physical(a1), 32))
                if current["mutations"] & 2:
                    value = struct.unpack("<H", uc.mem_read(physical(COMPONENT + 2), 2))[0]
                    uc.mem_write(physical(COMPONENT + 2), struct.pack("<H", value ^ 0x2222))
                uc.reg_write(UC_MIPS_REG_V0, current["second_status"])
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            null_component, null_translation, null_basis, mutations, *values = entry
            translation, first_status, second_status = values[:3], values[3], values[4]
            current.update(
                null_basis=null_basis, mutations=mutations, first_status=first_status,
                second_status=second_status, calls=0, initial_target=[0] * 8,
                translation=[0] * 3, final_target=[0] * 8,
            )
            cpu.mem_write(physical(COMPONENT), struct.pack("<8I", *INITIAL_COMPONENT))
            cpu.mem_write(physical(BASIS), bytes([0xA5]) * 32)
            cpu.mem_write(physical(TRANSLATION), struct.pack("<3I", *translation))
            cpu.reg_write(UC_MIPS_REG_A0, 0 if null_component else COMPONENT)
            cpu.reg_write(UC_MIPS_REG_A1, 0 if null_basis else BASIS)
            cpu.reg_write(UC_MIPS_REG_A2, 0 if null_translation else TRANSLATION)
            self.call(cpu, 0x8005536C)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, current["calls"],
                *current["initial_target"], *current["translation"], *current["final_target"],
                *struct.unpack("<8I", cpu.mem_read(physical(COMPONENT), 32)),
            )
            self.assertEqual(native, original, f"case {index}: {entry}")
