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


NODE_COUNT = 5
INPUT = struct.Struct("<19I")
OUTPUT = struct.Struct("<23I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSequencePlaybackTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_release_and_allocation_against_original(self):
        generator = random.Random(0x53180)
        cases = []
        for previous in range(NODE_COUNT + 1):
            for next_node in range(NODE_COUNT + 1):
                for free_head in range(NODE_COUNT + 1):
                    for count in (0, 1, 0xFFFF):
                        released = generator.randrange(NODE_COUNT + 1)
                        links_before = [generator.randrange(NODE_COUNT + 1) for _ in range(NODE_COUNT)]
                        links_after = [generator.randrange(NODE_COUNT + 1) for _ in range(NODE_COUNT)]
                        zero_previous = generator.randrange(NODE_COUNT + 1)
                        zero_next = generator.randrange(NODE_COUNT + 1)
                        if released:
                            links_before[released - 1] = previous
                            links_after[released - 1] = next_node
                        else:
                            zero_previous, zero_next = previous, next_node
                        cases.append((
                            *links_before, *links_after, generator.randrange(NODE_COUNT + 1),
                            free_head, released, count, 0, 0,
                            generator.randrange(NODE_COUNT + 1), zero_previous, zero_next,
                        ))
        for _ in range(4000):
            cases.append((
                *(generator.randrange(NODE_COUNT + 1) for _ in range(2 * NODE_COUNT)),
                generator.randrange(NODE_COUNT + 1), generator.randrange(NODE_COUNT + 1),
                generator.randrange(NODE_COUNT + 1), generator.getrandbits(32), 0, 0,
                *(generator.randrange(NODE_COUNT + 1) for _ in range(3)),
            ))
        for active in range(NODE_COUNT + 1):
            for free_head in range(NODE_COUNT + 1):
                for next_free in range(NODE_COUNT + 1):
                    for count in (0, 1, 0xFFFF):
                        previous = [generator.randrange(NODE_COUNT + 1) for _ in range(NODE_COUNT)]
                        following = [generator.randrange(NODE_COUNT + 1) for _ in range(NODE_COUNT)]
                        zero_next = generator.randrange(NODE_COUNT + 1)
                        if free_head:
                            following[free_head - 1] = next_free
                        else:
                            zero_next = next_free
                        cases.append((
                            *previous, *following, active, free_head, 0, count, 1,
                            generator.randrange(NODE_COUNT + 1),
                            generator.randrange(NODE_COUNT + 1),
                            generator.randrange(NODE_COUNT + 1), zero_next,
                        ))
        for _ in range(4000):
            previous = [generator.randrange(NODE_COUNT + 1) for _ in range(NODE_COUNT)]
            following = [generator.randrange(NODE_COUNT + 1) for _ in range(NODE_COUNT)]
            cases.append((
                *previous, *following, generator.randrange(NODE_COUNT + 1),
                generator.randrange(NODE_COUNT + 1), 0, generator.getrandbits(32),
                1, generator.randrange(NODE_COUNT + 1),
                *(generator.randrange(NODE_COUNT + 1) for _ in range(3)),
            ))
        native = run_native_probe(
            self, ["src/game/func_800530FC.c", "src/game/func_80053180.c",
                   "tests/test_sequence_playback.c"],
            cases, INPUT, OUTPUT,
        )

        base = 0x801E5000

        def pointer(index):
            return 0 if index == 0 else base + (index - 1) * 12

        def index(address):
            if address == 0:
                return 0
            self.assertEqual((address - base) % 12, 0)
            result = (address - base) // 12 + 1
            self.assertGreaterEqual(result, 1)
            self.assertLessEqual(result, NODE_COUNT)
            return result

        cpu = self.fresh_cpu()
        assertions = []

        def intercept(uc, address, size, data):
            if address == 0x800556D0:
                assertions.append(tuple(
                    uc.reg_read(register)
                    for register in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3)
                ))
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for case_index, (entry, actual) in enumerate(zip(cases, native)):
            previous = entry[:NODE_COUNT]
            following = entry[NODE_COUNT:2 * NODE_COUNT]
            active, free_head, released, count, operation, owner = entry[10:16]
            zero_owner, zero_previous, zero_next = entry[16:]
            for item in range(NODE_COUNT):
                cpu.mem_write(
                    physical(base + item * 12),
                    struct.pack("<III", item + 1, pointer(previous[item]), pointer(following[item])),
                )
            cpu.mem_write(physical(0), struct.pack(
                "<III", zero_owner, pointer(zero_previous), pointer(zero_next),
            ))
            write_word(cpu, GP + 0x5D0, pointer(active))
            write_word(cpu, GP + 0x5D4, pointer(free_head))
            cpu.mem_write(physical(GP + 0x5D8), struct.pack("<H", count & 0xFFFF))
            assertions.clear()
            cpu.reg_write(UC_MIPS_REG_A0, pointer(released) if operation == 0 else owner)
            self.call(cpu, 0x80053180 if operation == 0 else 0x800530FC)

            nodes = [
                struct.unpack("<III", cpu.mem_read(physical(base + item * 12), 12))
                for item in range(NODE_COUNT)
            ]
            original = (
                *(index(node[1]) for node in nodes),
                *(index(node[2]) for node in nodes),
                *(node[0] for node in nodes),
                index(read_word(cpu, GP + 0x5D0)),
                index(read_word(cpu, GP + 0x5D4)),
                struct.unpack("<H", cpu.mem_read(physical(GP + 0x5D8), 2))[0],
                cpu.reg_read(UC_MIPS_REG_V0) if operation == 0 else index(cpu.reg_read(UC_MIPS_REG_V0)),
                read_word(cpu, 0), index(read_word(cpu, 4)), index(read_word(cpu, 8)),
                len(assertions),
            )
            with self.subTest(case=case_index, input=entry):
                expected_assertions = [(1, 0x80013CAC, 0x80013B74, 0xC68)] if operation and not free_head else []
                self.assertEqual(assertions, expected_assertions)
                self.assertEqual(actual, original)

    def test_allocation_asserts_on_empty_free_list(self):
        cpu = self.fresh_cpu()
        write_word(cpu, GP + 0x5D4, 0)
        reached = []

        def intercept(uc, address, size, data):
            if address == 0x800556D0:
                reached.append(tuple(
                    uc.reg_read(register)
                    for register in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3)
                ))
                uc.emu_stop()

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.reg_write(UC_MIPS_REG_A0, 0x1234)
        cpu.reg_write(UC_MIPS_REG_RA, 0x800F1000)
        cpu.emu_start(0x800530FC, 0x800F1000, count=1000)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), 0x800556D0)
        self.assertEqual(reached, [(1, 0x80013CAC, 0x80013B74, 0xC68)])
        self.assertEqual(read_word(cpu, GP + 0x5D4), 0)
