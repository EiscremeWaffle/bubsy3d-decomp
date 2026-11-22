import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_PC, UC_MIPS_REG_RA,
        UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<17I")
OUTPUT = struct.Struct("<5I")
CONTEXT = 0x801E0000
HEADING_TIMED_RECORDS = 0x8009F444


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryResultLookupTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_result_lookup_against_original(self):
        generator = random.Random(0x2BD0C)
        values = [value for index in range(18) for value in (index, index + 0x10000)]
        values.extend((0xFFFF, 0x8000, 0x7FFF, 0xFFFFFFFF, 0xFFFF0000))
        cases = []
        for value in values:
            for selection in (0, 1):
                durations = tuple(generator.choice((-1, 0, 999, 1000, 1001, 2000))
                                  for _ in range(3))
                slots = [generator.getrandbits(32) for _ in range(10)]
                cases.append((
                    value, selection, generator.getrandbits(32),
                    generator.getrandbits(32), *(duration & 0xFFFFFFFF for duration in durations),
                    *slots,
                ))
        native = run_native_probe(
            self, [
                "src/game/func_8002BD0C.c",
                "tests/test_secondary_result_lookup.c",
            ], cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def intercept(uc, address, size, data):
            if address == 0x80026648:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF, current["value"])
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF, 0)
                current["getter_calls"] += 1
                current["getter_value"] = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
                status = CONTEXT
            elif address == 0x8001AD28:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF, 15)
                current["callback_kind"] = 1
                current["callback_argument"] = 15
                status = current["callback_result"]
            elif address == 0x80037AFC:
                current["callback_kind"] = 2
                status = current["query_result"]
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, status & 0xFFFFFFFF)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for case_number, (entry, native_result) in enumerate(zip(cases, native)):
            value, selection, callback_result, query_result = entry[:4]
            durations = entry[4:7]
            slots = entry[7:17]
            current.update(
                value=value, callback_result=callback_result, query_result=query_result,
                getter_calls=0, getter_value=0, callback_kind=0, callback_argument=0,
            )
            context = bytearray(0x40)
            struct.pack_into("<h", context, 0x14, selection)
            for slot in range(2):
                values_for_slot = slots[slot * 5:slot * 5 + 5]
                base = 0x18 + slot * 0x14
                struct.pack_into("<II", context, base, *values_for_slot[:2])
                context[base + 8] = values_for_slot[2] & 0xFF
                struct.pack_into("<II", context, base + 0x0C, *values_for_slot[3:])
            cpu.mem_write(physical(CONTEXT), bytes(context))

            timed_records = bytearray(18 * 0x38)
            for index, duration in zip((5, 15, 17), durations):
                signed_duration = duration if duration < 0x80000000 else duration - 0x100000000
                struct.pack_into("<i", timed_records, index * 0x38 + 0x30, signed_duration)
            cpu.mem_write(physical(HEADING_TIMED_RECORDS), bytes(timed_records))
            cpu.reg_write(UC_MIPS_REG_A0, value)
            cpu.reg_write(UC_MIPS_REG_A1, 0)
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.emu_start(0x8002BD0C, RETURN, count=1000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                current["getter_value"], current["getter_calls"],
                current["callback_kind"], current["callback_argument"],
            )
            self.assertEqual(native_result, original, f"case {case_number}: {entry}")


if __name__ == "__main__":
    unittest.main()
