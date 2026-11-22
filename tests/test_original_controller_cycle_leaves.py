import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, STACK, physical

if HAS_UNICORN:
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<12I")
OUTPUT = struct.Struct("<11I")
ACTOR, SOURCE = 0x801E4000, 0x801E4100
LEVEL_PARAMETERS, EVENT_PARAMETERS = 0x801D8958, 0x801D05C4


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerCycleLeavesTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_leaves_against_original(self):
        rng = random.Random(0x44C04)
        cases = []
        for flags, selector, index in itertools.product(
            (0, 0xFFFFFFFF, 0x80000000, 0x55555555), (0, 1, 31, 32, 255, 256, 511), range(8)
        ):
            cases.append((flags, selector, *(rng.getrandbits(32) for _ in range(8)), index, rng.getrandbits(32)))
        cases.extend((rng.getrandbits(32), rng.getrandbits(32),
                      *(rng.getrandbits(32) for _ in range(8)), rng.randrange(8), rng.getrandbits(32))
                     for _ in range(2000))
        sources = ["src/game/func_800226C0.c", "src/game/func_80028E80.c",
                   "src/game/controller_cycle_event_parameters.c", "src/game/func_80044C34.c",
                   "src/game/func_8005668C.c", "src/game/func_80056678.c", "tests/test_controller_cycle_leaves.c"]
        native_results = run_native_probe(self, sources, cases, INPUT, OUTPUT)
        cpu = self.fresh_cpu()

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def call(address, a0=0, a1=0):
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.reg_write(UC_MIPS_REG_A0, a0)
            cpu.reg_write(UC_MIPS_REG_A1, a1)
            cpu.emu_start(address, RETURN, count=200)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
            return cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF

        for index, (entry, native) in enumerate(zip(cases, native_results)):
            write32(ACTOR + 4, entry[0])
            cpu.mem_write(physical(SOURCE), struct.pack("<8I", *entry[2:10]))
            cpu.mem_write(physical(EVENT_PARAMETERS), bytes([0xA5]) * 32)
            write32(LEVEL_PARAMETERS, SOURCE)
            result = call(0x800226C0, ACTOR, entry[1])
            flags = read32(ACTOR + 4)
            call(0x80028E80)
            call(0x80044C04, entry[10], entry[11])
            readback = call(0x80044C1C, entry[10])
            original = (flags, result, readback,
                        *struct.unpack("<8I", cpu.mem_read(physical(EVENT_PARAMETERS), 32)))
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
