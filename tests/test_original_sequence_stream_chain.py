import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, physical, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_V0


FRAME_BYTES, HEIGHT_BYTES, LOOKUP_BYTES = 1536, 2048, 512
INPUT = struct.Struct(f"<9I{FRAME_BYTES}s{HEIGHT_BYTES}s{LOOKUP_BYTES}s")
OUTPUT = struct.Struct("<12I1024s1024s2048s3I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalStreamChainTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_real_stream_chain_against_original(self):
        generator = random.Random(0x67102)
        cases = []
        for key in range(3):
            for width in range(1, 5):
                for slot in range(3):
                    for _ in range(8):
                        records = [generator.getrandbits(32) & 0xFDFFFFFF for _ in range(5)]
                        lookup = b"".join(struct.pack(
                            "<I", generator.choice(records + [generator.getrandbits(32)])
                            | generator.choice((0, 0x02000000))) for _ in range(128))
                        cases.append((
                            key, width, slot, generator.randrange(1, 6), *records,
                            bytes(generator.getrandbits(8) for _ in range(FRAME_BYTES)),
                            bytes(generator.getrandbits(8) for _ in range(HEIGHT_BYTES)),
                            lookup,
                        ))
        native = run_native_probe(
            self,
            [
                "src/game/func_80052BDC.c", "src/game/func_80052E54.c",
                "src/game/func_800530FC.c", "src/game/func_80053180.c",
                "src/game/func_80050128.c", "src/player_model/player_actor_flags.c",
                "src/game/func_80010220.c", "src/game/func_80010244.c",
                "src/game/func_80010278.c",
                "src/game/func_80067DEC.c", "src/game/func_80010368.c",
                "src/game/func_800102BC.c", "tests/test_sequence_stream_chain.c",
            ],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        actor, sequence, slots = 0x801E2000, 0x801E2100, 0x801E2200
        owner, descriptor = 0x801E2300, 0x801E2400
        nodes = (0x801E2600, 0x801E2620, 0x801E2640)
        frames, heights, lookup = 0x801D0000, 0x801D1000, 0x801D2000
        grid, expanded, metadata = 0x801E3000, 0x801E4000, 0x801E5000
        records_address, count_address = 0x801D7A34, 0x800BE424
        visited = set()

        def watch(uc, address, size, data):
            if address in (0x800530FC, 0x800529F8, 0x80052E54, 0x80067DEC, 0x80010368, 0x800102BC):
                visited.add(address)
            if address == 0x800556D0:
                self.fail("unexpected sequence assertion")

        cpu.hook_add(UC_HOOK_CODE, watch)
        for number, (case, actual) in enumerate(zip(cases, native)):
            key, width, slot, count, *rest = case
            records, frame_data, height_data, lookup_data = rest[:5], rest[5], rest[6], rest[7]
            cpu.mem_write(physical(actor), bytes(0x20))
            cpu.mem_write(physical(sequence), bytes(0x30))
            cpu.mem_write(physical(slots), bytes(3 * 24))
            cpu.mem_write(physical(actor + 4), b"\xED\x00\xFF\x00")
            cpu.mem_write(physical(actor + 8), struct.pack("<h", key))
            write_word(cpu, actor + 0x18, sequence)
            write_word(cpu, sequence + 0x10, frames)
            write_word(cpu, sequence + 0x14, lookup)
            write_word(cpu, sequence + 0x18, heights)
            write_word(cpu, sequence + 0x24, slots)
            cpu.mem_write(physical(sequence + 0x28), struct.pack("<H", 0x55AA))
            cpu.mem_write(physical(sequence + 0x2B), b"\x01")
            write_word(cpu, sequence + 0x2C, 4 | (3 << 3))
            write_word(cpu, owner + 8, descriptor)
            write_word(cpu, descriptor, grid)
            write_word(cpu, descriptor + 8, expanded)
            write_word(cpu, descriptor + 0x10, metadata)
            for index in range(3):
                write_word(cpu, slots + index * 24 + 4, owner)
                cpu.mem_write(physical(slots + index * 24 + 0x10), bytes((0x40 + index,)))
                write_word(cpu, slots + index * 24 + 0xC,
                           (-100 if index == slot else index + 10) & 0xFFFFFFFF)
                write_word(cpu, slots + index * 24 + 0x14, 0xDEADBEEF)
            cpu.mem_write(physical(nodes[0]), struct.pack("<III", 0, 0, nodes[1]))
            cpu.mem_write(physical(nodes[1]), struct.pack("<III", 0, nodes[0], 0))
            cpu.mem_write(physical(nodes[2]), bytes(12))
            write_word(cpu, GP + 0x5D0, nodes[2])
            write_word(cpu, GP + 0x5D4, nodes[0])
            cpu.mem_write(physical(GP + 0x5D8), struct.pack("<H", 1))
            cpu.mem_write(physical(GP + 0x5F4), bytes((width,)))
            for index, value in enumerate(records):
                write_word(cpu, records_address + index * 0x34 + 0x10, value)
            cpu.mem_write(physical(count_address), struct.pack("<H", count))
            cpu.mem_write(physical(frames), frame_data)
            cpu.mem_write(physical(heights), height_data)
            cpu.mem_write(physical(lookup), lookup_data)
            cpu.mem_write(physical(grid), bytes((0xA5,)) * 1024)
            cpu.mem_write(physical(expanded), bytes((0xC3,)) * 1024)
            cpu.mem_write(physical(metadata), bytes((0x5A,)) * 2048)
            visited.clear()
            cpu.reg_write(UC_MIPS_REG_A0, actor)
            cpu.reg_write(UC_MIPS_REG_A1, 0)
            self.call(cpu, 0x80052BDC)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0),
                cpu.mem_read(physical(actor + 4), 1)[0],
                cpu.mem_read(physical(actor + 6), 1)[0],
                int(read_word(cpu, actor) == nodes[0]),
                struct.unpack("<H", cpu.mem_read(physical(sequence + 0x28), 2))[0],
                struct.unpack("<H", cpu.mem_read(physical(GP + 0x5D8), 2))[0],
                *(cpu.mem_read(physical(slots + index * 24 + 8), 1)[0] for index in range(3)),
                *(read_word(cpu, slots + index * 24 + 0x14) for index in range(3)),
                bytes(cpu.mem_read(physical(grid), 1024)),
                bytes(cpu.mem_read(physical(expanded), 1024)),
                bytes(cpu.mem_read(physical(metadata), 2048)),
                *(cpu.mem_read(physical(slots + index * 24 + 0x10), 1)[0] for index in range(3)),
            )
            with self.subTest(case=number, key=key, width=width, slot=slot, count=count):
                self.assertEqual(actual, original)
                self.assertEqual(visited, {
                    0x800530FC, 0x800529F8, 0x80052E54, 0x80067DEC, 0x80010368, 0x800102BC,
                })


if __name__ == "__main__":
    unittest.main()
