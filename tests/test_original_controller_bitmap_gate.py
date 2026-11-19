import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_S2, UC_MIPS_REG_S3,
        UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<20I")
OUTPUT = struct.Struct("<26I")
CONTEXT = 0x801E4000
REFRESHED = 0x801E4100
FLAGS = 0x801E0000
FLAGS_POINTER = 0x800BE698
HOLD = GP + 0x1A6
TIMEOUT = GP + 0x1A2
ACTION = GP + 0x1A4
LIFECYCLE = 0x801D5FB4
CURSOR = 0x800BDF20
EMPTY_VALUE = 0xFFFFFFFF


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerBitmapGateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_pre_bitmap_gates_against_original(self):
        generator = random.Random(0x24840)
        cases = []
        for buttons, hold, flags, first, second, mode, timeout in itertools.product(
            (0, 0x100, 0x110, 0x800, 0x8800, 0x8900, 0xFFFF, 0x8000, 0x10),
            (0, 1, 60, 61, 254, 255), (0, 4, 8, 12), (0, 1), (0, 1), (0, 1), (0, 1),
        ):
            cases.append((
                buttons, mode, 0, 0, 0, 4, 4, flags, hold, timeout,
                7, 9, first, second, 1, 3, 1, 0, 255, 0x1234,
            ))
        cases.append((
            0x8800, 2, 0, 0, 0, 4, 4, 4, 0, 0,
            7, 9, 1, 0, 0, 8, 1, 128, 255, 0x12345678,
        ))
        cases.extend(
            (
                generator.getrandbits(16), generator.randrange(4),
                generator.choice((0, 0, 0, 1, 2)), generator.choice((0, 0, 0, 1, 2)),
                generator.choice((0, 0, 1, 2)), generator.choice((0, 1, 4, 255)),
                generator.choice((0, 1, 4, 255)), generator.choice((0, 4, 8, 12, 255)),
                generator.choice((0, 0, 1, 60, 61, 254, 255)),
                generator.choice((0, 0, 1, 2, 255)), generator.randrange(256),
                generator.getrandbits(32), generator.choice((0, 1, 2, 255, 256, 257)),
                generator.choice((0, 1, 2, 255, 256, 257)),
                generator.choice((0, 1, 2, 0x101, 0xFFFFFFFF)),
                generator.randrange(16), generator.choice((0, 1, 2, 0x100, 0x101, 0xFFFFFFFF)),
                generator.randrange(512), generator.randrange(256), generator.getrandbits(32),
            )
            for _ in range(6000)
        )
        native_outputs = run_native_probe(
            self, ["src/game/controller_bitmap_gate.c", "src/game/func_80056678.c",
                   "tests/test_controller_bitmap_gate.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_word(uc, address):
            return struct.unpack("<I", uc.mem_read(physical(address), 4))[0]

        def write_word(uc, address, value):
            uc.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read_byte(uc, address):
            return uc.mem_read(physical(address), 1)[0]

        def write_byte(uc, address, value):
            uc.mem_write(physical(address), bytes((value & 0xFF,)))

        def intercept(uc, address, size, data):
            if address in (0x80024A70, 0x80024BD4):
                current["stop"] = address
                uc.emu_stop()
                return
            call = {
                0x80047050: 1, 0x8001B558: 2, 0x80023A30: 3, 0x8001A280: 4,
                0x800556D0: 5, 0x80038430: 6, 0x800222E0: 7, 0x80023A84: 8,
                0x8003AA1C: 9,
            }.get(address)
            if call is None:
                return
            current["calls"].append(call)
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            a3 = uc.reg_read(UC_MIPS_REG_A3) & 0xFFFFFFFF
            mutations = current["mutations"]
            if call == 1:
                self.assertEqual((a0, read_word(uc, LIFECYCLE)), (1, 2))
                if mutations & 1:
                    write_byte(uc, HOLD, current["replacement_hold"])
            elif call == 2:
                self.assertEqual((a0, a1, a2, a3), (1, 5, 0xFFFF, 0xFFFF))
                stack = uc.reg_read(UC_MIPS_REG_SP)
                self.assertEqual(tuple(read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C)),
                                 (0, 0, 0, 0))
                if mutations & 2:
                    write_byte(uc, HOLD, read_byte(uc, HOLD) ^ 0x55)
            elif call == 3:
                self.assertEqual(a0, 60)
                self.assertLess(current["interval_calls"], 2)
                result = current["intervals"][current["interval_calls"]]
                current["interval_calls"] += 1
                if mutations & 4:
                    write_byte(uc, CONTEXT + 0x29, read_byte(uc, CONTEXT + 0x29) ^ 1)
                uc.reg_write(UC_MIPS_REG_V0, result & 0xFF)
            elif call == 4:
                self.assertEqual(a0, CURSOR)
                if not mutations & 128:
                    for index in range(4):
                        write_word(uc, a1 + 4 * index, current["entry_seed"] + index)
                if mutations & 8:
                    write_word(uc, CURSOR, read_word(uc, CURSOR) ^ 0x23)
                uc.reg_write(UC_MIPS_REG_V0, current["cycle_result"])
            elif call == 5:
                self.assertEqual((a0, a1, a2, a3), (0, 0x8001214C, 0x800120AC, 0xA19))
            elif call == 6:
                current["entry_words"] = [read_word(uc, a0 + 4 * index) for index in range(4)]
                if mutations & 16:
                    write_byte(uc, CONTEXT + 0x29, read_byte(uc, CONTEXT + 0x29) ^ 1)
            elif call == 7:
                if mutations & 256:
                    write_byte(uc, ACTION, read_byte(uc, ACTION) ^ 0x55)
                uc.reg_write(UC_MIPS_REG_V0, current["ready"])
            elif call == 8:
                self.assertEqual(read_byte(uc, ACTION), 1)
                if mutations & 32:
                    write_byte(uc, ACTION, read_byte(uc, ACTION) ^ 0x80)
            else:
                write_word(uc, a0, CONTEXT if mutations & 64 else REFRESHED)
                uc.reg_write(UC_MIPS_REG_V0, 1)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            (buttons, mode, blocked_12, blocked_2b, blocked_29, toggle, refreshed_toggle,
             flags, hold, timeout, action, lifecycle, first, second, cycle_result, cursor,
             ready, mutations, replacement_hold, entry_seed) = entry
            current.update(
                intervals=(first, second), cycle_result=cycle_result, ready=ready,
                mutations=mutations, replacement_hold=replacement_hold, entry_seed=entry_seed,
                calls=[], interval_calls=0, entry_words=[EMPTY_VALUE] * 4, stop=None,
            )
            stack = STACK - 0x120
            cpu.mem_write(physical(stack), bytes(0x120))
            cpu.mem_write(physical(CONTEXT), bytes(0x30))
            cpu.mem_write(physical(REFRESHED), bytes(0x30))
            write_word(cpu, stack + 0x28, CONTEXT)
            for word in range(4):
                write_word(cpu, stack + 0x40 + 4 * word, entry_seed ^ 0xA5A50000 ^ word)
            write_byte(cpu, CONTEXT + 0x12, blocked_12)
            write_byte(cpu, CONTEXT + 0x2B, blocked_2b)
            write_byte(cpu, CONTEXT + 0x29, blocked_29)
            write_byte(cpu, CONTEXT + 0x13, toggle)
            write_byte(cpu, REFRESHED + 0x13, refreshed_toggle)
            write_word(cpu, FLAGS_POINTER, FLAGS)
            write_byte(cpu, FLAGS + 0x2A2A, flags)
            write_byte(cpu, HOLD, hold)
            write_byte(cpu, TIMEOUT, timeout)
            write_byte(cpu, ACTION, action)
            write_word(cpu, LIFECYCLE, lifecycle)
            write_word(cpu, CURSOR, cursor)
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.reg_write(UC_MIPS_REG_S2, mode & 0xFF)
            cpu.reg_write(UC_MIPS_REG_S3, buttons)
            cpu.emu_start(0x80024840, 0, count=5000)
            self.assertIn(current["stop"], (0x80024A70, 0x80024BD4), f"case {index}: {entry}")
            calls = current["calls"] + [EMPTY_VALUE] * (10 - len(current["calls"]))
            original = (
                int(current["stop"] == 0x80024A70), read_byte(cpu, CONTEXT + 0x29),
                read_byte(cpu, CONTEXT + 0x13), read_byte(cpu, REFRESHED + 0x13),
                read_byte(cpu, HOLD), read_byte(cpu, TIMEOUT), read_byte(cpu, ACTION),
                read_word(cpu, LIFECYCLE), read_word(cpu, CURSOR), read_byte(cpu, FLAGS + 0x2A2A),
                current["interval_calls"], *current["entry_words"], len(current["calls"]), *calls,
            )
            self.assertEqual(original, native, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
