import itertools
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_SP,
    )


INPUT = struct.Struct("<3I")
OUTPUT = struct.Struct("<4I")
CURRENT = 0x800A5438


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerTimingWaitTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_wait_backend_against_original(self):
        values = (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        cases = [
            (position, count, current)
            for position, count, current in itertools.product(values, (0, 0x20000), values)
        ]
        cases.append((1, 1, 0))
        native = run_native_probe(
            self, ["src/game/func_8006BCCC.c", "tests/test_controller_timing_wait.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def intercept(uc, address, size, data):
            if address not in (0x80082E4C, 0x8006BB74, 0x8006BD68):
                return
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            if address == 0x80082E4C:
                self.assertEqual(a0, 0x80013EA8)
                self.assertEqual(current["calls"], 0)
                current["arguments"][0] = 1
            elif address == 0x8006BB74:
                self.assertEqual(current["calls"], 1)
                current["arguments"][1] = a0
            else:
                self.assertEqual(current["calls"], 2)
                self.assertEqual(a1, 0)
                current["arguments"][2] = a0
            current["calls"] += 1
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native_result) in enumerate(zip(cases, native)):
            position, count, value = entry
            current.update(calls=0, arguments=[0, 0, 0])
            cpu.mem_write(physical(CURRENT), struct.pack("<I", value))
            cpu.reg_write(UC_MIPS_REG_A0, position)
            cpu.reg_write(UC_MIPS_REG_A1, count)
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.emu_start(0x8006BCCC, RETURN, count=1000000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
            original = (current["calls"], *current["arguments"])
            self.assertEqual(native_result, original, f"case {index}: {entry}")

    def test_wait_observes_counter_changes(self):
        cpu = self.fresh_cpu()
        visits = []

        def intercept(uc, address, size, data):
            if address == 0x8006BD40:
                visits.append(address)
                if len(visits) == 3:
                    cpu.mem_write(physical(CURRENT), struct.pack("<I", 10))
            elif address in (0x80082E4C, 0x8006BB74, 0x8006BD68):
                self.fail("timing wait should exit after the counter changes")

        cpu.mem_write(physical(CURRENT), struct.pack("<I", 0))
        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.reg_write(UC_MIPS_REG_A0, 10)
        cpu.reg_write(UC_MIPS_REG_A1, 1)
        cpu.reg_write(UC_MIPS_REG_SP, STACK)
        cpu.reg_write(UC_MIPS_REG_RA, RETURN)
        cpu.emu_start(0x8006BCCC, RETURN, count=1000)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
        self.assertEqual(len(visits), 3)


if __name__ == "__main__":
    unittest.main()
