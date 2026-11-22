import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import (
    BITMAP_STATE, GP, HAS_UNICORN, MAPPING_ROW, ORIGINAL, ROW_SELECTOR, STACK, physical,
)

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3, UC_MIPS_REG_PC,
        UC_MIPS_REG_RA, UC_MIPS_REG_S2, UC_MIPS_REG_S3, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<30I")
OUTPUT = struct.Struct("<88I")
EVENT = 0x801E5000
CONTEXT = 0x801E4000
FLAGS = 0x801E0000
SECOND_PORT = 0x801D5F7C
TIMEOUT = GP + 0x1A2
HOLD = GP + 0x1A6
LIFECYCLE = 0x801D5FB4
PULSE_VALUE = GP + 0x704
PULSE_DIRECTION = GP + 0x708
PULSE_SELECTION = GP + 0x1A5
PROMPT = GP + 0x710
TAG_A = GP + 0x714
TAG_B = GP + 0x718
BUTTON_TAG = GP + 0x71C
DISPLAY_TAG_A = PROMPT
DISPLAY_TAG_B = TAG_A
DISPLAY_TAG_C = TAG_B
PRESSED = 0x8009EEF0
RELEASED = 0x8009EFF0
EMPTY_VALUE = 0xFFFFFFFF


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerGroundedInputPathTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_preparation_through_bitmap_dispatch_against_original(self):
        generator = random.Random(0x23E9424A70)
        cases = []
        for buttons, port, selector, timeout in itertools.product(
            (0, 1 << 13, 1 << 15, 1 << 8, 1 << 11, 0x8900, 0xABCD1234),
            (0, 1, 2), (0, 1), (0, 1),
        ):
            cases.append((buttons, 1, 1, 1, 1, 0, port, selector, 2, 0, 0, 0, timeout,
                          0, 0, 0, 0xA5A50000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0))
        cases.extend(
            (
                generator.getrandbits(32), generator.choice((0, 1, 2)),
                generator.choice((0, 1, 2)), generator.choice((0, 1, 2)),
                generator.choice((0, 1, 2)), generator.choice((0, 1, 2)),
                generator.choice((0, 1, 2)), generator.randrange(2),
                generator.choice((0, 2, 3, 255)), generator.getrandbits(32),
                generator.choice((0, 0, 1, 2)), generator.getrandbits(32),
                generator.choice((0, 0, 1, 255)), generator.choice((0, 0, 0, 1)),
                generator.choice((0, 0, 0, 1)), generator.choice((0, 1, 60, 61, 255)),
                generator.getrandbits(32), generator.randrange(512), generator.getrandbits(32),
                generator.randrange(256), generator.randrange(256), generator.randrange(256),
                generator.randrange(256), generator.randrange(256), generator.randrange(256),
                generator.randrange(256), 0, generator.randrange(2),
            )
            for _ in range(5000)
        )
        cases.extend(
            (
                0 if index < 1000 else generator.getrandbits(32), 1, 1, 1, 1,
                generator.choice((0, 1, 2)), generator.choice((0, 1, 2)), generator.randrange(2),
                1, 0, 0, 0, generator.choice((0, 0, 1, 255)), generator.choice((0, 0, 1)),
                generator.choice((0, 0, 1)), generator.choice((0, 1, 61, 255)),
                generator.getrandbits(32), generator.randrange(16) * 64, 0,
                generator.choice((0, 1, 2, 4, 255)) if index < 1000 else 1,
                generator.randrange(256), generator.randrange(256), generator.randrange(256),
                generator.randrange(256), generator.randrange(256), generator.randrange(256),
                0, generator.randrange(2),
            )
            for index in range(1500)
        )
        cases.extend(
            (
                buttons, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
                0, mutations, 0, state, generator.randrange(256), generator.randrange(2),
                generator.randrange(2), generator.randrange(256), generator.randrange(256),
                generator.randrange(256), 0, 0,
            )
            for buttons, state, mutations in itertools.product(
                (1 << 14, 1 << 12, (1 << 14) | (1 << 12), (1 << 14) | (1 << 15)),
                (0, 2, 4), (0, 2048),
            )
        )
        cases.extend(
            (
                1 << 11, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, state, 128, 1, 0, 0x80, 0xFF, 0, interval, flash,
            )
            for state, interval, flash in itertools.product((0, 2, 4), (0, 1), (0, 1))
        )
        cases.extend(
            (
                1 << 8, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, state, 128, 1, 0, 0x80, 0xFF, 0, interval, flash,
            )
            for state, flash, interval in itertools.product((0, 2, 4), (0, 1), (0, 1))
            if flash == 0 or interval == 0
        )
        cases.extend(
            (
                1 << 13, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, state, 128, 1, 0, 0x80, 0xFF, 0, 0, 0,
            )
            for state in (0, 2, 4)
        )
        cases.extend(
            (
                1 << 8, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, state, 32, 1, 0, 0x80, 0xFF, button_tag, 1, 1,
            )
            for state, button_tag in ((0, 0x12), (2, 0x80), (4, 0xFF))
        )
        cases.extend(
            (
                buttons, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, state, 128, 1, selection, 0x80, 0xFF, 0, interval, 0,
            )
            for buttons, interval, selection, state in (
                (1 << 6, 0, 0, 0), (1 << 6, 0, 1, 0), (1 << 6, 1, 1, 0),
                (1 << 11, 0, 0, 0), (1 << 11, 0, 1, 0), (1 << 11, 1, 1, 0),
                (1 << 6, 1, 0, 0), (1 << 11, 1, 0, 0),
                (1 << 0, 0, 0, 0), (1 << 8, 0, 0, 0),
                (1 << 13, 0, 0, 0), (1 << 15, 0, 0, 0),
                (1 << 6, 1, 1, 2), (1 << 6, 1, 0, 2),
            )
        )
        cases.extend(
            (
                1 << bit, 1, 1, 1, 1, pulse_enabled, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 128, 1, 0, 0x80, 0xFF, 0, 0, 0,
            )
            for pulse_enabled, bit in itertools.product((0, 1), range(32))
        )
        cases = [(*case, 0, 0) for case in cases]
        cases.extend(
            (
                1 << 11, 1, 1, 1, 1, 0, 0, 0, mode, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 128, 1, 0, 0x80, 0xFF, 0, 0, flash,
                interval, ready,
            )
            for mode, flash, interval, ready in itertools.product(
                (0, 2), (0, 1, 255), (0, 1), (0, 1, 256, 257),
            )
        )
        cases.extend(
            (
                1 << 11, 1, 1, 1, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0,
                0, mutations, 0, 0, 128, 1, 0, 0x80, 0xFF, 0, 0, 255, 1, ready,
            )
            for mutations, ready in itertools.product((4096, 8192, 12288), (0, 1))
        )
        native_outputs = run_native_probe(
            self, [
                "src/game/controller_input_prepare.c", "src/game/controller_input_route.c",
                "src/game/controller_bitmap_gate.c", "src/game/controller_bitmap_dispatch.c",
                "src/game/controller_mode_one_tail.c", "src/game/controller_grounded_input_path.c",
                "src/game/func_80056678.c", "src/game/func_8005668C.c",
                "tests/test_controller_grounded_input_path.c",
            ],
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

        def flip_buttons(uc, mask):
            write_word(uc, EVENT + 0x0C, read_word(uc, EVENT + 0x0C) ^ mask)

        def intercept(uc, address, size, data):
            if address == 0x80024840:
                current["mode"] = uc.reg_read(UC_MIPS_REG_S2) & 0xFF
            elif address == 0x800245A4:
                current["tail_started"] = True
            elif address == 0x80024178:
                current["mode_one_button_branch"] = True
            elif address == 0x8002442C:
                current["mode_one_button_branch"] = True
            elif address == 0x80024A70:
                current["dispatched"] = True
            elif address == 0x80024BD4:
                current["stopped"] = True
                uc.emu_stop()
                return

            call = {
                0x8003AA1C: 1, 0x8001BCE4: 2, 0x8001AE88: 3, 0x8001BCF0: 4,
                0x8001BD00: 5, 0x8001C04C: 6, 0x80049454: 7, 0x80047050: 8,
                0x8001B558: 9, 0x80023A30: 10, 0x800486D0: 11,
                0x800483C0: 12, 0x800482D0: 13, 0x80023C98: 14,
                0x80048504: 15, 0x80048C48: 16, 0x8001BC08: 17,
                0x8002C9D4: 18, 0x800481D4: 19, 0x8001BC28: 20,
                0x8002C370: 21, 0x80027114: 22,
                0x80062890: 23, 0x800222E0: 24, 0x80023A84: 25,
            }.get(address)
            if call is None:
                return
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            a3 = uc.reg_read(UC_MIPS_REG_A3) & 0xFFFFFFFF
            mutations = current["mutations"]
            if call == 1:
                write_word(uc, a0, CONTEXT)
                uc.reg_write(UC_MIPS_REG_V0, 1)
                if mutations & 1:
                    flip_buttons(uc, 0x11111111)
                    write_byte(uc, SECOND_PORT, read_byte(uc, SECOND_PORT) ^ 1)
            elif call == 2:
                if mutations & 2:
                    selector = struct.unpack("<H", uc.mem_read(physical(ROW_SELECTOR), 2))[0]
                    uc.mem_write(physical(ROW_SELECTOR), struct.pack("<H", selector ^ 1))
                uc.reg_write(UC_MIPS_REG_V0, current["live_mode"])
            elif call == 3:
                write_word(uc, a0, current["sample"])
                if mutations & 4:
                    write_word(uc, EVENT + 0x0C, current["replacement"])
            elif call == 4:
                uc.reg_write(UC_MIPS_REG_V0, current["timeout_gate"])
            elif call == 5:
                if mutations & 8:
                    flip_buttons(uc, 0x80000000)
                uc.reg_write(UC_MIPS_REG_V0, current["reference"])
            elif call == 6:
                self.assertEqual(read_byte(uc, TIMEOUT), 40)
                current["timed_out"] = True
                if mutations & 32:
                    write_byte(uc, TIMEOUT, read_byte(uc, TIMEOUT) ^ 0x7F)
                    flip_buttons(uc, 0x33333333)
                uc.reg_write(UC_MIPS_REG_V0, 0)
            elif call == 7:
                current["secondary"] = a0
                if mutations & 16:
                    flip_buttons(uc, 0x22222222)
            elif call == 8:
                if current["mode_one_enabled_pulse_transition"]:
                    expected = (0, 3)
                elif current["mode_one_enabled_pulse_branch"]:
                    expected = (1, 1)
                elif current["mode_one_enabled_pulse_cleanup"] and current["mode_state"] != 0:
                    expected = (0, 2)
                else:
                    expected = (1, 2)
                self.assertEqual((a0, read_word(uc, LIFECYCLE)), expected)
                if mutations & 128:
                    write_byte(uc, HOLD, 255)
            elif call == 22:
                self.assertEqual((a0, a1), (0, 1))
                current["ui_calls"].append(5)
            elif call == 9:
                stack = uc.reg_read(UC_MIPS_REG_SP)
                current["events"].append(a0)
                current["snapshots"].append(read_word(uc, BITMAP_STATE))
                if a1 == 5:
                    self.assertEqual((a2, a3), (0xFFFF, 0xFFFF))
                    if a0 == 5:
                        data_a = read_word(uc, stack + 0x10)
                        data_b = read_word(uc, stack + 0x14)
                        self.assertEqual((read_word(uc, data_a), read_word(uc, data_b)), (0x200, 0x1E0))
                    else:
                        self.assertEqual(a0, 1)
                        self.assertEqual(
                            tuple(read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C)),
                            (0, 0, 0, 0),
                        )
                    if mutations & 256:
                        write_byte(uc, HOLD, read_byte(uc, HOLD) ^ 0x55)
                else:
                    self.assertEqual(
                        tuple(read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C)),
                        (0, 0, 0, 0),
                    )
                    self.assertEqual((a1, a2, a3), (4, 0xFFFE, 0))
                    if mutations & 64 and len(current["events"]) == 1:
                        write_word(uc, BITMAP_STATE, read_word(uc, BITMAP_STATE) | (1 << 5))
                        uc.mem_write(physical(ROW_SELECTOR), struct.pack("<H", 1))
                        write_word(uc, MAPPING_ROW + 4, 2)
                        write_word(uc, PRESSED + (2 * 16 + 4) * 4, 0x1F1)
                        write_word(uc, RELEASED + (2 * 16 + 5) * 4, 0x222)
            elif call == 10:
                if a0 == 30:
                    current["interval_30_calls"] += 1
                    uc.reg_write(UC_MIPS_REG_V0, current["interval_30"])
                else:
                    self.assertEqual(a0, 60)
                    current["gate_interval_calls"] += 1
                    uc.reg_write(UC_MIPS_REG_V0, current["interval_60"])
            elif call == 11:
                if current["ui_setup"]:
                    current["ui_calls"].append(5)
                    expected = (
                        (0x80012128, 160, 180, DISPLAY_TAG_A),
                        (0x800BDF08, 240, 230, DISPLAY_TAG_B),
                        (0x800BDF0C, 240, 275, DISPLAY_TAG_C),
                    )[current["ui_prompt_index"]]
                    self.assertEqual((a0, a1, a2, a3), expected)
                    write_byte(uc, a3, current["prompt"] ^ 0x55)
                    current["ui_prompt_index"] += 1
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                    return
                current["tail_calls"].append(1)
                self.assertEqual((a0, a1, a2, a3), (0x80012134, 180, 100, PROMPT))
                write_byte(uc, PROMPT, current["prompt"] ^ 0x55)
                if mutations & 512:
                    write_byte(uc, CONTEXT + 0x6D, 9)
                    write_byte(uc, GP + 0x1A3, read_byte(uc, GP + 0x1A3) ^ 1)
                    write_byte(uc, PULSE_VALUE, read_byte(uc, PULSE_VALUE) ^ 0x80)
            elif call == 12:
                if current["ui_setup"]:
                    current["ui_calls"].append(7)
                    current["prompt_call_tag"] = a0
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                    return
                current["tail_calls"].append(2)
                current["prompt_call_tag"] = a0
                if mutations & 1024:
                    write_byte(uc, GP + 0x1A3, read_byte(uc, GP + 0x1A3) ^ 1)
                    write_byte(uc, PULSE_DIRECTION, read_byte(uc, PULSE_DIRECTION) ^ 1)
                    write_byte(uc, PULSE_SELECTION, read_byte(uc, PULSE_SELECTION) ^ 1)
            elif call == 14:
                current["interval_callback_count"] += 1
            elif call == 17:
                current["ui_calls"].append(3)
            elif call == 18:
                current["ui_calls"].append(4)
                self.assertEqual(a0, 0 if current["mode_one_enabled_pulse_cleanup"] else 1)
            elif call == 19:
                current["ui_calls"].append(6)
                self.assertEqual((a1, a2), (0x1000, 0x1000))
                expected_tags = tuple(
                    read_byte(uc, address)
                    for address in (DISPLAY_TAG_A, DISPLAY_TAG_B, DISPLAY_TAG_C)
                )
                expected_tag = expected_tags[current["ui_color_index"]]
                expected_tag = expected_tag if expected_tag < 128 else expected_tag - 256
                self.assertEqual(a0, expected_tag & 0xFFFFFFFF)
                current["ui_color_index"] += 1
            elif call == 20:
                current["ui_calls"].append(8)
            elif call == 21:
                current["ui_calls"].append(10)
            elif call == 23:
                current["resource_clear_calls"] += 1
                uc.reg_write(UC_MIPS_REG_V0, 0)
            elif call == 24:
                current["ready_calls"] += 1
                if mutations & 4096:
                    write_byte(uc, GP + 0x1A4, read_byte(uc, GP + 0x1A4) ^ 0x55)
                uc.reg_write(UC_MIPS_REG_V0, current["ready"])
            elif call == 25:
                self.assertEqual(read_byte(uc, GP + 0x1A4), 1)
                current["action_calls"] += 1
                if mutations & 8192:
                    write_byte(uc, GP + 0x1A4, read_byte(uc, GP + 0x1A4) ^ 0x80)
            elif call == 15 or call == 16:
                expected_tags = (
                    (current["prompt"], current["tag_a"], current["tag_b"])
                    if current["mode_one_enabled_pulse_cleanup"] else (current["prompt"],)
                )
                expected_tag = expected_tags[min(current["ui_tag_index"] // 2, len(expected_tags) - 1)]
                expected_tag = expected_tag if expected_tag < 128 else expected_tag - 256
                self.assertEqual(a0, expected_tag & 0xFFFFFFFF)
                current["ui_calls"].append(1 if call == 15 else 2)
                current["ui_tag_index"] += 1
            else:
                current["tail_calls"].append(3)
                color = uc.mem_read(physical(a1), 3)
                self.assertEqual(color[0], color[1])
                self.assertEqual(color[1], color[2])
                current["color_call_tag"] = a0
                current["color_value"] = color[0]
                if mutations & 2048:
                    write_byte(uc, PULSE_VALUE, read_byte(uc, PULSE_VALUE) ^ 0xAA)
                    write_byte(uc, PULSE_SELECTION, read_byte(uc, PULSE_SELECTION) ^ 1)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        outcomes = [0, 0, 0, 0, 0]
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            (buttons, status_1, status_2, latch_1, latch_2, mirror, port, selector,
             mode, sample, timeout_gate, reference, timeout, blocked_12, blocked_2b,
             hold, bitmap, mutations, replacement, mode_state, pulse_value,
             pulse_direction, pulse_selection, tag_a, tag_b, prompt, interval_30,
             flash_enabled, interval_60, ready) = entry
            current.update(
                mutations=mutations, live_mode=mode, sample=sample, timeout_gate=timeout_gate,
                mode_state=mode_state,
                reference=reference, replacement=replacement, secondary=EMPTY_VALUE,
                events=[], snapshots=[], mode=None, timed_out=False, dispatched=False, stopped=False,
                tail_started=False, tail_calls=[], prompt=prompt, prompt_call_tag=EMPTY_VALUE,
                color_call_tag=EMPTY_VALUE, color_value=EMPTY_VALUE,
                mode_one_button_branch=False,
                interval_30=interval_30, interval_30_calls=0, interval_callback_count=0,
                interval_60=interval_60, ready=ready, gate_interval_calls=0,
                ready_calls=0, action_calls=0,
                ui_calls=[], ui_prompt_index=0,
                ui_color_index=0, tag_a=tag_a, tag_b=tag_b,
                ui_tag_index=0, resource_clear_calls=0,
                ui_setup=(mode == 1 and (buttons & (1 << 8)) != 0 and mirror == 0
                          and flash_enabled != 0 and interval_30 != 0
                          and mode_state not in (1, 3)),
                mode_one_enabled_pulse_cleanup=(
                    mode == 1 and (buttons & ((1 << 6) | (1 << 11))) != 0
                    and (buttons & ((1 << 14) | (1 << 12))) == 0
                    and mirror != 0 and pulse_selection == 0 and mode_state not in (1, 3)
                    and interval_30 != 0
                ),
                mode_one_enabled_pulse_branch=(
                    mode == 1 and (buttons & ((1 << 6) | (1 << 11))) != 0
                    and (buttons & ((1 << 14) | (1 << 12))) == 0
                    and mirror != 0 and pulse_selection != 0 and mode_state == 0
                    and interval_30 != 0
                ),
                mode_one_enabled_pulse_transition=(
                    mode == 1 and (buttons & ((1 << 6) | (1 << 11))) != 0
                    and (buttons & ((1 << 14) | (1 << 12))) == 0
                    and mirror != 0 and pulse_selection != 0 and mode_state not in (0, 1, 3)
                    and interval_30 != 0
                ),
            )
            stack = STACK - 0x120
            cpu.mem_write(physical(stack), bytes(0x120))
            cpu.mem_write(physical(EVENT), bytes(28))
            cpu.mem_write(physical(CONTEXT), bytes(0x70))
            write_word(cpu, EVENT + 0x0C, buttons)
            write_byte(cpu, EVENT + 0x10, status_1)
            write_byte(cpu, EVENT + 0x11, status_2)
            write_byte(cpu, GP + 0x1A7, latch_1)
            write_byte(cpu, GP + 0x1A8, latch_2)
            write_byte(cpu, GP + 0x1A3, mirror)
            write_byte(cpu, GP + 0x1A4, flash_enabled)
            cpu.mem_write(physical(ROW_SELECTOR), struct.pack("<H", selector))
            write_byte(cpu, TIMEOUT, timeout)
            write_byte(cpu, HOLD, hold)
            write_byte(cpu, CONTEXT + 0x12, blocked_12)
            write_byte(cpu, CONTEXT + 0x2B, blocked_2b)
            write_byte(cpu, CONTEXT + 0x6D, mode_state)
            write_byte(cpu, PULSE_VALUE, pulse_value)
            write_byte(cpu, PULSE_DIRECTION, pulse_direction)
            write_byte(cpu, PULSE_SELECTION, pulse_selection)
            write_byte(cpu, TAG_A, tag_a)
            write_byte(cpu, TAG_B, tag_b)
            write_byte(cpu, PROMPT, prompt)
            write_byte(cpu, BUTTON_TAG, prompt)
            write_word(cpu, 0x800BE698, FLAGS)
            write_byte(cpu, FLAGS + 0x52, 0)
            write_byte(cpu, FLAGS + 0x2A2A, 0)
            write_word(cpu, LIFECYCLE, 9)
            write_word(cpu, BITMAP_STATE, bitmap)
            cpu.mem_write(physical(MAPPING_ROW), struct.pack("<4i", 0, 1, 2, 3))
            write_byte(cpu, SECOND_PORT, port)
            pressed = [0 if (row + bit) % 3 == 0 else 0x100 + row * 32 + bit
                       for row in range(4) for bit in range(16)]
            released = [0 if (row + 2 * bit) % 4 == 0 else 0x200 + row * 32 + bit
                        for row in range(4) for bit in range(16)]
            cpu.mem_write(physical(PRESSED), struct.pack("<64I", *pressed))
            cpu.mem_write(physical(RELEASED), struct.pack("<64I", *released))
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_A0, EVENT)
            cpu.reg_write(UC_MIPS_REG_S2, EMPTY_VALUE)
            cpu.reg_write(UC_MIPS_REG_S3, EMPTY_VALUE)
            cpu.emu_start(0x80023E94, 0, count=10000)
            self.assertTrue(current["stopped"], f"case {index}: {entry}")
            outcome = (
                0 if current["timed_out"] else 2 if current["dispatched"]
                else 4 if current["tail_calls"] or (current["tail_started"] and current["mode"] is None)
                else 4 if current["mode_one_button_branch"] else 1
            )
            outcomes[outcome] += 1
            observed_mode = current["mode"]
            if observed_mode is None:
                observed_mode = cpu.reg_read(UC_MIPS_REG_S2) & 0xFF
            events = current["events"] + [EMPTY_VALUE] * (16 - len(current["events"]))
            snapshots = current["snapshots"] + [EMPTY_VALUE] * (16 - len(current["snapshots"]))
            tail_calls = current["tail_calls"] + [EMPTY_VALUE] * (3 - len(current["tail_calls"]))
            ui_calls = current["ui_calls"] + [EMPTY_VALUE] * (16 - len(current["ui_calls"]))
            original = (
                outcome, read_word(cpu, EVENT + 0x0C), cpu.reg_read(UC_MIPS_REG_S3) & 0xFFFFFFFF,
                observed_mode, read_byte(cpu, GP + 0x1A7), read_byte(cpu, GP + 0x1A8),
                read_byte(cpu, SECOND_PORT), struct.unpack("<H", cpu.mem_read(physical(ROW_SELECTOR), 2))[0],
                read_byte(cpu, TIMEOUT), read_byte(cpu, HOLD), read_word(cpu, BITMAP_STATE),
                read_word(cpu, MAPPING_ROW + 4), read_word(cpu, PRESSED + (2 * 16 + 4) * 4),
                read_word(cpu, RELEASED + (2 * 16 + 5) * 4), current["secondary"],
                len(current["events"]), read_word(cpu, LIFECYCLE), *events, *snapshots,
                read_byte(cpu, CONTEXT + 0x6D), read_byte(cpu, GP + 0x1A3),
                read_byte(cpu, PULSE_VALUE), read_byte(cpu, PULSE_DIRECTION),
                read_byte(cpu, PULSE_SELECTION), read_byte(cpu, PROMPT), current["prompt_call_tag"],
                current["color_call_tag"], current["color_value"], len(current["tail_calls"]), *tail_calls,
                read_byte(cpu, GP + 0x1A4), current["interval_callback_count"],
                read_byte(cpu, FLAGS + 0x52), len(current["ui_calls"]), *ui_calls,
                read_byte(cpu, DISPLAY_TAG_A), read_byte(cpu, DISPLAY_TAG_B),
                read_byte(cpu, DISPLAY_TAG_C), current["gate_interval_calls"],
                current["ready_calls"], current["action_calls"],
            )
            self.assertEqual(original, native, f"case {index}: {entry}")
            self.assertEqual(
                current["resource_clear_calls"],
                int(current["mode_one_enabled_pulse_transition"]
                    or (current["mode_one_enabled_pulse_cleanup"] and mode_state != 0)),
                f"resource cleanup calls for case {index}: {entry}",
            )
        self.assertTrue(all(outcomes[index] for index in (0, 1, 2, 4)),
                        f"missing an integrated outcome: {outcomes}")


if __name__ == "__main__":
    unittest.main()
