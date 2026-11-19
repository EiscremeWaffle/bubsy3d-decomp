import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical, write_word

if HAS_UNICORN:
    from unicorn import UcError
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )


BUFFER_BYTES = 8192
INPUT = struct.Struct(f"<10I{BUFFER_BYTES}s")
OUTPUT = struct.Struct(f"<I{BUFFER_BYTES}s")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalStreamTransferTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_transfer_against_original(self):
        generator = random.Random(0x102BC)
        cases = []
        for _ in range(1200):
            source = generator.choice((0, 4, 512, 1024, 2048))
            output = generator.choice((0, 4, 512, 1024, 2048, 4096))
            metadata = generator.choice((0, 1024, 2048, 3072, 6144))
            lookup = generator.choice((0, 4, 512, 2048, 4096))
            records = [generator.getrandbits(32) & 0xFDFFFFFF for _ in range(5)]
            data = bytearray(generator.getrandbits(8) for _ in range(BUFFER_BYTES))
            for index in range(128):
                value = generator.choice(records + [generator.getrandbits(32)])
                struct.pack_into("<I", data, lookup + index * 4, value | generator.choice((0, 0x02000000)))
            cases.append((generator.randrange(1, 6), source, output, metadata, lookup, *records, bytes(data)))
        for count in (0, 0xFFFF, 0x8000):
            records = [0x10000000 + index for index in range(5)]
            data = bytearray(BUFFER_BYTES)
            for index in range(128):
                struct.pack_into("<I", data, 512 + index * 4, records[0])
            cases.append((count, 0, 1024, 2048, 512, *records, bytes(data)))

        native = run_native_probe(
            self, ["src/game/func_800102BC.c", "tests/test_sequence_stream_transfer.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        memory, records_address, count_address = 0x801E4000, 0x801D7A34, 0x800BE424
        overlap_cases = set()
        for number, (case, actual) in enumerate(zip(cases, native)):
            count, source, output, metadata, lookup, *rest = case
            records, data = rest[:5], rest[5]
            cpu.mem_write(physical(memory), data)
            cpu.mem_write(physical(count_address), struct.pack("<H", count))
            for index, value in enumerate(records):
                write_word(cpu, records_address + index * 0x34 + 0x10, value)
            cpu.reg_write(UC_MIPS_REG_A0, memory + output)
            cpu.reg_write(UC_MIPS_REG_A1, memory + source)
            cpu.reg_write(UC_MIPS_REG_A2, memory + metadata)
            cpu.reg_write(UC_MIPS_REG_A3, memory + lookup)
            self.call(cpu, 0x800102BC)
            original = (cpu.reg_read(UC_MIPS_REG_V0), bytes(cpu.mem_read(physical(memory), BUFFER_BYTES)))
            with self.subTest(case=number, count=count, source=source, output=output,
                              metadata=metadata, lookup=lookup):
                self.assertEqual(actual, original)
            for kind, a, alen, b, blen in (
                ("source/output", source, 512, output, 1024),
                ("source/metadata", source, 512, metadata, 2048),
                ("lookup/output", lookup, 512, output, 1024),
                ("lookup/metadata", lookup, 512, metadata, 2048),
            ):
                if a < b + blen and b < a + alen:
                    overlap_cases.add(kind)
        self.assertEqual(overlap_cases, {
            "source/output", "source/metadata", "lookup/output", "lookup/metadata",
        })

    def test_zero_search_count_still_reads_first_record(self):
        cpu = self.fresh_cpu()
        memory = 0x801E4000
        cpu.mem_write(physical(memory), bytes(BUFFER_BYTES))
        cpu.mem_write(physical(0x800BE424), struct.pack("<H", 1))
        cpu.mem_unmap(physical(0x801D7000), 0x1000)
        cpu.reg_write(UC_MIPS_REG_A0, memory + 1024)
        cpu.reg_write(UC_MIPS_REG_A1, memory)
        cpu.reg_write(UC_MIPS_REG_A2, memory + 2048)
        cpu.reg_write(UC_MIPS_REG_A3, memory + 512)
        cpu.reg_write(UC_MIPS_REG_RA, controller_route.RETURN)
        with self.assertRaises(UcError):
            cpu.emu_start(0x800102BC, controller_route.RETURN, count=1000)
        self.assertIn(cpu.reg_read(UC_MIPS_REG_PC), (0x8001032C, 0x80010330))


if __name__ == "__main__":
    unittest.main()
