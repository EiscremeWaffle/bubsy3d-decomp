import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, physical, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<5I")
OUTPUT = struct.Struct("<28I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalStreamBackendTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_stream_backend_against_original(self):
        generator = random.Random(0x67DEC)
        edge = (0, 1, 2, 0xFF, 0x100, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFE, 0xFFFFFFFF)
        cases = []
        for _ in range(5000):
            slot = generator.choice((-2, -1, 0, 1, 2))
            context = generator.choice((0x100, 0, 1, 2, 3, 7, 0xFF, 0x10001, 0xFFFFFF80))
            if slot >= 0 and not context & 0xFF:
                context |= 1
            cases.append((
                slot & 0xFFFFFFFF, generator.choice(edge + (generator.getrandbits(32),)),
                context, generator.randrange(1024), generator.choice((-2, -1, 0, 1, 2)) & 0xFFFFFFFF,
            ))
        native = run_native_probe(
            self, ["src/game/func_80067DEC.c", "tests/test_sequence_stream.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        actor, sequence, alternate = 0x801E2000, 0x801E2100, 0x801E2180
        slots = (0x801E3000, 0x801E3100)
        owners, descriptors = 0x801E4000, 0x801E5000
        outputs, metadata, frames, tables, lookups = (
            0x801E6000, 0x801E7000, 0x801D0000, 0x801D4000, 0x801D5000,
        )
        calls = []
        mutations = 0
        changed_slot = 0

        def intercept(uc, address, size, data):
            if address == 0x80010368:
                pointer = uc.reg_read(UC_MIPS_REG_A0)
                arguments = list(struct.unpack("<9I", uc.mem_read(physical(pointer), 36)))
                descriptor = arguments[4]
                self.assertEqual((descriptor - descriptors) % 0x20, 0)
                arguments[4] = (descriptor - descriptors) // 0x20
                arguments[5] = (arguments[5] - tables) & 0xFFFFFFFF
                arguments[8] = (arguments[8] - lookups) & 0xFFFFFFFF
                calls.append(tuple(arguments))
                if mutations & 1:
                    write_word(uc, sequence + 0x14, lookups + 0x100)
                if mutations & 2:
                    uc.mem_write(physical(actor + 6), bytes((changed_slot & 0xFF,)))
                if mutations & 4:
                    write_word(uc, sequence + 0x24, slots[1] + 2 * 24)
                if mutations & 8:
                    write_word(uc, actor + 0x18, alternate)
                if mutations & 16:
                    write_word(uc, descriptor + 8, outputs + 0x700)
                    write_word(uc, descriptor + 0x10, metadata + 0x700)
                if mutations & 32:
                    write_word(uc, sequence + 0x10, frames + 0x900)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
            elif address == 0x800102BC:
                calls.append((
                    (uc.reg_read(UC_MIPS_REG_A0) - outputs) & 0xFFFFFFFF,
                    (uc.reg_read(UC_MIPS_REG_A1) - (frames + 0x800)) & 0xFFFFFFFF,
                    (uc.reg_read(UC_MIPS_REG_A2) - metadata) & 0xFFFFFFFF,
                    (uc.reg_read(UC_MIPS_REG_A3) - lookups) & 0xFFFFFFFF,
                ))
                if mutations & 64:
                    uc.mem_write(physical(actor + 6), bytes((changed_slot & 0xFF,)))
                if mutations & 128:
                    write_word(uc, sequence + 0x24, slots[1] + 2 * 24)
                if mutations & 256:
                    write_word(uc, actor + 0x18, alternate)
                if mutations & 512:
                    write_word(uc, sequence + 0x14, lookups + 0x200)
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
            elif address == 0x800556D0:
                self.fail("unexpected stream-width assertion")

        cpu.hook_add(UC_HOOK_CODE, intercept)
        covered = set()
        for number, (case, actual) in enumerate(zip(cases, native)):
            slot, key, context, mutations, changed_slot = case
            cpu.mem_write(physical(actor), bytes(0x20))
            cpu.mem_write(physical(sequence), bytes(0x30))
            cpu.mem_write(physical(alternate), bytes(0x30))
            uc_slot = slot & 0xFF
            cpu.mem_write(physical(actor + 6), bytes((uc_slot,)))
            write_word(cpu, actor + 0x18, sequence)
            write_word(cpu, sequence + 0x10, frames + 0x800)
            write_word(cpu, sequence + 0x14, lookups)
            write_word(cpu, sequence + 0x18, tables + 0x100)
            write_word(cpu, sequence + 0x24, slots[0] + 2 * 24)
            for index in range(5):
                owner = owners + index * 0x20
                descriptor = descriptors + index * 0x20
                write_word(cpu, owner + 8, descriptor)
                write_word(cpu, descriptor + 8, outputs + index * 0x40)
                write_word(cpu, descriptor + 0x10, metadata + index * 0x40)
                for table in range(2):
                    address = slots[table] + index * 24
                    write_word(cpu, address + 4, owner)
                    cpu.mem_write(physical(address + 0x10), bytes((0x40 + table * 0x20 + index,)))
            calls.clear()
            cpu.reg_write(UC_MIPS_REG_A0, actor)
            cpu.reg_write(UC_MIPS_REG_A1, key)
            cpu.reg_write(UC_MIPS_REG_A2, context)
            self.call(cpu, 0x80067DEC)
            if calls:
                self.assertEqual(len(calls), 2)
            original = (
                len(calls),
                *((calls[0] if calls else (0,) * 9)),
                *((calls[1] if calls else (0,) * 4)),
                *(cpu.mem_read(physical(slots[table] + index * 24 + 0x10), 1)[0]
                  for table in range(2) for index in range(5)),
                int(read_word(cpu, actor + 0x18) == alternate),
                int(read_word(cpu, sequence + 0x24) == slots[1] + 2 * 24),
                struct.unpack("<b", cpu.mem_read(physical(actor + 6), 1))[0] & 0xFFFFFFFF,
                (read_word(cpu, sequence + 0x14) - lookups) & 0xFFFFFFFF,
            )
            with self.subTest(case=number, input=case):
                self.assertEqual(actual, original)
            if uc_slot & 0x80:
                covered.add("negative initial slot")
            else:
                covered.add("streaming")
                if changed_slot & 0x80000000 and mutations & (2 | 64):
                    covered.add("negative final slot")
                if mutations & 1:
                    covered.add("lookup mutated by first callback")
                if mutations & 16:
                    covered.add("descriptor mutated after capture")
                if mutations & (4 | 128):
                    covered.add("slot table mutated")
                if mutations & (8 | 256):
                    covered.add("actor sequence mutated")
        self.assertEqual(covered, {
            "negative initial slot", "streaming", "negative final slot",
            "lookup mutated by first callback", "descriptor mutated after capture",
            "slot table mutated", "actor sequence mutated",
        })

    def test_zero_width_assertion_and_trap(self):
        for context in (0, 0x100, 0xFFFFFF00):
            run_native_probe(
                self, ["src/game/func_80067DEC.c", "tests/test_sequence_stream.c"],
                [(0, 17, context, 0, 0)], INPUT, OUTPUT,
                expected_error=b"stream width assertion returned",
            )
            cpu = self.fresh_cpu()
            actor, sequence = 0x801E2000, 0x801E2100
            cpu.mem_write(physical(actor), bytes(0x20))
            write_word(cpu, actor + 0x18, sequence)
            observed = []

            def intercept(uc, address, size, data):
                if address == 0x800556D0:
                    observed.append((
                        uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1),
                        uc.reg_read(UC_MIPS_REG_A2), uc.reg_read(UC_MIPS_REG_A3),
                    ))
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                elif address == 0x80067E50:
                    observed.append("divide-by-zero break")
                    uc.emu_stop()

            cpu.hook_add(UC_HOOK_CODE, intercept)
            cpu.reg_write(UC_MIPS_REG_A0, actor)
            cpu.reg_write(UC_MIPS_REG_A1, 17)
            cpu.reg_write(UC_MIPS_REG_A2, context)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.emu_start(0x80067DEC, RETURN, count=1000)
            self.assertEqual(observed, [(0, 0x80013E20, 0x80013E30, 0xED), "divide-by-zero break"])


if __name__ == "__main__":
    unittest.main()
