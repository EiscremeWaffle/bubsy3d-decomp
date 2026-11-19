import itertools
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, RETURN, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<6I")
OUTPUT = struct.Struct("<12I")
ARGUMENTS = (0, 0x801E6000, 0x801E6010)
FLAG_505 = GP + 0x505
FLAG_507 = GP + 0x507
CONTEXT_BYTE = 0x801D5F7D
DISPLAY = 0x8018B950


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerCleanupTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_cleanup_against_original_with_real_context_getter(self):
        cases = list(itertools.product(range(3), range(3), (0, 1, 255), (0, 1, 255),
                                       (0, 1, 255), range(32)))
        native_outputs = run_native_probe(
            self, ["src/game/func_800493F8.c", "tests/test_secondary_controller_cleanup.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_byte(uc, address):
            return uc.mem_read(physical(address), 1)[0]

        def write_byte(uc, address, value):
            uc.mem_write(physical(address), bytes((value & 0xFF,)))

        def intercept(uc, address, size, data):
            call = {
                0x80026648: 1, 0x8002C370: 2, 0x80050650: 3, 0x80028EA8: 4,
                0x8002CB2C: 5, 0x8002BAD4: 6,
            }.get(address)
            if call is None:
                return
            current["calls"].append(call)
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            mutations = current["mutations"]
            if call == 1:
                self.assertEqual((a0, a1), current["arguments"])
                return
            if call == 2:
                current["argument0"] = ARGUMENTS.index(a0)
                current["argument1"] = ARGUMENTS.index(a1)
                if mutations & 1:
                    write_byte(uc, FLAG_505, 0x11)
                    write_byte(uc, FLAG_507, 0x22)
                    write_byte(uc, CONTEXT_BYTE, 0x33)
            elif call == 3:
                self.assertEqual(a0, DISPLAY)
                if mutations & 2:
                    write_byte(uc, FLAG_505, 0x44)
                    write_byte(uc, FLAG_507, 0x55)
                    write_byte(uc, CONTEXT_BYTE, 0x66)
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            elif call == 4:
                self.assertEqual((read_byte(uc, FLAG_505), read_byte(uc, FLAG_507),
                                  read_byte(uc, CONTEXT_BYTE)), (0, 0, 0))
                if mutations & 4:
                    write_byte(uc, FLAG_505, 0xA1)
                    write_byte(uc, FLAG_507, 0xA2)
                    write_byte(uc, CONTEXT_BYTE, 0xA3)
            elif call == 5:
                self.assertEqual(a0, 5)
                if mutations & 8:
                    write_byte(uc, FLAG_505, read_byte(uc, FLAG_505) ^ 8)
            else:
                self.assertEqual(a0, 5)
                if mutations & 16:
                    write_byte(uc, FLAG_507, read_byte(uc, FLAG_507) ^ 16)
                    write_byte(uc, CONTEXT_BYTE, read_byte(uc, CONTEXT_BYTE) ^ 16)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            argument0, argument1, flag_505, flag_507, context_byte, mutations = entry
            current.update(
                mutations=mutations, arguments=(ARGUMENTS[argument0], ARGUMENTS[argument1]),
                argument0=0, argument1=0, calls=[],
            )
            write_byte(cpu, FLAG_505, flag_505)
            write_byte(cpu, FLAG_507, flag_507)
            write_byte(cpu, CONTEXT_BYTE, context_byte)
            cpu.reg_write(UC_MIPS_REG_A0, ARGUMENTS[argument0])
            cpu.reg_write(UC_MIPS_REG_A1, ARGUMENTS[argument1])
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.emu_start(0x800493F8, RETURN, count=1000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
            original = (
                read_byte(cpu, FLAG_505), read_byte(cpu, FLAG_507), read_byte(cpu, CONTEXT_BYTE),
                current["argument0"], current["argument1"], len(current["calls"]), *current["calls"],
            )
            self.assertEqual(original, native, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
