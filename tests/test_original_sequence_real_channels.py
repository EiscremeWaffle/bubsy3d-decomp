import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, physical, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_V0


INPUT = struct.Struct("<4I64s64s")
OUTPUT = struct.Struct("<26I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSequenceRealChannelsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_playback_through_real_channel_copies(self):
        generator = random.Random(0x52B2E)
        cases = []
        for format_flags in (0, 0x20000000, 0x40000000, 0x60000000):
            for key in range(3):
                for enabled in (0, 1):
                    for matching_slot in (0, 1):
                        for _ in range(5):
                            first = bytes(generator.getrandbits(8) for _ in range(64))
                            second = bytes(generator.getrandbits(8) for _ in range(64))
                            cases.append((format_flags, key, enabled, matching_slot, first, second))

        native = run_native_probe(
            self,
            [
                "src/game/func_80052BDC.c", "src/game/func_80052E54.c",
                "src/game/func_800530FC.c", "src/game/func_80053180.c",
                "src/game/func_80050128.c", "src/player_model/player_actor_flags.c",
                "src/game/func_80010220.c", "src/game/func_80010244.c",
                "src/game/func_80010278.c", "tests/test_sequence_real_channels.c",
            ],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        actor, sequence, entry = 0x801E2000, 0x801E2100, 0x801E2200
        resource, channels, slots = 0x801E2300, 0x801E2400, 0x801E2500
        nodes = (0x801E2600, 0x801E2620, 0x801E2640)
        frames = (0x801E3000, 0x801E3100)
        outputs = (0x801E3200, 0x801E3300)
        visited = set()
        covered = set()

        def watch(uc, address, size, data):
            if address in (0x80052E54, 0x80010220, 0x80010244, 0x80010278):
                visited.add(address)
            if address in (0x800556D0, 0x80067DEC):
                raise AssertionError(f"unexpected backend {address:#x}")

        cpu.hook_add(UC_HOOK_CODE, watch)
        for index, (case, actual) in enumerate(zip(cases, native)):
            format_flags, key, enabled, matching_slot, first, second = case
            cpu.mem_write(physical(actor), bytes(0x20))
            cpu.mem_write(physical(sequence), bytes(0x30))
            write_word(cpu, actor + 0x18, sequence)
            cpu.mem_write(physical(actor + 4), b"\xED\x00\xFF\x00")
            write_word(cpu, sequence + 4, entry)
            write_word(cpu, entry, key)
            write_word(cpu, sequence + 0x10, frames[0])
            write_word(cpu, sequence + 0x14, frames[1])
            write_word(cpu, sequence + 0x20, resource)
            write_word(cpu, sequence + 0x24, slots)
            cpu.mem_write(physical(sequence + 0x28), struct.pack("<H", 0x55AA))
            cpu.mem_write(physical(sequence + 0x2B), bytes((enabled,)))
            write_word(cpu, sequence + 0x2C, format_flags | (2 << 3))
            write_word(cpu, resource + 8, channels)
            cpu.mem_write(physical(channels), struct.pack("<4I", outputs[0], 2, outputs[1], 2))
            for slot_index, timestamp in enumerate((7, -10)):
                slot = bytearray(24)
                struct.pack_into("<i", slot, 12, timestamp)
                slot[16] = key if slot_index == 0 and matching_slot else 0xC0 + slot_index
                struct.pack_into("<I", slot, 20, 0xDEADBEEF)
                cpu.mem_write(physical(slots + slot_index * 24), bytes(slot))
            cpu.mem_write(physical(nodes[0]), struct.pack("<III", 0, 0, nodes[1]))
            cpu.mem_write(physical(nodes[1]), struct.pack("<III", 0, nodes[0], 0))
            cpu.mem_write(physical(nodes[2]), bytes(12))
            write_word(cpu, GP + 0x5D0, nodes[2])
            write_word(cpu, GP + 0x5D4, nodes[0])
            cpu.mem_write(physical(GP + 0x5D8), struct.pack("<H", 1))
            cpu.mem_write(physical(frames[0]), first)
            cpu.mem_write(physical(frames[1]), second)
            cpu.mem_write(physical(outputs[0]), b"\xA5" * 32)
            cpu.mem_write(physical(outputs[1]), b"\x5A" * 32)
            visited.clear()
            cpu.reg_write(UC_MIPS_REG_A0, actor)
            cpu.reg_write(UC_MIPS_REG_A1, 0)
            self.call(cpu, 0x80052BDC)

            slot_data = [cpu.mem_read(physical(slots + slot_index * 24), 24) for slot_index in range(2)]
            original = (
                cpu.reg_read(UC_MIPS_REG_V0),
                cpu.mem_read(physical(actor + 4), 1)[0],
                cpu.mem_read(physical(actor + 6), 1)[0],
                int(read_word(cpu, actor) == nodes[0]),
                struct.unpack("<H", cpu.mem_read(physical(sequence + 0x28), 2))[0],
                struct.unpack("<H", cpu.mem_read(physical(GP + 0x5D8), 2))[0],
                *(slot[8] for slot in slot_data),
                *(struct.unpack_from("<I", slot, 20)[0] for slot in slot_data),
                *struct.unpack("<8I", cpu.mem_read(physical(outputs[0]), 32)),
                *struct.unpack("<8I", cpu.mem_read(physical(outputs[1]), 32)),
            )
            with self.subTest(case=index, format=format_flags, key=key, enabled=enabled, match=matching_slot):
                self.assertEqual(actual, original)
            if matching_slot:
                self.assertNotIn(0x80052E54, visited)
                covered.add("key match")
            else:
                self.assertIn(0x80052E54, visited)
                if enabled:
                    covered.update(visited & {0x80010220, 0x80010244, 0x80010278})
                else:
                    covered.add("disabled initializer")
        self.assertEqual(covered, {
            "key match", "disabled initializer", 0x80010220, 0x80010244, 0x80010278,
        })


if __name__ == "__main__":
    unittest.main()
