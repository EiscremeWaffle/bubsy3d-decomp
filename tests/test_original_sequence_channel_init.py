import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, physical, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<9I")
OUTPUT = struct.Struct("<14I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSequenceChannelInitTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_channel_initialization_against_original(self):
        generator = random.Random(0x52E54)
        edge = (0, 1, 2, 3, 0x7FFF, 0xFFFF, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        flags = (0, 4, 0x20000000, 0x40000000, 0x60000000, 0x60000004, 0xFFFFFFFF)
        cases = []
        for _ in range(5000):
            cases.append((
                generator.choice((0, 1, 1, 2, 255)),
                generator.choice(flags),
                generator.getrandbits(32),
                generator.choice(edge),
                generator.choice(edge),
                generator.choice(edge),
                generator.randrange(64),
                generator.choice(flags),
                generator.choice(edge),
            ))

        native = run_native_probe(
            self, ["src/game/func_80052E54.c", "tests/test_sequence_channel_init.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        actor, sequence, alternate = 0x801E2000, 0x801E2100, 0x801E2180
        resources = (0x801E2200, 0x801E2240)
        channels = (0x801E2300, 0x801E2320)
        input_base, output_base = 0x801E3000, 0x801E4000
        calls = []
        mutations = 0
        changed_flags = 0
        changed_count = 0
        covered = set()

        def mutate(uc):
            if mutations & 1:
                write_word(uc, sequence + 0x2C, changed_flags)
            if mutations & 2:
                write_word(uc, sequence + 0x20, resources[1])
            if mutations & 4:
                write_word(uc, sequence + 0x14, input_base + 0x300)
            if mutations & 8:
                write_word(uc, channels[0] + 0x0C, changed_count)
            if mutations & 16:
                write_word(uc, actor + 0x18, alternate)
            if mutations & 32:
                uc.mem_write(physical(sequence + 0x28), struct.pack("<H", 0xFFFE))

        def intercept(uc, address, size, data):
            if address in (0x80010220, 0x80010244, 0x80010278):
                kind = {0x80010220: 0, 0x80010244: 1, 0x80010278: 2}[address]
                calls.append((
                    kind,
                    (uc.reg_read(UC_MIPS_REG_A0) - output_base) & 0xFFFFFFFF,
                    (uc.reg_read(UC_MIPS_REG_A1) - input_base) & 0xFFFFFFFF,
                    uc.reg_read(UC_MIPS_REG_A2),
                ))
                if len(calls) == 1:
                    mutate(uc)
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
            elif address == 0x80067DEC:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), actor)
                calls.append((3, 0, uc.reg_read(UC_MIPS_REG_A1), uc.reg_read(UC_MIPS_REG_A2)))
                mutate(uc)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (case, actual) in enumerate(zip(cases, native)):
            (enabled, initial_flags, context, key, first_count, second_count,
             mutations, changed_flags, changed_count) = case
            cpu.mem_write(physical(actor), bytes(0x20))
            cpu.mem_write(physical(sequence), bytes(0x30))
            cpu.mem_write(physical(alternate), bytes(0x30))
            write_word(cpu, actor + 0x18, sequence)
            write_word(cpu, sequence + 0x10, input_base + 0x100)
            write_word(cpu, sequence + 0x14, input_base + 0x200)
            write_word(cpu, sequence + 0x20, resources[0])
            cpu.mem_write(physical(sequence + 0x28), struct.pack("<H", 0x5678))
            cpu.mem_write(physical(sequence + 0x2B), bytes((enabled,)))
            write_word(cpu, sequence + 0x2C, initial_flags)
            write_word(cpu, resources[0] + 8, channels[0])
            write_word(cpu, resources[1] + 8, channels[1])
            cpu.mem_write(physical(channels[0]), struct.pack(
                "<4I", output_base + 0x20, first_count, output_base + 0x40, second_count))
            cpu.mem_write(physical(channels[1]), struct.pack(
                "<4I", 0, 0, output_base + 0x80, changed_count))
            cpu.mem_write(physical(GP + 0x5F4), bytes((context & 0xFF,)))
            calls.clear()
            cpu.reg_write(UC_MIPS_REG_A0, actor)
            cpu.reg_write(UC_MIPS_REG_A1, key)
            self.call(cpu, 0x80052E54)
            sequence_pointer = read_word(cpu, actor + 0x18)
            resource_pointer = read_word(cpu, sequence + 0x20)
            self.assertIn(sequence_pointer, (sequence, alternate))
            self.assertIn(resource_pointer, resources)
            padded_calls = calls + [(0, 0, 0, 0)] * (2 - len(calls))
            original = (
                cpu.reg_read(UC_MIPS_REG_V0),
                struct.unpack("<H", cpu.mem_read(physical(sequence + 0x28), 2))[0],
                int(sequence_pointer == alternate),
                read_word(cpu, sequence + 0x2C),
                int(resource_pointer == resources[1]),
                len(calls),
                *(value for call in padded_calls for value in call),
            )
            with self.subTest(case=index):
                self.assertEqual(actual, original)

            if not enabled:
                covered.add("disabled")
            elif initial_flags & 4:
                covered.add("streamed")
            else:
                covered.add(("first", calls[0][0]))
                covered.add(("second", calls[1][0]))
                if mutations & 16:
                    covered.add("component sequence changed")
                if mutations & 8:
                    covered.add("second count changed")
                if mutations & 7 == 7:
                    covered.add("flags, resource, and frames changed")
        self.assertEqual(covered, {
            "disabled", "streamed", ("first", 0), ("first", 1),
            ("second", 0), ("second", 2), "component sequence changed",
            "second count changed", "flags, resource, and frames changed",
        })


if __name__ == "__main__":
    unittest.main()
