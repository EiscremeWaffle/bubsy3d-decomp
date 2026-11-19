import struct
import unittest

import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0,
        UC_MIPS_REG_A1,
        UC_MIPS_REG_A2,
        UC_MIPS_REG_A3,
        UC_MIPS_REG_PC,
        UC_MIPS_REG_RA,
        UC_MIPS_REG_V0,
    )


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHeadingTimedFlowTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_start_then_stop_with_real_display_paths(self):
        cpu = self.fresh_cpu()
        row = 0x8009F444 + 5 * 0x38
        owner, elements, event = 0x801E5000, 0x801E8000, 0x801E9000
        cpu.mem_write(physical(row), bytes(0x38))
        write_word(cpu, row + 4, owner)
        write_word(cpu, row + 0x20, elements)
        write_word(cpu, row + 0x24, 1)
        cpu.mem_write(physical(row + 0x34), b"\x02")
        calls = []

        def intercept(uc, address, size, user_data):
            a0 = uc.reg_read(UC_MIPS_REG_A0)
            a1 = uc.reg_read(UC_MIPS_REG_A1)
            a2 = uc.reg_read(UC_MIPS_REG_A2)
            a3 = uc.reg_read(UC_MIPS_REG_A3)
            if address == 0x8002BD0C:
                self.assertEqual(a0, 5)
                calls.append(1)
                uc.reg_write(UC_MIPS_REG_V0, 123)
            elif address == 0x80082F1C:
                self.assertEqual((a1, a2), (0x800BDFD8, 123))
                uc.mem_write(physical(a0), b"123\x00")
                calls.append(2)
                uc.reg_write(UC_MIPS_REG_V0, 3)
            elif address == 0x8002BAA0:
                self.assertEqual(a0, 123)
                calls.append(3)
                uc.reg_write(UC_MIPS_REG_V0, 3)
            elif address == 0x8002BE68:
                self.assertEqual(a0, 5)
                uc.mem_write(physical(a1), struct.pack("<h", 10))
                uc.mem_write(physical(a2), struct.pack("<h", -20))
                calls.append(4)
            elif address == 0x8002C4C8:
                self.assertEqual((a0, a1, a2, a3), (5, 123, 45, 0xFFFFFFEC))
                calls.append(5)
            elif address == 0x8004FF50:
                self.assertEqual((a0, a1, a2), (owner, 0x3C, row + 0xC))
                write_word(uc, a2, event)
                calls.append(6)
                uc.reg_write(UC_MIPS_REG_V0, 0)
            elif address == 0x8004FFEC:
                self.assertEqual(a0, event)
                calls.append(7)
                uc.reg_write(UC_MIPS_REG_V0, 0)
            elif address == 0x80050650:
                if a0 == 1:
                    calls.append(9)
                else:
                    self.assertEqual(a0, elements + calls.count(8) * 0x28)
                    calls.append(8)
                uc.reg_write(UC_MIPS_REG_V0, 0)
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.reg_write(UC_MIPS_REG_A0, 5)
        cpu.reg_write(UC_MIPS_REG_A1, 20)
        self.call(cpu, 0x8002CA88)
        self.assertEqual(read_word(cpu, row + 0x30), 20)
        self.assertEqual(cpu.mem_read(physical(row + 0x1C), 1)[0], 1)
        self.assertEqual(struct.unpack("<H", cpu.mem_read(physical(row + 0x1E), 2))[0], 2)
        self.assertEqual(read_word(cpu, row + 0xC), event)
        cpu.reg_write(UC_MIPS_REG_A0, 5)
        cpu.reg_write(UC_MIPS_REG_A1, 99)
        self.call(cpu, 0x8002CA88)
        self.assertEqual(read_word(cpu, row + 0x30), 20)
        self.assertEqual(calls, [1, 2, 3, 4, 5, 6])
        cpu.reg_write(UC_MIPS_REG_A0, 5)
        self.call(cpu, 0x8002CB2C)
        self.assertEqual(read_word(cpu, row + 0x30), 0)
        self.assertEqual(cpu.mem_read(physical(row + 0x1C), 1)[0], 0)
        self.assertEqual(struct.unpack("<H", cpu.mem_read(physical(row + 0x1E), 2))[0], 0)
        self.assertEqual(read_word(cpu, row + 0xC), 0)
        self.assertEqual(calls, [1, 2, 3, 4, 5, 6, 7, 8, 8])

        calls.clear()
        cpu.mem_write(physical(row), bytes(0x38))
        write_word(cpu, row + 4, owner)
        write_word(cpu, row + 0x20, elements)
        write_word(cpu, row + 0x24, 1)
        cpu.mem_write(physical(row + 0x34), b"\x02")
        cpu.mem_write(physical(row + 0x08), b"\x07")
        cpu.mem_write(physical(row + 0x1D), b"\x08")
        write_word(cpu, row + 0x10, 9)
        cpu.reg_write(UC_MIPS_REG_A0, 5)
        cpu.reg_write(UC_MIPS_REG_A1, 20)
        self.call(cpu, 0x8002CA4C)
        cpu.reg_write(UC_MIPS_REG_A0, 5)
        cpu.reg_write(UC_MIPS_REG_A1, 20)
        self.call(cpu, 0x8002CA88)
        cpu.reg_write(UC_MIPS_REG_A0, 5)
        self.call(cpu, 0x8002BAD4)
        self.assertEqual(read_word(cpu, row + 0x30), 20)
        self.assertEqual(cpu.mem_read(physical(row + 0x1C), 1)[0], 0)
        self.assertEqual(struct.unpack("<H", cpu.mem_read(physical(row + 0x1E), 2))[0], 0)
        self.assertEqual(read_word(cpu, row + 0x0C), 0)
        self.assertEqual(cpu.mem_read(physical(row + 0x08), 1)[0], 0)
        self.assertEqual(cpu.mem_read(physical(row + 0x1D), 1)[0], 0)
        self.assertEqual(read_word(cpu, row + 0x10), 0)
        self.assertEqual(calls, [1, 2, 3, 4, 5, 6, 9, 8, 8, 7])
