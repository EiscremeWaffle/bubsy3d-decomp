import itertools
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0


INPUT = struct.Struct("<5I")
OUTPUT = struct.Struct("<8I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHeadingTimedCleanupTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_timed_cleanup_against_original(self):
        cases = list(itertools.product(
            (5, 15), (0, 1, 255), (0, 1, 2, 3, 0xFFFF, 0x8000), (0, 1), range(32)
        ))
        native_outputs = run_native_probe(self, [
            "src/game/func_8002C708.c", "tests/test_heading_timed_cleanup.c",
        ], cases, INPUT, OUTPUT)
        table, bases = 0x8009F444, (0x801E8000, 0x801E8100)
        for case_index, (entry, native) in enumerate(zip(cases, native_outputs)):
            index, active, count, base_present, mutations = entry
            with self.subTest(index=case_index, input=entry):
                cpu = self.fresh_cpu()
                row = table + index * 0x38
                cpu.mem_write(physical(row + 0x1C), bytes((active,)))
                cpu.mem_write(physical(row + 0x1E), struct.pack("<H", count))
                write_word(cpu, row + 0x20, bases[0] if base_present else 0)
                calls = []

                def intercept(uc, address, size, user_data):
                    if address != 0x80050650:
                        return
                    payload = uc.reg_read(UC_MIPS_REG_A0)
                    choices = {
                        base + element * 0x28: base_index * 10 + element + 1
                        for base_index, base in enumerate(bases) for element in range(4)
                    }
                    self.assertIn(payload, choices)
                    calls.append(choices[payload])
                    self.assertLessEqual(len(calls), 4)
                    if len(calls) == 1:
                        if mutations & 1:
                            uc.mem_write(physical(row + 0x1E), struct.pack("<H", 1))
                        if mutations & 2:
                            write_word(uc, row + 0x20, bases[1])
                        if mutations & 4:
                            uc.mem_write(physical(row + 0x1E), struct.pack("<H", 3))
                        if mutations & 8:
                            uc.mem_write(physical(row + 0x1C), b"\x7F")
                        if mutations & 16:
                            uc.mem_write(physical(row + 0x1E), b"\xFF\xFF")
                    uc.reg_write(UC_MIPS_REG_V0, 1)
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_A0, index)
                self.call(cpu, 0x8002C708)
                base = read_word(cpu, row + 0x20)
                self.assertIn(base, (0, *bases))
                original = (
                    cpu.mem_read(physical(row + 0x1C), 1)[0],
                    struct.unpack("<H", cpu.mem_read(physical(row + 0x1E), 2))[0],
                    0 if base == 0 else bases.index(base) + 1,
                    len(calls), *calls, *([0] * (4 - len(calls))),
                )
                self.assertEqual(native, original)
