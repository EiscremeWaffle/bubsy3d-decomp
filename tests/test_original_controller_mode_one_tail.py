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
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_S2, UC_MIPS_REG_SP,
    )


INPUT = struct.Struct("<10I")
OUTPUT = struct.Struct("<43I")
CONTEXT = 0x801E4000
ENABLED = GP + 0x1A3
VALUE = GP + 0x704
DIRECTION = GP + 0x708
SELECTION = GP + 0x1A5
PROMPT = GP + 0x710
TAG_A = GP + 0x714
TAG_B = GP + 0x718
EMPTY_VALUE = 0xFFFFFFFF


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerModeOneTailTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_mode_one_tail_against_original(self):
        generator = random.Random(0x245A424838)
        cases = [
            (1, 0, enabled, value, direction, selection, 0x7F, 0x80, 0xFF, 0)
            for value, direction, selection, enabled in itertools.product(
                range(256), (0, 1, 2, 255), (0, 1), (0, 1),
            )
        ]
        cases.extend(
            (mode, state, enabled, value, direction, selection, 0x80, 0xFF, 0x7F, 0)
            for mode, state, enabled, value, direction, selection in itertools.product(
                (0, 1, 2, 255), (0, 1, 2, 3, 4, 255), (0, 1, 2),
                (0, 32, 33, 127, 128, 255), (0, 1, 255), (0, 1),
            )
        )
        cases.extend(
            (
                generator.choice((0, 1, 1, 1, 2, 255)), generator.choice((0, 1, 2, 3, 4, 255)),
                generator.choice((0, 1, 2, 255)), generator.randrange(256), generator.randrange(256),
                generator.randrange(256), generator.randrange(256), generator.randrange(256),
                generator.randrange(256), generator.randrange(8),
            )
            for _ in range(3000)
        )
        native_outputs = run_native_probe(
            self, ["src/game/controller_mode_one_tail.c", "tests/test_controller_mode_one_tail.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_byte(uc, address):
            return uc.mem_read(physical(address), 1)[0]

        def read_word(uc, address):
            return struct.unpack("<I", uc.mem_read(physical(address), 4))[0]

        def write_word(uc, address, value):
            uc.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def write_byte(uc, address, value):
            uc.mem_write(physical(address), bytes((value & 0xFF,)))

        def intercept(uc, address, size, data):
            if address in (0x80024840, 0x80024BD4):
                current["stop"] = address
                uc.emu_stop()
                return
            call = {
                0x800486D0: 1, 0x800483C0: 2, 0x800482D0: 3,
                0x80048504: 4, 0x80048C48: 5, 0x80027F40: 6,
                0x80061A64: 7, 0x8001B558: 8, 0x8006BB84: 9,
                0x8004F688: 10, 0x80027114: 11,
            }.get(address)
            if call is None:
                return
            if call == 1 and current["state"] == 3:
                current["calls"].append(12 + current["display_index"])
            else:
                current["calls"].append(call)
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            a3 = uc.reg_read(UC_MIPS_REG_A3) & 0xFFFFFFFF
            mutations = current["mutations"]
            if call == 1:
                if current["state"] == 3:
                    expected = (
                        (0x80012140, 305, 35, TAG_A),
                        (0x800BDF18, 345, 60, TAG_B),
                    )[current["display_index"]]
                    self.assertEqual((a0, a1, a2, a3), expected)
                    current["display_index"] += 1
                else:
                    self.assertEqual((a0, a1, a2, a3), (0x80012134, 180, 100, PROMPT))
                write_byte(uc, a3, current["prompt"] ^ 0x55)
                if current["state"] != 3 and mutations & 1:
                    write_byte(uc, CONTEXT + 0x6D, 9)
                    write_byte(uc, ENABLED, read_byte(uc, ENABLED) ^ 1)
                    write_byte(uc, VALUE, read_byte(uc, VALUE) ^ 0x80)
            elif call == 2:
                current["prompt_call_tag"] = a0
                if mutations & 2:
                    write_byte(uc, ENABLED, read_byte(uc, ENABLED) ^ 1)
                    write_byte(uc, DIRECTION, read_byte(uc, DIRECTION) ^ 1)
                    write_byte(uc, SELECTION, read_byte(uc, SELECTION) ^ 1)
            elif call == 3:
                current["color_call_tag"] = a0
                current["color"] = list(uc.mem_read(physical(a1), 3))
                if mutations & 4:
                    write_byte(uc, VALUE, read_byte(uc, VALUE) ^ 0xAA)
                    write_byte(uc, SELECTION, read_byte(uc, SELECTION) ^ 1)
            elif call == 4 or call == 5:
                self.assertEqual(a0, (current["prompt"] if current["prompt"] < 128
                                      else current["prompt"] - 256) & 0xFFFFFFFF)
            elif call == 7:
                self.assertEqual(a0, 30)
                return
            elif call == 8:
                self.assertEqual((a0, a1, a2, a3), (4, 5, 0xFFFF, 0xFFFF))
                event_pointer = read_word(uc, uc.reg_read(UC_MIPS_REG_SP) + 0x10)
                self.assertEqual(
                    struct.unpack("<2I", uc.mem_read(physical(event_pointer), 8)),
                    (read_word(uc, GP + 0x1B4), read_word(uc, GP + 0x1B8)),
                )
                self.assertEqual(uc.reg_read(UC_MIPS_REG_SP), current["stack"])
            elif call == 9:
                self.assertEqual(a0, 0)
            elif call == 11:
                self.assertEqual((a0, a1), (1, 1))
            if call == 7:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            mode, state, enabled, value, direction, selection, tag_a, tag_b, prompt, mutations = entry
            current.update(
                mutations=mutations, prompt=prompt, state=state, calls=[], color=[EMPTY_VALUE] * 3,
                prompt_call_tag=EMPTY_VALUE, color_call_tag=EMPTY_VALUE, stop=None,
                display_index=0, stack=STACK - 0x120,
            )
            stack = STACK - 0x120
            cpu.mem_write(physical(stack), bytes(0x120))
            cpu.mem_write(physical(CONTEXT), bytes(0x70))
            cpu.mem_write(physical(stack + 0x28), struct.pack("<I", CONTEXT))
            write_byte(cpu, CONTEXT + 0x6D, state)
            write_byte(cpu, ENABLED, enabled)
            write_byte(cpu, VALUE, value)
            write_byte(cpu, DIRECTION, direction)
            write_byte(cpu, SELECTION, selection)
            write_byte(cpu, PROMPT, prompt)
            write_byte(cpu, TAG_A, tag_a)
            write_byte(cpu, TAG_B, tag_b)
            write_word(cpu, GP + 0x1B4, tag_a)
            write_word(cpu, GP + 0x1B8, tag_b)
            write_word(cpu, 0x800BE400, 42)
            write_byte(cpu, 0x800BE680, 0x5A)
            write_word(cpu, 0x800BE698, 0x801E0000)
            write_byte(cpu, 0x801E0052, 0)
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.reg_write(UC_MIPS_REG_S2, mode & 0xFF)
            cpu.emu_start(0x800245A4, 0, count=1000)
            self.assertIn(current["stop"], (0x80024840, 0x80024BD4), f"case {index}: {entry}")
            outcome = {0x80024840: 0, 0x80024BD4: 1}[current["stop"]]
            calls = current["calls"] + [EMPTY_VALUE] * (24 - len(current["calls"]))
            original = (
                outcome, read_byte(cpu, CONTEXT + 0x6D), read_byte(cpu, ENABLED), read_byte(cpu, VALUE),
                read_byte(cpu, DIRECTION), read_byte(cpu, SELECTION), read_byte(cpu, TAG_A),
                read_byte(cpu, TAG_B), read_byte(cpu, PROMPT), current["prompt_call_tag"],
                current["color_call_tag"], *current["color"], len(current["calls"]), *calls,
                read_byte(cpu, 0x801E0052), read_word(cpu, 0x800BE400),
                read_byte(cpu, 0x800BE680), read_byte(cpu, CONTEXT + 0x2B),
            )
            self.assertEqual(original, native, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
