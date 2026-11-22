import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_PC, UC_MIPS_REG_RA,
        UC_MIPS_REG_S4, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<11I")
OUTPUT = struct.Struct("<24I")
CONTEXT = 0x801E4000
REFERENCE = 0x801E4300
BASIS = 0x8018B978
POSITION = 0x8018B950
EVENT = 0x800BE640
FLAG = GP + 0x505


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerConfirmStartTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_confirmation_start_against_original(self):
        rng = random.Random(0x49920)
        cases = [
            (selection, mutations, initial, *(rng.getrandbits(32) for _ in range(8)))
            for selection, mutations, initial in itertools.product((0, 1, 0xFFFF), range(128), (0, 2))
        ]
        native_results = run_native_probe(
            self, ["src/game/secondary_controller_confirm_start.c",
                   "tests/test_secondary_controller_confirm_start.c"], cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def set_selection(value):
            cpu.mem_write(physical(CONTEXT + 0x14), struct.pack("<H", value))

        def intercept(uc, address, size, data):
            if address in (0x8004A0F4, 0x80049A24):
                current["outcome"] = int(address == 0x80049A24)
                uc.emu_stop()
                return
            a0, a1, a2 = (uc.reg_read(reg) & 0xFFFFFFFF for reg in
                          (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2))
            if address == 0x8002E348:
                self.assertEqual(a0, 0xFFFFFFFF)
                current["calls"].append(1)
                if current["mutations"] & 1:
                    set_selection(0)
            elif address == 0x800516F8:
                self.assertEqual(a0, current["stack"] + 0x9C)
                self.assertEqual(uc.mem_read(physical(FLAG), 1)[0], 1)
                current["calls"].append(2)
                write32(a0, REFERENCE)
                if current["mutations"] & 2:
                    set_selection(1)
            elif address == 0x80055D94:
                self.assertEqual((a0, a1, a2), (POSITION, 256, 240))
                self.assertEqual(bytes(uc.mem_read(physical(BASIS), 32)),
                                 bytes(uc.mem_read(physical(REFERENCE), 32)))
                current["calls"].append(3)
                if current["mutations"] & 4:
                    set_selection(0)
            elif address == 0x8004FF50:
                self.assertEqual((a0, a1, a2), (0x800493CC, 60, EVENT))
                self.assertEqual(uc.mem_read(physical(CONTEXT + 0x0D), 1)[0], 0)
                current["calls"].append(4)
                write32(EVENT, 0x1234)
                if current["mutations"] & 8:
                    uc.mem_write(physical(FLAG), b"\0")
            elif address == 0x8002C708:
                self.assertEqual(uc.mem_read(physical(CONTEXT + 0x0D), 1)[0], 1)
                current["calls"].append(0x100 + a0)
                if a0 == 6 and current["mutations"] & 16:
                    set_selection(1)
            elif address == 0x8002BAD4:
                current["calls"].append(0x200 + a0)
                if current["mutations"] & 32:
                    set_selection(0)
            elif address == 0x8002CA88:
                self.assertEqual((a0, a1), (5, 10))
                current["calls"].append(5)
            elif address == 0x8001C6BC:
                self.assertEqual((a0, a1, a2), (6, 2, 1))
                current["calls"].append(6)
                if current["mutations"] & 64:
                    uc.mem_write(physical(FLAG), b"\0")
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        stack = STACK - 0x128
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            selection, mutations, initial, *basis = entry
            current.update(mutations=mutations, stack=stack, calls=[], outcome=None)
            cpu.mem_write(physical(CONTEXT), bytes(0x80))
            cpu.mem_write(physical(BASIS), bytes(32))
            cpu.mem_write(physical(REFERENCE), struct.pack("<8I", *basis))
            cpu.mem_write(physical(FLAG), bytes((initial,)))
            set_selection(selection)
            write32(EVENT, 0)
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.reg_write(UC_MIPS_REG_S4, CONTEXT)
            cpu.emu_start(0x80049920, 0, count=1000)
            self.assertIsNotNone(current["outcome"], f"case {index} did not reach a boundary")
            calls = current["calls"] + [0] * (10 - len(current["calls"]))
            original = (
                current["outcome"],
                struct.unpack("<H", cpu.mem_read(physical(CONTEXT + 0x14), 2))[0],
                cpu.mem_read(physical(FLAG), 1)[0],
                cpu.mem_read(physical(CONTEXT + 0x0D), 1)[0],
                int(read32(EVENT) != 0),
                *struct.unpack("<8I", cpu.mem_read(physical(BASIS), 32)),
                len(current["calls"]), *calls,
            )
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
