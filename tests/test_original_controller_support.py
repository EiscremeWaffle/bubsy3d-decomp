import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical, write_word

if HAS_UNICORN:
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_V0


INPUT = struct.Struct("<8I")
OUTPUT = struct.Struct("<6I")
SELECTION_POINTER = 0x800BE20C
SELECTION_SOURCE = 0x801E0100
SCREEN_ORIGIN = 0x801D33C0
POSITION = 0x801E0200


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerSupportTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_controller_support_against_original(self):
        values = (0, 1, 0x7FFF, 0x8000, 0xFFFF, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        cases = [
            (0, 0, selection, 0, 0, 0, 0, 0x12345678)
            for selection in values
        ]
        cases.extend(
            (1, null_position, 0, origin, origin ^ 0xFFFF, x, x ^ 0xFFFFFFFF, 0x12345678)
            for null_position, origin, x in itertools.product((0, 1), values, values)
        )
        generator = random.Random(0x4753C55D94)
        for _ in range(1000):
            cases.append((
                generator.randrange(2), generator.randrange(2),
                *(generator.getrandbits(32) for _ in range(6)),
            ))
        native_outputs = run_native_probe(self, [
            "src/game/func_8004753C.c", "src/game/func_80055D94.c",
            "tests/test_controller_support.c",
        ], cases, INPUT, OUTPUT)
        for case_index, (entry, native) in enumerate(zip(cases, native_outputs)):
            operation, null_position, selection, origin_x, origin_y, x, y, marker = entry
            with self.subTest(index=case_index, input=entry):
                cpu = self.fresh_cpu()
                write_word(cpu, SELECTION_POINTER, SELECTION_SOURCE)
                write_word(cpu, SELECTION_SOURCE + 0x0C, selection)
                cpu.mem_write(physical(SCREEN_ORIGIN), struct.pack("<HH", origin_x & 0xFFFF, origin_y & 0xFFFF))
                cpu.mem_write(physical(POSITION), struct.pack("<IHH", marker, marker & 0xFFFF, marker >> 16))
                if operation == 0:
                    self.call(cpu, 0x8004753C)
                else:
                    cpu.reg_write(UC_MIPS_REG_A0, 0 if null_position else POSITION)
                    cpu.reg_write(UC_MIPS_REG_A1, x)
                    cpu.reg_write(UC_MIPS_REG_A2, y)
                    self.call(cpu, 0x80055D94)
                marker_after, pos_x, pos_y = struct.unpack(
                    "<IHH", cpu.mem_read(physical(POSITION), 8)
                )
                final_origin_x, final_origin_y = struct.unpack(
                    "<HH", cpu.mem_read(physical(SCREEN_ORIGIN), 4)
                )
                original = (
                    cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                    marker_after, pos_x, pos_y, final_origin_x, final_origin_y,
                )
                self.assertEqual(native, original)
