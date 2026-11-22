import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, STACK, physical
from test_original_cycle_movement_update import (
    ACTIVE, ADJUSTMENT, CALLBACK_FIELD, DURATION, ELAPSED, ENABLED, MODE, OTHER_COMPONENT, PERMIT, STEP,
)

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<19I")
OUTPUT = struct.Struct("<18I")
FIRST, SECOND, CALLBACK, DIRECTION = 0x801E4000, 0x801E4100, 0x801E4200, 0x801E4300
TARGET = 0x801E4400
MODE_COPY = 0x800BE022


@unittest.skipUnless(HAS_UNICORN and ORIGINAL.is_file(), "original executable and Unicorn are required")
class OriginalCycleMovementFlowTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_setup_through_completion(self):
        rng = random.Random(0x32A2CEB)
        cases = []
        for enabled, permit, active, mode, duration, attempts in itertools.product(
            (0, 1, 2), (0, 1, 2), (0, 1), (0, 1), (0, 1, 2, 5, 0xFFFFFFFF), range(8)
        ):
            cases.append((enabled, permit, active, mode, duration, attempts,
                          *(rng.getrandbits(32) for _ in range(9))))
        cases.extend((1, 1, 0, 0, rng.randrange(1, 6), rng.randrange(8),
                      *(rng.getrandbits(32) for _ in range(9))) for _ in range(1000))
        cases = [(*case, use_target, *(rng.getrandbits(32) for _ in range(3)))
                 for case in cases for use_target in (0, 1)]
        native_results = run_native_probe(
            self, ["src/game/cycle_movement_setup.c", "src/game/func_80032A2C.c",
                   "tests/test_cycle_movement_flow.c"], cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        state = {}

        def write32(addr, value):
            cpu.mem_write(physical(addr), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(addr):
            return struct.unpack("<I", cpu.mem_read(physical(addr), 4))[0]

        def write8(addr, value):
            cpu.mem_write(physical(addr), bytes((value & 0xFF,)))

        def read8(addr):
            return cpu.mem_read(physical(addr), 1)[0]

        def words(addr, count):
            return struct.unpack(f"<{count}I", cpu.mem_read(physical(addr), 4 * count))

        def intercept(uc, address, size, data):
            a0, a1, a2, a3 = (uc.reg_read(reg) & 0xFFFFFFFF
                              for reg in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3))
            if address == 0x800516F8:
                write32(a0, FIRST)
            elif address == 0x80054B20:
                self.assertEqual(a1, 0)
                self.assertIn(a0, (FIRST, SECOND))
                cpu.mem_write(physical(a2), struct.pack("<3I", *state["first" if a0 == FIRST else "second"]))
            elif address == 0x80054AF0:
                self.assertEqual(a0, FIRST)
                cpu.mem_write(physical(a1), struct.pack("<3I", 7, 8, 9))
            elif address == 0x80054C7C:
                self.assertEqual(a1, 0)
                self.assertIn(a0, (FIRST, SECOND))
                state["first" if a0 == FIRST else "second"] = words(a2, 3)
            elif address == 0x800550A4:
                self.assertEqual((a0, a1), (FIRST, SECOND))
            elif address == 0x8002E858:
                self.assertEqual((a0, a1, a2), (DIRECTION, 321, 654))
                cpu.mem_write(physical(a3), struct.pack("<3I", *state["destination"]))
            elif address == CALLBACK:
                state["callbacks"] += 1
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            enabled, permit, active, mode, duration, attempts = entry[:6]
            vectors, use_target, target = entry[6:15], entry[15], entry[16:19]
            state.update(first=vectors[:3], second=vectors[3:6], destination=vectors[6:9], callbacks=0)
            for addr, value in ((ENABLED, enabled), (PERMIT, permit), (ACTIVE, active),
                                (MODE, mode), (MODE_COPY, 9)):
                write8(addr, value)
            for addr, value in ((DURATION, 0x11111111), (ELAPSED, 0x22222222),
                                (CALLBACK_FIELD, 0), (OTHER_COMPONENT, SECOND)):
                write32(addr, value)
            for i in range(3):
                write32(STEP + 4 * i, 0x33333333 + i)
                write32(ADJUSTMENT + 4 * i, 0x44444444 + i)
            cpu.mem_write(physical(DIRECTION), struct.pack("<4I", 0x1234, 0x5678, 0x9ABC, 0))
            cpu.mem_write(physical(TARGET), struct.pack("<4I", *target, 0))
            for offset, value in ((0x10, 0), (0x14, CALLBACK),
                                  (0x18, TARGET if use_target else 0), (0x1C, 0)):
                write32(STACK + offset, value)
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            for reg, value in ((UC_MIPS_REG_A0, DIRECTION), (UC_MIPS_REG_A1, 321),
                               (UC_MIPS_REG_A2, 654), (UC_MIPS_REG_A3, duration)):
                cpu.reg_write(reg, value)
            cpu.emu_start(0x8002EB70, RETURN, count=2000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN, f"setup case {index} did not return")
            returns = 0
            for i in range(attempts):
                cpu.reg_write(UC_MIPS_REG_SP, STACK)
                cpu.reg_write(UC_MIPS_REG_RA, RETURN)
                cpu.reg_write(UC_MIPS_REG_A0, 42)
                cpu.emu_start(0x80032A2C, RETURN, count=500)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN, f"update case {index} did not return")
                returns |= (cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF) << i
            original = (*state["first"], *state["second"], read32(DURATION), read32(ELAPSED),
                        read8(ACTIVE), read8(MODE), *words(STEP, 3), *words(ADJUSTMENT, 3),
                        returns, state["callbacks"])
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
