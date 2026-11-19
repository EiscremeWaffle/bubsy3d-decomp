import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, physical, write_word

if HAS_UNICORN:
    from unicorn import UcError
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0


BUFFER_BYTES = 4096
INPUT = struct.Struct(f"<6I{BUFFER_BYTES}s")
OUTPUT = struct.Struct(f"<I{BUFFER_BYTES}s")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalStreamGridTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_grid_against_original(self):
        generator = random.Random(0x10368)
        cases = []
        for _ in range(1200):
            stride = generator.choice((0, 1, 7, 9, 17, 25, 41, 97))
            offset = generator.randrange(32)
            table = generator.choice((0, 4, 64, 512, 1024, 2048))
            output = generator.choice((0, 4, 256, 512, 1024, 2048, 3072))
            memory = bytes(generator.getrandbits(8) for _ in range(BUFFER_BYTES))
            cases.append((generator.getrandbits(32), generator.getrandbits(32),
                          stride, offset, table, output, memory))
        native = run_native_probe(
            self, ["src/game/func_80010368.c", "tests/test_sequence_stream_grid.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        arguments, descriptor, memory = 0x801E2000, 0x801E2100, 0x801E4000
        overlaps = set()
        for number, (case, actual) in enumerate(zip(cases, native)):
            x, y, stride, offset, table, output, data = case
            cpu.mem_write(physical(memory), data)
            cpu.mem_write(physical(arguments), bytes(36))
            write_word(cpu, descriptor, memory + output)
            write_word(cpu, arguments + 8, x)
            write_word(cpu, arguments + 0xC, y)
            write_word(cpu, arguments + 0x10, descriptor)
            write_word(cpu, arguments + 0x14, memory + table)
            write_word(cpu, arguments + 0x18, stride)
            write_word(cpu, arguments + 0x1C, offset)
            cpu.reg_write(UC_MIPS_REG_A0, arguments)
            self.call(cpu, 0x80010368)
            original = (cpu.reg_read(UC_MIPS_REG_V0), bytes(cpu.mem_read(physical(memory), BUFFER_BYTES)))
            with self.subTest(case=number, stride=stride, table=table, output=output):
                self.assertEqual(actual, original)
            overlaps.add(table + offset < output + 1024 and output < table + offset + 14 * stride + 9)
        self.assertEqual(overlaps, {False, True})

    def test_final_unused_height_load_can_fault(self):
        cpu = self.fresh_cpu()
        arguments, descriptor, output = 0x801E2000, 0x801E2100, 0x801E4000
        cpu.mem_write(physical(arguments), bytes(36))
        write_word(cpu, descriptor, output)
        write_word(cpu, arguments + 0x10, descriptor)
        write_word(cpu, arguments + 0x14, 0x801FFFF0)
        write_word(cpu, arguments + 0x18, 1)
        cpu.reg_write(UC_MIPS_REG_A0, arguments)
        cpu.reg_write(UC_MIPS_REG_RA, RETURN)
        with self.assertRaises(UcError):
            cpu.emu_start(0x80010368, RETURN, count=10000)
        self.assertIn(cpu.reg_read(UC_MIPS_REG_PC), (0x80010394, 0x80010398))


if __name__ == "__main__":
    unittest.main()
