import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
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


INPUT = struct.Struct("<10I")
OUTPUT = struct.Struct("<14I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHeadingTimedStartTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_timed_start_against_original(self):
        positions = ((0, 0), (0x7FFF, 0xFFFF), (0x8000, 0x7FFF))
        cases = [
            (index, active, present, maximum, flag, count, first, second, 0xFFFE7960, mutations)
            for index, active, present, maximum, flag, count, (first, second), mutations
            in itertools.product(
                (5, 15, 17), (0, 1, 2), (0, 1), (0, 1, 3, 255), (0, 1),
                (0, 1, 2, 3, 255, 0x7FFF, 0x8000, 0xFFFF, 0x10001),
                positions, (0, 1, 2, 4, 8, 15),
            )
        ]
        generator = random.Random(0x2C604)
        for _ in range(500):
            cases.append((
                generator.choice((5, 15, 17)), generator.randrange(3), generator.randrange(2),
                generator.randrange(256), generator.getrandbits(32), generator.getrandbits(32),
                generator.getrandbits(16), generator.getrandbits(16), generator.getrandbits(32),
                generator.getrandbits(4),
            ))

        native_outputs = run_native_probe(self, [
            "src/game/func_8002C604.c", "tests/test_heading_timed_start.c",
        ], cases, INPUT, OUTPUT)
        table, elements, format_address = 0x8009F444, 0x801E8000, 0x800BDFD8
        for case_index, (entry, native) in enumerate(zip(cases, native_outputs)):
            (index, active, present, maximum, flag, count, first, second, value, mutations) = entry
            with self.subTest(index=case_index, input=entry):
                cpu = self.fresh_cpu()
                row = table + index * 0x38
                cpu.mem_write(physical(row + 0x1C), bytes((active,)))
                cpu.mem_write(physical(row + 0x1E), struct.pack("<H", 0xFFF7))
                write_word(cpu, row + 0x20, elements if present else 0)
                write_word(cpu, row + 0x24, flag)
                cpu.mem_write(physical(row + 0x34), bytes((maximum,)))
                calls = []
                data = {"value": 0, "first": 0, "second": 0}

                def intercept(uc, address, size, user_data):
                    a0 = uc.reg_read(UC_MIPS_REG_A0)
                    a1 = uc.reg_read(UC_MIPS_REG_A1)
                    a2 = uc.reg_read(UC_MIPS_REG_A2)
                    a3 = uc.reg_read(UC_MIPS_REG_A3)
                    if address == 0x8002BD0C:
                        self.assertEqual(a0, index)
                        calls.append(1)
                        if mutations & 1:
                            uc.mem_write(physical(row + 0x34), b"\x03")
                            write_word(uc, row + 0x24, 1)
                            write_word(uc, row + 0x20, 0)
                        uc.reg_write(UC_MIPS_REG_V0, value)
                    elif address == 0x80082F1C:
                        self.assertEqual((a1, a2), (format_address, value))
                        calls.append(2)
                        uc.mem_write(physical(a0), b"0\x00")
                        uc.reg_write(UC_MIPS_REG_V0, 1)
                    elif address == 0x8002BAA0:
                        self.assertEqual(a0, value)
                        calls.append(3)
                        if mutations & 2:
                            uc.mem_write(physical(row + 0x34), b"\x02")
                        uc.reg_write(UC_MIPS_REG_V0, count)
                    elif address == 0x8002BE68:
                        self.assertEqual(a0, index)
                        calls.append(4)
                        uc.mem_write(physical(a1), struct.pack("<H", first))
                        uc.mem_write(physical(a2), struct.pack("<H", second))
                        if mutations & 4:
                            write_word(uc, row + 0x24, 1)
                            uc.mem_write(physical(row + 0x1E), struct.pack("<H", 0xFFF9))
                    elif address == 0x8002C4C8:
                        self.assertEqual(a0, index)
                        calls.append(5)
                        data["value"], data["first"], data["second"] = a1, a2, a3
                        if mutations & 8:
                            uc.mem_write(physical(row + 0x1C), b"\x02")
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_A0, index)
                self.call(cpu, 0x8002C604)
                original = (
                    cpu.mem_read(physical(row + 0x1C), 1)[0],
                    struct.unpack("<H", cpu.mem_read(physical(row + 0x1E), 2))[0],
                    cpu.mem_read(physical(row + 0x34), 1)[0],
                    read_word(cpu, row + 0x24), int(read_word(cpu, row + 0x20) != 0),
                    data["value"], data["first"], data["second"],
                    len(calls), *calls, *([0] * (5 - len(calls))),
                )
                self.assertEqual(native, original)
