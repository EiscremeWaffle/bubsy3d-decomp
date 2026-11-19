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
        UC_MIPS_REG_A0, UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_S0,
        UC_MIPS_REG_S1, UC_MIPS_REG_S2, UC_MIPS_REG_S3, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<16I")
OUTPUT = struct.Struct("<21I")
EVENT = 0x801E5000
CONTEXT = 0x801E4000
SECOND_PORT = 0x801D5F7C
PORT_SELECTION = 0x801D5F84
TIMEOUT_TICKS = GP + 0x1A2
EMPTY_VALUE = 0xFFFFFFFF


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerInputRouteTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_controller_timeout_and_port_selection_against_original(self):
        generator = random.Random(0x24068)
        cases = []
        references = (0, 1, 0x7FFFFF9B, 0x7FFFFFFF, 0x80000000, 0xFFFFFF9B, 0xFFFFFFFF)
        for reference in references:
            threshold = (reference + 100) & 0xFFFFFFFF
            samples = (threshold, (threshold - 1) & 0xFFFFFFFF, (threshold + 1) & 0xFFFFFFFF, 0)
            for sample, buttons, gate, second_port, selector in itertools.product(
                samples, (0, 1, 0x80000000, 0xABCD1234), (0, 1), (0, 1, 2), (0, 1, 0xFFFF)
            ):
                cases.append((buttons, 3, sample, gate, reference, second_port, selector, 7, 0, 0,
                              len(cases) & 1, 1, 1, 0, 0, 0))
        cases.extend(
            (generator.getrandbits(32), generator.randrange(256), generator.getrandbits(32),
             generator.choice((0, 1, 2, 255)), generator.getrandbits(32),
             generator.choice((0, 1, 2, 255)), generator.choice((0, 1, 2, 0x7FFF, 0x8000, 0xFFFF)),
             generator.randrange(256), generator.randrange(64), generator.getrandbits(32),
             generator.randrange(2), *(generator.choice((0, 1, 2, 255)) for _ in range(5)))
            for _ in range(5000)
        )
        native_outputs = run_native_probe(
            self, [
                "src/game/controller_input_route.c", "src/game/controller_input_prepare.c",
                "src/game/func_80056678.c", "src/game/func_8005668C.c",
                "tests/test_controller_input_route.c",
            ],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_word(uc, address):
            return struct.unpack("<I", uc.mem_read(physical(address), 4))[0]

        def write_word(uc, address, value):
            uc.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def update_buttons(uc, mask):
            write_word(uc, EVENT + 0x0C, read_word(uc, EVENT + 0x0C) ^ mask)

        def intercept(uc, address, size, data):
            if address in (0x8002413C, 0x80024BD4):
                current["stop"] = address
                uc.emu_stop()
                return
            call = {
                0x8003AA1C: 1, 0x8001BCE4: 2, 0x8001AE88: 3, 0x8001BCF0: 4,
                0x8001BD00: 5, 0x8001C04C: 6, 0x80049454: 7,
            }.get(address)
            if call is None:
                return
            current["calls"].append(call)
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            if call == 1:
                write_word(uc, a0, CONTEXT)
                uc.reg_write(UC_MIPS_REG_V0, 1)
                if current["mutations"] & 1:
                    update_buttons(uc, 0x11111111)
                    value = uc.mem_read(physical(SECOND_PORT), 1)[0]
                    uc.mem_write(physical(SECOND_PORT), bytes((value ^ 1,)))
            elif call == 2:
                uc.reg_write(UC_MIPS_REG_V0, current["live_mode"])
                if current["mutations"] & 2:
                    value, = struct.unpack("<H", uc.mem_read(physical(PORT_SELECTION), 2))
                    uc.mem_write(physical(PORT_SELECTION), struct.pack("<H", value ^ 1))
            elif call == 3:
                write_word(uc, a0, current["sample"])
                if current["mutations"] & 4:
                    write_word(uc, EVENT + 0x0C, current["replacement"])
            elif call == 4:
                uc.reg_write(UC_MIPS_REG_V0, current["gate"] & 0xFF)
            elif call == 5:
                uc.reg_write(UC_MIPS_REG_V0, current["reference"])
                if current["mutations"] & 8:
                    update_buttons(uc, 0x80000000)
                    uc.mem_write(physical(SECOND_PORT), bytes(((current["second_port"] ^ 1) & 0xFF,)))
            elif call == 6:
                self.assertEqual(uc.mem_read(physical(TIMEOUT_TICKS), 1)[0], 40)
                if current["mutations"] & 32:
                    uc.mem_write(physical(TIMEOUT_TICKS), bytes((40 ^ 0x7F,)))
                    update_buttons(uc, 0x33333333)
                uc.reg_write(UC_MIPS_REG_V0, 0)
            else:
                current["secondary_argument"] = a0
                if current["mutations"] & 16:
                    update_buttons(uc, 0x22222222)
                    value = uc.mem_read(physical(SECOND_PORT), 1)[0]
                    uc.mem_write(physical(SECOND_PORT), bytes((value ^ 1,)))
                    current["live_mode"] ^= 0xFF
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            (buttons, mode, sample, gate, reference, second_port, selector, timer, mutations, replacement,
             prepare, status_1, status_2, latched_1, latched_2, mirror_extra) = entry
            current.update(sample=sample, gate=gate, reference=reference, second_port=second_port,
                           mutations=mutations, replacement=replacement, live_mode=mode & 0xFF,
                           calls=[], secondary_argument=EMPTY_VALUE, stop=None)
            stack = STACK - 0x120
            cpu.mem_write(physical(stack), bytes(0x120))
            cpu.mem_write(physical(EVENT), bytes(28))
            write_word(cpu, EVENT + 0x0C, buttons)
            cpu.mem_write(physical(EVENT + 0x10), bytes((status_1 & 0xFF, status_2 & 0xFF)))
            cpu.mem_write(physical(SECOND_PORT), bytes((second_port & 0xFF,)))
            cpu.mem_write(physical(PORT_SELECTION), struct.pack("<H", selector & 0xFFFF))
            cpu.mem_write(physical(TIMEOUT_TICKS), bytes((timer & 0xFF,)))
            cpu.mem_write(physical(GP + 0x1A7), bytes((latched_1 & 0xFF,)))
            cpu.mem_write(physical(GP + 0x1A8), bytes((latched_2 & 0xFF,)))
            cpu.mem_write(physical(GP + 0x1A3), bytes((mirror_extra & 0xFF,)))
            cpu.reg_write(UC_MIPS_REG_SP, STACK if prepare else stack)
            cpu.reg_write(UC_MIPS_REG_A0, EVENT)
            cpu.reg_write(UC_MIPS_REG_S0, EVENT + 0x0C)
            cpu.reg_write(UC_MIPS_REG_S1, EMPTY_VALUE)
            cpu.reg_write(UC_MIPS_REG_S2, EMPTY_VALUE)
            cpu.reg_write(UC_MIPS_REG_S3, EMPTY_VALUE)
            cpu.emu_start(0x80023E94 if prepare else 0x80024068, 0, count=5000)
            self.assertIn(current["stop"], (0x8002413C, 0x80024BD4))
            timeout = current["stop"] == 0x80024BD4
            calls = current["calls"] + [EMPTY_VALUE] * (8 - len(current["calls"]))
            original = (
                int(timeout), read_word(cpu, stack + 0x28), cpu.reg_read(UC_MIPS_REG_S2) & 0xFF,
                EMPTY_VALUE if timeout else cpu.reg_read(UC_MIPS_REG_S3) & 0xFFFFFFFF,
                read_word(cpu, EVENT + 0x0C), cpu.mem_read(physical(SECOND_PORT), 1)[0],
                struct.unpack("<H", cpu.mem_read(physical(PORT_SELECTION), 2))[0],
                cpu.mem_read(physical(TIMEOUT_TICKS), 1)[0], current["live_mode"],
                current["secondary_argument"], cpu.mem_read(physical(GP + 0x1A7), 1)[0],
                cpu.mem_read(physical(GP + 0x1A8), 1)[0], len(current["calls"]), *calls,
            )
            with self.subTest(index=index, input=entry):
                self.assertEqual(native, original)
