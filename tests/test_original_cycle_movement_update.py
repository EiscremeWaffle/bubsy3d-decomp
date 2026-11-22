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
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<20I")
OUTPUT = struct.Struct("<18I")
FIRST, SECOND, CALLBACK, THIRD = 0x801E4000, 0x801E4100, 0x801E4200, 0x801E4300
ENABLED, PERMIT = 0x800BE024, 0x800BE020
ACTIVE, MODE, DURATION, ELAPSED = 0x80186108, 0x80186109, 0x80186110, 0x80186114
STEP, ADJUSTMENT, CALLBACK_FIELD, OTHER_COMPONENT = 0x80186118, 0x80186148, 0x80186144, 0x800BE528


@unittest.skipUnless(HAS_UNICORN and ORIGINAL.is_file(), "original executable and Unicorn are required")
class OriginalCycleMovementUpdateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_update_against_original(self):
        rng = random.Random(0x32A2C)
        cases = []
        for enabled, permit, active, equal, callback, mutations in itertools.product(
            (0, 1, 2), (0, 1, 2), (0, 1, 2), (0, 1), (0, 1), range(32)
        ):
            duration = rng.getrandbits(32)
            elapsed = duration if equal else (duration + 1) & 0xFFFFFFFF
            cases.append((enabled, permit, active, rng.randrange(3), duration, elapsed, callback,
                          mutations, *(rng.getrandbits(32) for _ in range(12))))
        cases.extend((1, 1, 1, rng.randrange(3), rng.getrandbits(32), rng.getrandbits(32),
                      rng.randrange(2), rng.randrange(32), *(rng.getrandbits(32) for _ in range(12)))
                     for _ in range(1200))
        native_results = run_native_probe(
            self, ["src/game/func_80032A2C.c", "tests/test_cycle_movement_update.c"],
            cases, INPUT, OUTPUT,
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

        def record(call):
            state["calls"] = state["calls"] * 10 + call

        def intercept(uc, address, size, data):
            a0, a1, a2 = (uc.reg_read(reg) & 0xFFFFFFFF
                          for reg in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2))
            if address == 0x800516F8:
                record(1)
                write32(a0, FIRST)
            elif address == 0x80054B20:
                self.assertEqual(a1, 0)
                if a0 == FIRST:
                    record(2)
                    cpu.mem_write(physical(a2), struct.pack("<3I", *state["first_input"]))
                else:
                    self.assertEqual(a0, read32(OTHER_COMPONENT))
                    record(5)
                    cpu.mem_write(physical(a2), struct.pack("<3I", *state["second_input"]))
            elif address == 0x80054AF0:
                self.assertEqual(a0, FIRST)
                record(3)
                cpu.mem_write(physical(a1), struct.pack("<3I", 7, 8, 9))
                if state["mutations"] & 1:
                    write32(STEP, read32(STEP) ^ 0x80000000)
            elif address == 0x80054C7C:
                self.assertEqual(a1, 0)
                if a0 == FIRST:
                    record(4)
                    state["first_output"] = words(a2, 3)
                    if state["mutations"] & 2:
                        write32(ADJUSTMENT + 4, read32(ADJUSTMENT + 4) ^ 0x40000000)
                    if state["mutations"] & 16:
                        write32(OTHER_COMPONENT, THIRD)
                else:
                    self.assertEqual(a0, read32(OTHER_COMPONENT))
                    record(6)
                    state["second_output"] = words(a2, 3)
            elif address == 0x800550A4:
                self.assertEqual((a0, a1), (FIRST, read32(OTHER_COMPONENT)))
                record(7)
                if state["mutations"] & 8:
                    write32(ELAPSED, read32(ELAPSED) ^ 0x80000000)
            elif address == CALLBACK:
                record(8)
                if state["mutations"] & 4:
                    write8(ACTIVE, 7)
                    write32(ELAPSED, 0x87654321)
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            enabled, permit, active, mode, duration, elapsed, callback, mutations, *vectors = entry
            state.update(calls=0, mutations=mutations, first_input=vectors[:3], second_input=vectors[3:6],
                         first_output=(0, 0, 0), second_output=(0, 0, 0))
            for addr, value in ((ENABLED, enabled), (PERMIT, permit), (ACTIVE, active), (MODE, mode)):
                write8(addr, value)
            for addr, value in ((DURATION, duration), (ELAPSED, elapsed),
                                (CALLBACK_FIELD, CALLBACK if callback else 0), (OTHER_COMPONENT, SECOND)):
                write32(addr, value)
            for i in range(3):
                write32(STEP + 4 * i, vectors[6 + i])
                write32(ADJUSTMENT + 4 * i, vectors[9 + i])
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.reg_write(UC_MIPS_REG_A0, 42)
            cpu.emu_start(0x80032A2C, RETURN, count=500)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN, f"case {index} did not return")
            original = (cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, read32(DURATION), read32(ELAPSED),
                        read8(ACTIVE), read8(MODE), *words(STEP, 3), *words(ADJUSTMENT, 3),
                        *state["first_output"], *state["second_output"], state["calls"])
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
