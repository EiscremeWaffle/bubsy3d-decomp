import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0,
        UC_MIPS_REG_A1,
        UC_MIPS_REG_A2,
        UC_MIPS_REG_PC,
        UC_MIPS_REG_RA,
        UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<7I")
OUTPUT = struct.Struct("<13I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHeadingTimedRecordTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_timed_records_against_original(self):
        indices = (0, 4, 5, 15, 17, 18, 0xFFFFFFFF, 0x10005)
        durations = (0, 1, 20, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFE, 0xFFFFFFFF)
        stored = (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        cases = [
            (operation, index, duration, current, event, 0x12345678, mutations)
            for operation, index, duration, current, event, mutations in itertools.product(
                (0, 1), indices, durations, stored, (0, 0x10101010), (0, 1, 2, 4, 8, 15)
            )
        ]
        generator = random.Random(0x2CA882CB2C)
        for _ in range(300):
            cases.append((
                generator.randrange(2), generator.choice(indices), generator.getrandbits(32),
                generator.getrandbits(32), generator.choice((0, generator.getrandbits(32))),
                generator.getrandbits(32), generator.getrandbits(4),
            ))

        native_outputs = run_native_probe(self, [
            "src/game/func_8002CA4C.c", "src/game/func_8002CA88.c", "src/game/func_8002CB2C.c",
            "tests/test_heading_timed_records.c",
        ], cases, INPUT, OUTPUT)
        table = 0x8009F444
        for case_index, (entry, native) in enumerate(zip(cases, native_outputs)):
            operation, index, duration, current, event, owner, mutations = entry
            with self.subTest(index=case_index, input=entry):
                cpu = self.fresh_cpu()
                row = table + (index if index < 18 else 0) * 0x38
                write_word(cpu, row + 4, owner)
                write_word(cpu, row + 0xC, event)
                write_word(cpu, row + 0x30, current)
                calls = []
                data = {"start": 0, "stop": 0, "owner": 0, "value": 0, "removed": 0}

                def intercept(uc, address, size, user_data):
                    a0 = uc.reg_read(UC_MIPS_REG_A0)
                    a1 = uc.reg_read(UC_MIPS_REG_A1)
                    a2 = uc.reg_read(UC_MIPS_REG_A2)
                    if address == 0x8002C604:
                        calls.append(1)
                        data["start"] = a0
                        if mutations & 1:
                            write_word(uc, row + 4, 0x22222222)
                            write_word(uc, row + 0x30, 0x33333333)
                            write_word(uc, row + 0xC, 0x44444444)
                    elif address == 0x8004FF50:
                        self.assertEqual(a2, row + 0xC)
                        calls.append(2)
                        data["owner"], data["value"] = a0, a1
                        if mutations & 2:
                            write_word(uc, a2, 0x55555555)
                        uc.reg_write(UC_MIPS_REG_V0, 3)
                    elif address == 0x8004FFEC:
                        calls.append(3)
                        data["removed"] = a0
                        if mutations & 4:
                            write_word(uc, row + 0xC, 0x66666666)
                            write_word(uc, row + 0x30, 0x77777777)
                        uc.reg_write(UC_MIPS_REG_V0, 1)
                    elif address == 0x8002C708:
                        calls.append(4)
                        data["stop"] = a0
                        if mutations & 8:
                            write_word(uc, row + 4, 0x88888888)
                            write_word(uc, row + 0x30, 0x11111111)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_A0, index)
                cpu.reg_write(UC_MIPS_REG_A1, duration)
                self.call(cpu, 0x8002CA88 if operation == 0 else 0x8002CB2C)
                self.assertLessEqual(len(calls), 4)
                original = (
                    read_word(cpu, row + 0x30), read_word(cpu, row + 0xC),
                    read_word(cpu, row + 4), len(calls), *calls, *([0] * (4 - len(calls))),
                    data["start"], data["stop"], data["owner"], data["value"], data["removed"],
                )
                self.assertEqual(native, original)

    def test_duration_setter_against_original(self):
        indices = (0, 4, 5, 6, 14, 15, 16, 17, 18, 0xFFFFFFFF, 0x10005)
        values = (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        cases = [
            (2, index, duration, current, 0x10101010, 0x12345678, 0)
            for index, duration, current in itertools.product(indices, values, values)
        ]
        native_outputs = run_native_probe(self, [
            "src/game/func_8002CA4C.c", "src/game/func_8002CA88.c", "src/game/func_8002CB2C.c",
            "tests/test_heading_timed_records.c",
        ], cases, INPUT, OUTPUT)
        table = 0x8009F444
        for case_index, (entry, native) in enumerate(zip(cases, native_outputs)):
            _, index, duration, current, event, owner, _ = entry
            with self.subTest(index=case_index, input=entry):
                cpu = self.fresh_cpu()
                row = table + (index if index < 18 else 0) * 0x38
                write_word(cpu, row + 4, owner)
                write_word(cpu, row + 0xC, event)
                write_word(cpu, row + 0x30, current)
                before = bytes(cpu.mem_read(table & 0x1FFFFFFF, 18 * 0x38))
                cpu.reg_write(UC_MIPS_REG_A0, index)
                cpu.reg_write(UC_MIPS_REG_A1, duration)
                self.call(cpu, 0x8002CA4C)
                after = bytearray(before)
                if index in (5, 15, 17):
                    struct.pack_into("<I", after, index * 0x38 + 0x30, duration)
                self.assertEqual(bytes(cpu.mem_read(table & 0x1FFFFFFF, len(after))), bytes(after))
                self.assertEqual(native, (
                    read_word(cpu, row + 0x30), read_word(cpu, row + 0xC),
                    read_word(cpu, row + 4), 0, *([0] * 9),
                ))
