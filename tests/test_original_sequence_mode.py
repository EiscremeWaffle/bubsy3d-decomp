import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, physical, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )


SLOT_COUNT = 31
NODE_COUNT = 3
INPUT = struct.Struct("<138I")
OUTPUT = struct.Struct("<151I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSequenceModeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_sequence_mode_and_real_list_helpers_against_original(self):
        generator = random.Random(0x52BDC)
        cases = []
        for case_index in range(5000):
            actor_present = generator.randrange(10) != 0
            mode = generator.choice((-0x80000000, -2, 0, 1, 1, 2, 0x7FFFFFFF))
            has_node = generator.randrange(2)
            early_return = (mode == 1 and not has_node) or (mode != 1 and has_node)
            sequence_present = not early_return or generator.randrange(5) != 0
            slot = generator.choice((-128, -1, 0, 1, 2, 15, 30))
            table_present = generator.randrange(4) != 0
            if mode == 1 and has_node and slot >= 0:
                table_present = True
            slot_count = generator.choice((0, 1, 2, 3, 7, 15, 31))
            sequence_flags = (generator.getrandbits(32) & ~0xF8) | (slot_count << 3)
            cursor = generator.choice((-32768, -1, 0, 1, 17, 255, 256, 32767))
            if sequence_flags & 4 == 0:
                cursor = 0
            entry = generator.choice((-0x80000000, -2001, -1000, -1, 0, 17, 999, 2017, 0x7FFFFFFF))
            free_present = generator.randrange(3) != 0
            mutations = generator.randrange(8)
            if not free_present and mutations & 1 and mode != 1 and not has_node:
                key = cursor
            elif sequence_flags & 4:
                key = cursor
            else:
                key = (abs(entry) % 1000) * (-1 if entry < 0 else 1)

            slots = []
            for index in range(SLOT_COUNT):
                active = generator.choice((0, 0, 1, 2, 255))
                timestamp = generator.choice((-0x80000000, -1, 0, 1, 100, 0x7FFFFFFF))
                slot_key = generator.randrange(256)
                value = generator.getrandbits(32)
                slots.append([active, timestamp, slot_key, value])

            if slot_count and case_index % 4 == 0 and 0 <= key <= 255:
                match_index = generator.randrange(slot_count)
                slots[match_index][0] = 0
                slots[match_index][2] = key
            elif case_index % 4 == 1:
                for slot_values in slots[:slot_count]:
                    slot_values[2] = (key + 1) & 0xFF
            elif case_index % 4 == 2:
                for slot_values in slots[:slot_count]:
                    slot_values[0] = 1

            values = (
                actor_present, sequence_present, mode, generator.getrandbits(32), has_node,
                slot, sequence_flags, cursor, entry, table_present, generator.getrandbits(32),
                generator.getrandbits(32), free_present, mutations,
                *(value for slot_values in slots for value in slot_values),
            )
            cases.append(tuple(value & 0xFFFFFFFF for value in values))

        native = run_native_probe(
            self,
            [
                "src/game/func_80052BDC.c", "src/game/func_800530FC.c",
                "src/game/func_80053180.c", "src/game/func_80050128.c",
                "src/player_model/player_actor_flags.c", "tests/test_sequence_mode.c",
            ],
            cases, INPUT, OUTPUT,
        )

        actor = 0x801E2000
        sequence = 0x801E2100
        alternate = 0x801E2180
        entry_address = 0x801E2200
        slots_address = 0x801E2400
        nodes_address = 0x801E2800

        def node_pointer(index):
            return 0 if index == 0 else nodes_address + (index - 1) * 12

        def node_index(address):
            if address == 0:
                return 0
            self.assertEqual((address - nodes_address) % 12, 0)
            index = (address - nodes_address) // 12 + 1
            self.assertGreaterEqual(index, 1)
            self.assertLessEqual(index, NODE_COUNT)
            return index

        def owner_index(address):
            self.assertIn(address, (0, actor))
            return int(address != 0)

        cpu = self.fresh_cpu()
        assertions = []
        initializations = []
        visited = set()
        coverage = set()
        mutations = 0

        def intercept(uc, address, size, data):
            visited.add(address)
            if address == 0x800556D0:
                values = tuple(
                    uc.reg_read(register)
                    for register in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3)
                )
                assertions.append(values)
                if values[3] == 0xC68:
                    if mutations & 1:
                        write_word(uc, actor + 0x18, alternate)
                    if mutations & 2:
                        write_word(uc, GP + 0x5D0, node_pointer(1))
                        uc.mem_write(physical(GP + 0x5D8), struct.pack("<H", 0xFFFF))
                elif values[3] == 0xB3F and mutations & 4:
                    uc.mem_write(physical(actor + 6), b"\x04")
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
            elif address == 0x80052E54:
                initializations.append((uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)))
                uc.reg_write(UC_MIPS_REG_V0, 17)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for case_index, (case, actual) in enumerate(zip(cases, native)):
            (actor_present, sequence_present, mode, flags, has_node, slot, sequence_flags,
             cursor, entry, table_present, sample, count, free_present, mutations) = case[:14]
            cpu.mem_write(physical(actor), bytes(0x1C))
            write_word(cpu, actor, node_pointer(1) if has_node else 0)
            cpu.mem_write(physical(actor + 4), bytes((flags & 0xFF, 0, slot & 0xFF, 0)))
            cpu.mem_write(physical(actor + 8), struct.pack("<H", cursor & 0xFFFF))
            write_word(cpu, actor + 0x18, sequence if sequence_present else 0)
            cpu.mem_write(physical(sequence), bytes(0x30))
            write_word(cpu, sequence + 4, entry_address)
            write_word(cpu, sequence + 0x24, slots_address if table_present else 0)
            write_word(cpu, sequence + 0x2C, sequence_flags)
            cpu.mem_write(physical(alternate), bytes(0x30))
            write_word(cpu, alternate + 0x2C, 4)
            write_word(cpu, entry_address, entry)
            for index in range(SLOT_COUNT):
                active, timestamp, key, value = case[14 + index * 4:18 + index * 4]
                slot_data = bytearray(24)
                slot_data[8] = active & 0xFF
                struct.pack_into("<I", slot_data, 12, timestamp)
                slot_data[16] = key & 0xFF
                struct.pack_into("<I", slot_data, 20, value)
                cpu.mem_write(physical(slots_address + index * 24), bytes(slot_data))
            for index, words in enumerate((
                (actor, 0, 0), (0, 0, node_pointer(3)), (0, node_pointer(2), 0),
            )):
                cpu.mem_write(physical(node_pointer(index + 1)), struct.pack("<III", *words))
            cpu.mem_write(physical(0), struct.pack("<III", 0, node_pointer(3), node_pointer(3)))
            write_word(cpu, GP + 0x5D0, node_pointer(1) if has_node else 0)
            write_word(cpu, GP + 0x5D4, node_pointer(2) if free_present else 0)
            cpu.mem_write(physical(GP + 0x5D8), struct.pack("<H", count & 0xFFFF))
            write_word(cpu, GP + 0x5AC, sample)
            assertions.clear()
            initializations.clear()
            visited.clear()
            cpu.reg_write(UC_MIPS_REG_A0, actor if actor_present else 0)
            cpu.reg_write(UC_MIPS_REG_A1, mode)
            self.call(cpu, 0x80052BDC)

            if not actor_present:
                coverage.add("null actor")
            if actor_present and not sequence_present:
                coverage.add("null sequence on early return")
            if 0x80052C38 in visited:
                coverage.add("release")
            if 0x80052C50 in visited:
                coverage.add("timestamp")
            if 0x80052CD0 in visited:
                coverage.add("allocation")
                if not table_present:
                    coverage.add("null slot table")
                if (sequence_flags >> 3) & 31 == 31:
                    coverage.add("31 slots")
            if 0x80052CA4 in visited:
                coverage.add("matching slot")
            if 0x80052DDC in visited:
                coverage.add("oldest slot")
            if any(item[3] == 0xC68 for item in assertions):
                coverage.add("empty free list")
                if mutations & 1:
                    coverage.add("sequence mutation")
                if mutations & 2:
                    coverage.add("pool mutation")
            if any(item[3] == 0xB3F for item in assertions):
                coverage.add("no available slot")
                if mutations & 4:
                    coverage.add("assertion mutation")

            sequence_pointer = read_word(cpu, actor + 0x18)
            self.assertIn(sequence_pointer, (0, sequence, alternate))
            nodes = [
                struct.unpack("<III", cpu.mem_read(physical(node_pointer(index + 1)), 12))
                for index in range(NODE_COUNT)
            ]
            slot_values = []
            for index in range(SLOT_COUNT):
                slot_data = cpu.mem_read(physical(slots_address + index * 24), 24)
                slot_values.extend((
                    slot_data[8], struct.unpack_from("<I", slot_data, 12)[0], slot_data[16],
                    struct.unpack_from("<I", slot_data, 20)[0],
                ))
            codes = [item[0] for item in assertions] + [0, 0]
            lines = [item[3] for item in assertions] + [0, 0]
            original = (
                cpu.reg_read(UC_MIPS_REG_V0), cpu.mem_read(physical(actor + 4), 1)[0],
                cpu.mem_read(physical(actor + 6), 1)[0], node_index(read_word(cpu, actor)),
                1 if sequence_pointer == sequence else 2 if sequence_pointer == alternate else 0,
                node_index(read_word(cpu, GP + 0x5D0)), node_index(read_word(cpu, GP + 0x5D4)),
                struct.unpack("<H", cpu.mem_read(physical(GP + 0x5D8), 2))[0],
                owner_index(read_word(cpu, 0)), node_index(read_word(cpu, 4)), node_index(read_word(cpu, 8)),
                len(assertions), *codes[:2], *lines[:2], len(initializations),
                initializations[0][1] if initializations else 0,
                *(node_index(node[1]) for node in nodes),
                *(node_index(node[2]) for node in nodes),
                *(owner_index(node[0]) for node in nodes),
                *slot_values,
            )
            with self.subTest(case=case_index):
                for code, condition, source, line in assertions:
                    expected = (1, 0x80013CAC) if line == 0xC68 else (0, 0x80013C98)
                    self.assertEqual((code, condition), expected)
                    self.assertEqual(source, 0x80013B74)
                    self.assertIn(line, (0xC68, 0xB3F))
                self.assertLessEqual(len(assertions), 2)
                self.assertLessEqual(len(initializations), 1)
                if initializations:
                    self.assertEqual(initializations[0][0], actor)
                self.assertEqual(actual, original)
        self.assertEqual(
            coverage,
            {
                "null actor", "null sequence on early return", "release", "timestamp",
                "allocation", "null slot table", "31 slots", "matching slot", "oldest slot",
                "empty free list", "sequence mutation", "pool mutation", "no available slot",
                "assertion mutation",
            },
        )
