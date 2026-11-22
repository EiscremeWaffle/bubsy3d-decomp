import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, RETURN, STACK, physical
from test_original_cycle_movement_update import (
    ACTIVE, ADJUSTMENT, CALLBACK, CALLBACK_FIELD, DURATION, ELAPSED, ENABLED,
    FIRST, MODE, OTHER_COMPONENT, PERMIT, SECOND, STEP,
)

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_PC, UC_MIPS_REG_RA,
        UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<18I")
OUTPUT = struct.Struct("<12I")


@unittest.skipUnless(HAS_UNICORN and ORIGINAL.is_file(), "original executable and Unicorn are required")
class OriginalCycleMovementComponentTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_update_with_original_component_helpers(self):
        rng = random.Random(0x54B20)
        cases = []
        for enabled, permit, active, equal in itertools.product((0, 1, 2), (0, 1, 2), (0, 1, 2), (0, 1)):
            duration = rng.getrandbits(32)
            elapsed = duration if equal else (duration + 1) & 0xFFFFFFFF
            cases.append((enabled, permit, active, rng.randrange(3), duration, elapsed,
                          *(rng.getrandbits(32) for _ in range(12))))
        for value in (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF):
            cases.append((1, 1, 1, 0, 2, 0, *([value] * 12)))
        cases.extend((1, 1, 1, rng.randrange(3), rng.getrandbits(32), rng.getrandbits(32),
                      *(rng.getrandbits(32) for _ in range(12))) for _ in range(1500))
        sources = [
            "src/game/func_80032A2C.c", "src/game/func_800516F8.c", "src/game/func_80054AF0.c",
            "src/game/func_80054B20.c", "src/game/func_80054C7C.c",
            "tests/test_cycle_movement_components.c",
        ]
        native_results = run_native_probe(self, sources, cases, INPUT, OUTPUT)
        cpu = self.fresh_cpu()
        state = {}

        def write32(addr, value):
            cpu.mem_write(physical(addr), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(addr):
            return struct.unpack("<I", cpu.mem_read(physical(addr), 4))[0]

        def read8(addr):
            return cpu.mem_read(physical(addr), 1)[0]

        def intercept(uc, address, size, data):
            if address == 0x800550A4:
                self.assertEqual((uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)), (FIRST, SECOND))
                state["calls"] += 1
            elif address == CALLBACK:
                state["calls"] += 2
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            enabled, permit, active, mode, duration, elapsed, *vectors = entry
            state["calls"] = 0
            for addr, value in ((ENABLED, enabled), (PERMIT, permit), (ACTIVE, active), (MODE, mode)):
                cpu.mem_write(physical(addr), bytes((value,)))
            for addr, value in ((DURATION, duration), (ELAPSED, elapsed), (CALLBACK_FIELD, CALLBACK),
                                (OTHER_COMPONENT, SECOND), (GP + 0x888, FIRST)):
                write32(addr, value)
            for address in (FIRST, SECOND):
                cpu.mem_write(physical(address), bytes(32))
            for i in range(3):
                write32(FIRST + 20 + i * 4, vectors[i])
                write32(SECOND + 20 + i * 4, vectors[3 + i])
                write32(STEP + i * 4, vectors[6 + i])
                write32(ADJUSTMENT + i * 4, vectors[9 + i])
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.reg_write(UC_MIPS_REG_A0, 42)
            cpu.emu_start(0x80032A2C, RETURN, count=2000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN, f"case {index} did not return")
            original = (cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, read32(DURATION), read32(ELAPSED),
                        read8(ACTIVE), read8(MODE), *(read32(FIRST + 20 + 4 * i) for i in range(3)),
                        *(read32(SECOND + 20 + 4 * i) for i in range(3)), state["calls"])
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
