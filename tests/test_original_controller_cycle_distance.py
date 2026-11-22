import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, physical

if HAS_UNICORN:
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0

INPUT = struct.Struct("<8I")
OUTPUT = struct.Struct("<I")
LEFT, RIGHT = 0x801E4000, 0x801E4100


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerCycleDistanceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_distance_against_original(self):
        rng = random.Random(0x22B5C)
        edge = (0, 1, 2, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        cases = [(a, rng.getrandbits(32), b, rng.getrandbits(32),
                  c, rng.getrandbits(32), d, rng.getrandbits(32))
                 for a, b, c, d in itertools.product(edge, repeat=4)]
        for x, z in itertools.product(range(0, 20), repeat=2):
            cases.append((x, 0, z, 0, 0, 0xFFFFFFFF, 0, 0))
        cases.extend(tuple(rng.getrandbits(32) for _ in range(8)) for _ in range(5000))
        native_results = run_native_probe(
            self, ["src/game/func_80022B5C.c", "tests/test_controller_cycle_distance.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            cpu.mem_write(physical(LEFT), struct.pack("<4I", *entry[:4]))
            cpu.mem_write(physical(RIGHT), struct.pack("<4I", *entry[4:]))
            cpu.reg_write(UC_MIPS_REG_A0, LEFT)
            cpu.reg_write(UC_MIPS_REG_A1, RIGHT)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.emu_start(0x80022B5C, RETURN, count=100)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN, f"case {index} did not return")
            self.assertEqual(native[0], cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                             f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
