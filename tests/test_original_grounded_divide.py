import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical

if HAS_UNICORN:
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_V0

INPUT = struct.Struct("<9I")
OUTPUT = struct.Struct("<7I")
VECTOR = 0x801E0000
OUTPUT_ADDRESS = 0x801E0100


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalGroundedDivideTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_division_and_vector_wrapper_against_original(self):
        rng = random.Random(0x53350)
        thresholds = (0, 1, 2, 0x7FFFE, 0x7FFFF, 0x80000, 0x7FFFFF, 0x800000,
                      0x7FFFFFF, 0x8000000, 0x7FFFFFFF, 0x80000000)
        boundaries = sorted(set(thresholds) | {(-value) & 0xFFFFFFFF for value in thresholds})
        cases = [
            (0, alias, scale, value, 0x11223344, 0x55667788, 0x12345678, 3, 4)
            for alias, value, scale in itertools.product(range(2), boundaries, boundaries)
        ]
        cases.extend(
            (mode, alias, *(rng.getrandbits(32) for _ in range(7)))
            for mode, alias in itertools.product(range(2), range(2))
            for _ in range(1500)
        )
        cases.extend(
            (1, alias, scale, 0, 0x7FFFFFFF, 0x80000000, 0xABCD, 0xDCBA, 0x1234)
            for alias, scale in itertools.product(range(2), (0, 1, 0xFFFFFFFF, 0x80000000))
        )
        native_results = run_native_probe(
            self, ["src/game/func_80053350.c", "src/game/func_80053B70.c",
                   "tests/test_grounded_divide.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            mode, alias, scale = entry[:3]
            values, output = entry[3:6], entry[6:9]
            cpu.mem_write(physical(VECTOR), struct.pack("<3I", *values))
            cpu.mem_write(physical(OUTPUT_ADDRESS), struct.pack("<3I", *output))
            cpu.reg_write(UC_MIPS_REG_A0, values[0] if mode == 0 else VECTOR)
            cpu.reg_write(UC_MIPS_REG_A1, scale)
            cpu.reg_write(UC_MIPS_REG_A2, VECTOR if alias else OUTPUT_ADDRESS)
            self.call(cpu, 0x80053350 if mode == 0 else 0x80053B70)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                *struct.unpack("<3I", cpu.mem_read(physical(VECTOR), 12)),
                *struct.unpack("<3I", cpu.mem_read(physical(OUTPUT_ADDRESS), 12)),
            )
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
