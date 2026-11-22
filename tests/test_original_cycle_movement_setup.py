import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<19I")
OUTPUT = struct.Struct("<17I")
DIRECTION, COMPONENT, CALLBACK, TARGET_COMPONENT = 0x801E4000, 0x801E4100, 0x801E4200, 0x801E4300
OVERRIDE = 0x801E4400
TARGET_COMPONENT_POINTER = 0x800BE528
ENABLED, PERMIT, MODE_COPY = 0x800BE024, 0x800BE020, 0x800BE022
ACTIVE, MODE, DURATION, ELAPSED = 0x80186108, 0x80186109, 0x80186110, 0x80186114
STEP, ADJUSTMENT, CALLBACK_FIELD = 0x80186118, 0x80186148, 0x80186144


@unittest.skipUnless(HAS_UNICORN and ORIGINAL.is_file(), "original executable and Unicorn are required")
class OriginalCycleMovementSetupTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_cycle_movement_setup(self):
        self.compare_setup(real_target=False)

    def test_cycle_movement_setup_with_real_target(self):
        self.compare_setup(real_target=True)

    def compare_setup(self, real_target):
        rng = random.Random(0x2EB70)
        cases = []
        for enabled, permit, active, mode, duration in itertools.product(
            (0, 1, 2), (0, 1, 2), (0, 1, 2), (0, 1, 2),
            (0, 1, 2, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF),
        ):
            cases.append((enabled, permit, active, mode, duration,
                          *(rng.getrandbits(32) for _ in range(6)), rng.randrange(4)))
        for delta, duration in itertools.product(
            (0, 1, 2, 3, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFD, 0xFFFFFFFE, 0xFFFFFFFF),
            (1, 2, 3, 0x7FFFFFFF),
        ):
            cases.append((1, 1, 0, 0, duration, 0, 0, 0, delta, delta, delta, 0))
        cases.extend((1, 1, rng.choice((0, 1, 2)), rng.choice((0, 1, 2)),
                      rng.randrange(1, 0x80000000), *(rng.getrandbits(32) for _ in range(6)),
                      rng.randrange(4))
                     for _ in range(1000))
        cases = [(*case, use_target, *(rng.getrandbits(32) for _ in range(6)))
                 for case in cases for use_target in (0, 1)]
        for delta, duration in itertools.product(
            (0, 1, 2, 3, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFD, 0xFFFFFFFE, 0xFFFFFFFF),
            (1, 2, 3, 0x7FFFFFFF),
        ):
            cases.append((1, 1, 0, 0, duration, 0, 0, 0, 0, 0, 0, 0,
                          1, 0, 0, 0, delta, delta, delta))
        sources = ["src/game/cycle_movement_setup.c", "tests/test_cycle_movement_setup.c"]
        flags = []
        if real_target:
            sources.append("src/game/grounded_transform.c")
            flags.append("-DCYCLE_REAL_TARGET")
        native_results = run_native_probe(self, sources, cases, INPUT, OUTPUT, compiler_flags=flags)
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

        def mutate_current():
            if state["mutations"] & 1:
                address = state["current_pointer"]
                write32(address, read32(address) ^ 0x80000000)
            if state["mutations"] & 2:
                write32(OVERRIDE + 4, read32(OVERRIDE + 4) ^ 0x40000000)

        def intercept(uc, address, size, data):
            a0, a1, a2, a3 = (uc.reg_read(reg) & 0xFFFFFFFF
                              for reg in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3))
            if address == 0x800516F8:
                state["calls"] = state["calls"] * 10 + 1
                write32(a0, COMPONENT)
            elif address == 0x80054B20:
                self.assertEqual(a1, 0)
                if a0 == TARGET_COMPONENT:
                    state["calls"] = state["calls"] * 10 + 3
                    cpu.mem_write(physical(a2), bytes(cpu.mem_read(physical(TARGET_COMPONENT), 12)))
                else:
                    self.assertEqual(a0, COMPONENT)
                    state["calls"] = state["calls"] * 10 + 2
                    state["current_pointer"] = a2
                    cpu.mem_write(physical(a2), struct.pack("<3I", *state["current"]))
            elif address == 0x8002E858:
                if real_target:
                    return
                self.assertEqual((a0, a1, a2), (DIRECTION, 321, 654))
                state["calls"] = state["calls"] * 10 + 3
                mutate_current()
                cpu.mem_write(physical(a3), struct.pack("<3I", *state["destination"]))
            elif address == 0x80053D00 and real_target:
                self.assertEqual((a0, a1), (DIRECTION, 321))
                state["calls"] = state["calls"] * 10 + 4
                mutate_current()
                cpu.mem_write(physical(a2), struct.pack("<3I", *state["destination"]))
            elif address == 0x80054C7C:
                self.assertEqual((a0, a1), (TARGET_COMPONENT, 0))
                state["calls"] = state["calls"] * 10 + 5
                cpu.mem_write(physical(TARGET_COMPONENT), bytes(cpu.mem_read(physical(a2), 12)))
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            enabled, permit, active, mode, duration = entry[:5]
            vectors, mutations, use_target = entry[5:11], entry[11], entry[12]
            target_position, override_position = entry[13:16], entry[16:19]
            state.update(calls=0, current=vectors[:3], destination=vectors[3:], mutations=mutations)
            for addr, value in ((ENABLED, enabled), (PERMIT, permit), (ACTIVE, active), (MODE, mode), (MODE_COPY, 9)):
                write8(addr, value)
            for addr, value in ((DURATION, 0x11111111), (ELAPSED, 0x22222222), (CALLBACK_FIELD, 0)):
                write32(addr, value)
            write32(TARGET_COMPONENT_POINTER, TARGET_COMPONENT)
            cpu.mem_write(physical(TARGET_COMPONENT), struct.pack("<3I", *target_position))
            cpu.mem_write(physical(OVERRIDE), struct.pack("<4I", *override_position, 0))
            for i in range(3):
                write32(STEP + 4 * i, 0x33333333 + i)
                write32(ADJUSTMENT + 4 * i, 0x44444444 + i)
            cpu.mem_write(physical(DIRECTION), struct.pack("<4I", 0x1234, 0x5678, 0x9ABC, 0))
            for offset, value in ((0x10, 0), (0x14, CALLBACK),
                                  (0x18, OVERRIDE if use_target else 0), (0x1C, 0)):
                write32(STACK + offset, value)
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            for reg, value in ((UC_MIPS_REG_A0, DIRECTION), (UC_MIPS_REG_A1, 321),
                               (UC_MIPS_REG_A2, 654), (UC_MIPS_REG_A3, duration)):
                cpu.reg_write(reg, value)
            cpu.emu_start(0x8002EB70, RETURN, count=2000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN, f"case {index} did not return")
            original = (*(read32(STEP + 4 * i) for i in range(3)),
                        *(read32(ADJUSTMENT + 4 * i) for i in range(3)),
                        read32(DURATION), read32(ELAPSED), read8(ACTIVE), read8(MODE),
                        read8(PERMIT), read8(MODE_COPY), int(read32(CALLBACK_FIELD) == CALLBACK), state["calls"],
                        *(read32(TARGET_COMPONENT + 4 * i) for i in range(3)))
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
