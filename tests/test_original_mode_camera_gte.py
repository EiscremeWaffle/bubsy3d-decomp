import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_PC, UC_MIPS_REG_RA,
    )


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalCameraGteTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_background_register_writes_against_original(self):
        edges = (0, 1, 0xF, 0x10, 0xFFF, 0x7FFFFFFF, 0x80000000, 0xFFFFFFF0, 0xFFFFFFFF)
        cases = [
            (pipeline, first, second, third)
            for pipeline in (0, 1)
            for first in edges
            for second in edges
            for third in (0, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        ]
        generator = random.Random(0x77874)
        cases.extend(
            (pipeline, *(generator.getrandbits(32) for _ in range(3)))
            for pipeline in (0, 1) for _ in range(1000)
        )
        native = run_native_probe(
            self,
            ["src/game/func_80077874.c", "src/game/func_80072DF0.c",
             "src/game/func_8004EF10.c", "tests/test_mode_camera_gte.c"],
            cases, struct.Struct("<4I"), struct.Struct("<3I"),
        )
        registers = (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2)

        for index, (entry, expected) in enumerate(zip(cases, native)):
            pipeline, *values = entry
            with self.subTest(index=index, input=entry):
                cpu = self.fresh_cpu()
                writes = []

                def capture(uc, address, size, data):
                    if address in (0x80077880, 0x80077884, 0x80077888):
                        offset = (address - 0x80077880) // 4
                        writes.append((13 + offset, uc.reg_read(registers[offset])))
                        uc.reg_write(UC_MIPS_REG_PC, address + 4)

                cpu.hook_add(UC_HOOK_CODE, capture)
                for register, value in zip(registers, values):
                    cpu.reg_write(register, value)
                cpu.reg_write(UC_MIPS_REG_RA, RETURN)
                cpu.emu_start(0x80072DF0 if pipeline else 0x80077874, RETURN, count=100)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
                self.assertEqual([register for register, _ in writes], [13, 14, 15])
                self.assertEqual(tuple(value for _, value in writes), expected)
