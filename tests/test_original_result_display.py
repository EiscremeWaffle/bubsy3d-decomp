import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA,
    )


INPUT = struct.Struct("<11I")
OUTPUT = struct.Struct("<25I")
PHASE = 0x800BDFD1
SCALE = 0x800BE520
TAG = 0x800BE510
EVENT = 0x800BE000
ACTOR_CONTEXT = 0x801D5F70
PREVIOUS_SELECTION = 0x800BDFDC


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalResultDisplayTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_result_display_against_original(self):
        scales = (0, 1, 0x3FF, 0x400, 0xFFF, 0x1000, 0x27FF, 0x2800, 0x2801,
                  0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        cases = [
            (operation, phase, scale, tag, 0x1234, 0x87654321, mutation,
             0x123, 0x22, 0xFE, 0x5678)
            for operation, phase, scale, tag, mutation in itertools.product(
                (0, 1), (0, 1, 2, 34, 35, 255), scales, (0, 127, 128, 255),
                (0, 1, 2, 4, 8, 16, 32, 63),
            )
        ]
        generator = random.Random(0x2DE342DF24)
        for _ in range(1000):
            cases.append((
                generator.randrange(2), generator.randrange(256), generator.getrandbits(32),
                generator.randrange(256), generator.getrandbits(32), generator.getrandbits(32),
                generator.randrange(64), generator.getrandbits(32), generator.randrange(256),
                generator.randrange(256), generator.getrandbits(32),
            ))
        native_outputs = run_native_probe(self, [
            "src/game/func_8002DE34.c", "src/game/func_8002DF24.c", "tests/test_result_display.c",
        ], cases, INPUT, OUTPUT)
        for case_index, (entry, native) in enumerate(zip(cases, native_outputs)):
            operation, phase, scale, tag, event, text, mutations, new_scale, new_phase, new_tag, new_event = entry
            with self.subTest(index=case_index, input=entry):
                cpu = self.fresh_cpu()
                cpu.mem_write(physical(PHASE), bytes((phase & 0xFF,)))
                write_word(cpu, SCALE, scale)
                cpu.mem_write(physical(TAG), bytes((tag & 0xFF,)))
                write_word(cpu, EVENT, event)
                calls, arguments = [], []

                def set_byte(uc, address, value):
                    uc.mem_write(physical(address), bytes((value & 0xFF,)))

                def intercept(uc, address, size, user_data):
                    a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
                    a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
                    a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
                    a3 = uc.reg_read(UC_MIPS_REG_A3) & 0xFFFFFFFF
                    if address == 0x800486D0:
                        self.assertEqual(a3, TAG)
                        calls.append(1)
                        arguments.extend((a0, a1, a2))
                        if mutations & 1:
                            set_byte(uc, TAG, new_tag)
                            set_byte(uc, PHASE, new_phase)
                            write_word(uc, SCALE, new_scale)
                    elif address == 0x800481D4:
                        calls.append(2)
                        arguments.extend((a0, a1, a2))
                        if mutations & 2:
                            set_byte(uc, TAG, new_tag)
                            set_byte(uc, PHASE, new_phase)
                            write_word(uc, SCALE, new_scale)
                    elif address == 0x800483C0:
                        calls.append(3)
                        arguments.extend((a0, 0, 0))
                    elif address == 0x8004FF50:
                        calls.append(4)
                        arguments.extend((int(a0 == 0x8002DE34), a1, int(a2 == EVENT)))
                        if mutations & 32:
                            write_word(uc, EVENT, new_event)
                    elif address == 0x80048504:
                        calls.append(5)
                        arguments.extend((a0, 0, 0))
                        if mutations & 4:
                            set_byte(uc, TAG, new_tag ^ 0x55)
                            write_word(uc, EVENT, new_event)
                    elif address == 0x80048C48:
                        calls.append(6)
                        arguments.extend((a0, 0, 0))
                        if mutations & 8:
                            write_word(uc, EVENT, new_event)
                    elif address == 0x8004FFEC:
                        calls.append(7)
                        arguments.extend((a0, 0, 0))
                        if mutations & 16:
                            write_word(uc, EVENT, 0xABCDEF01)
                            set_byte(uc, PHASE, new_phase)
                            write_word(uc, SCALE, new_scale)
                            set_byte(uc, TAG, new_tag)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_A0, text)
                self.call(cpu, 0x8002DF24 if operation == 0 else 0x8002DE34)
                final_tag = int.from_bytes(cpu.mem_read(physical(TAG), 1), "little", signed=True)
                original = (
                    cpu.mem_read(physical(PHASE), 1)[0], read_word(cpu, SCALE),
                    final_tag & 0xFFFFFFFF, read_word(cpu, EVENT), len(calls),
                    *calls, *([0] * (5 - len(calls))),
                    *arguments, *([0] * (15 - len(arguments))),
                )
                self.assertEqual(native, original)

    def test_controller_to_display_lifecycle(self):
        cpu = self.fresh_cpu()
        cpu.mem_write(physical(ACTOR_CONTEXT), bytes(0x40))
        cpu.mem_write(physical(ACTOR_CONTEXT + 0x0C), b"\x01")
        cpu.mem_write(physical(ACTOR_CONTEXT + 0x14), struct.pack("<h", 1))
        cpu.mem_write(physical(PREVIOUS_SELECTION), struct.pack("<h", 0))
        cpu.mem_write(physical(PHASE), b"\0")
        cpu.mem_write(physical(TAG), b"\0")
        write_word(cpu, SCALE, 0)
        write_word(cpu, EVENT, 0)
        event = 0x801E9000
        counts = {"create_text": 0, "scale": 0, "enable": 0, "remove_text": 0,
                  "release": 0, "create_event": 0, "remove_event": 0, "followup": 0}

        def intercept(uc, address, size, user_data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            a3 = uc.reg_read(UC_MIPS_REG_A3) & 0xFFFFFFFF
            if address == 0x800486D0:
                self.assertEqual((a0, a1, a2, a3), (0x80012730, 0xFFFFFFFF, 0xF0, TAG))
                self.assertEqual((uc.mem_read(physical(PHASE), 1)[0], read_word(uc, SCALE)), (1, 0))
                uc.mem_write(physical(TAG), b"\xFE")
                uc.mem_write(physical(ACTOR_CONTEXT + 0x14), struct.pack("<h", 0))
                counts["create_text"] += 1
            elif address == 0x800481D4:
                self.assertEqual(a0, 0xFFFFFFFE)
                self.assertLessEqual(a1, 0x1000)
                self.assertLessEqual(a2, 0x2C00)
                counts["scale"] += 1
            elif address == 0x800483C0:
                self.assertEqual(a0, 0xFFFFFFFE)
                counts["enable"] += 1
            elif address == 0x80048504:
                self.assertEqual(a0, 0xFFFFFFFE)
                counts["remove_text"] += 1
            elif address == 0x80048C48:
                self.assertEqual(a0, 0xFFFFFFFE)
                counts["release"] += 1
            elif address == 0x8004FF50:
                self.assertEqual((a0, a1, a2), (0x8002DE34, 4, EVENT))
                write_word(uc, EVENT, event)
                counts["create_event"] += 1
            elif address == 0x8004FFEC:
                self.assertEqual(a0, event)
                self.assertEqual((uc.mem_read(physical(PHASE), 1)[0], read_word(uc, SCALE)), (0, 0x1000))
                counts["remove_event"] += 1
            elif address == 0x8002C604:
                self.assertIn(a0, (6, 7))
                counts["followup"] += 1
            elif address == 0x8002BD0C:
                self.assertEqual(a0, 17)
                uc.reg_write(controller_route.UC_MIPS_REG_V0, 123)
                counts["followup"] += 1
            elif address == 0x8002CA88:
                self.assertEqual((a0, a1), (17, 123))
                counts["followup"] += 1
            elif address in (0x8002E1E0, 0x8002E348):
                self.assertEqual(a0, 0)
                counts["followup"] += 1
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        self.call(cpu, 0x8002C370)
        self.assertEqual(struct.unpack("<h", cpu.mem_read(physical(PREVIOUS_SELECTION), 2))[0], 0)
        self.assertEqual(read_word(cpu, EVENT), event)
        ticks = 0
        while read_word(cpu, EVENT) != 0 and ticks < 100:
            self.call(cpu, 0x8002DE34)
            ticks += 1
        self.assertEqual(ticks, 55)
        self.assertEqual((cpu.mem_read(physical(PHASE), 1)[0], read_word(cpu, SCALE)), (0, 0x1000))
        self.assertEqual(counts, {
            "create_text": 1, "scale": 57, "enable": 1, "remove_text": 1,
            "release": 1, "create_event": 1, "remove_event": 1, "followup": 6,
        })
