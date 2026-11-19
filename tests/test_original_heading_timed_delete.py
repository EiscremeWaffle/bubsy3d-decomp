import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_PC, UC_MIPS_REG_RA


INPUT = struct.Struct("<8I")
OUTPUT = struct.Struct("<12I")
TABLE = 0x8009F444


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHeadingTimedDeleteTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_timed_delete_against_original(self):
        cases = [
            (index, resource, event, 7, 8, 9, mutations, replacement)
            for index, resource, event, mutations, replacement in itertools.product(
                (0, 5, 15, 17, 0x10005), (0, 0x1234), (0, 0x4321), range(8), (0, 0x5678)
            )
        ]
        generator = random.Random(0x2BAD4)
        for _ in range(500):
            cases.append((
                generator.randrange(18) | (generator.getrandbits(16) << 16),
                generator.choice((0, generator.getrandbits(32))),
                generator.choice((0, generator.getrandbits(32))),
                generator.getrandbits(32), generator.getrandbits(32), generator.getrandbits(32),
                generator.randrange(8), generator.choice((0, generator.getrandbits(32))),
            ))
        native_outputs = run_native_probe(self, [
            "src/game/func_8002BAD4.c", "tests/test_heading_timed_delete.c",
        ], cases, INPUT, OUTPUT)
        for case_index, (entry, native) in enumerate(zip(cases, native_outputs)):
            index, resource, event, state_08, state_1d, state_10, mutations, replacement = entry
            with self.subTest(index=case_index, input=entry):
                cpu = self.fresh_cpu()
                row = TABLE + (index & 0xFFFF) * 0x38
                write_word(cpu, row + 0x24, resource)
                write_word(cpu, row + 0x0C, event)
                cpu.mem_write(physical(row + 0x08), bytes((state_08 & 0xFF,)))
                cpu.mem_write(physical(row + 0x1D), bytes((state_1d & 0xFF,)))
                write_word(cpu, row + 0x10, state_10)
                calls = []
                arguments = {"resource": 0, "cleanup": 0, "remove": 0}

                def intercept(uc, address, size, user_data):
                    argument = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
                    if address == 0x80050650:
                        calls.append(1)
                        arguments["resource"] = argument
                        if mutations & 1:
                            write_word(uc, row + 0x0C, replacement)
                    elif address == 0x8002C708:
                        calls.append(2)
                        arguments["cleanup"] = argument
                        if mutations & 2:
                            write_word(uc, row + 0x0C, replacement)
                    elif address == 0x8004FFEC:
                        calls.append(3)
                        arguments["remove"] = argument
                        if mutations & 4:
                            write_word(uc, row + 0x0C, 0x55555555)
                            write_word(uc, row + 0x24, 0x66666666)
                            uc.mem_write(physical(row + 0x08), b"\x77")
                            uc.mem_write(physical(row + 0x1D), b"\x88")
                            write_word(uc, row + 0x10, 0x99999999)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_A0, index)
                self.call(cpu, 0x8002BAD4)
                original = (
                    read_word(cpu, row + 0x0C), read_word(cpu, row + 0x24),
                    cpu.mem_read(physical(row + 0x08), 1)[0],
                    cpu.mem_read(physical(row + 0x1D), 1)[0],
                    read_word(cpu, row + 0x10), len(calls), *calls, *([0] * (3 - len(calls))),
                    arguments["resource"], arguments["cleanup"], arguments["remove"],
                )
                self.assertEqual(native, original)
