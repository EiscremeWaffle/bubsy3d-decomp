import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical, read_word, write_word

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0,
        UC_MIPS_REG_A1,
        UC_MIPS_REG_A2,
        UC_MIPS_REG_A3,
        UC_MIPS_REG_PC,
        UC_MIPS_REG_RA,
        UC_MIPS_REG_SP,
        UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<12I")
OUTPUT = struct.Struct("<37I")


class NativeHeadingEffectDomainTests(unittest.TestCase):
    def test_unsupported_effect_is_reported(self):
        run_native_probe(
            self, ["src/game/func_80039AB4.c", "tests/test_heading_effects.c"],
            [(5,) + (0,) * 11], INPUT, OUTPUT,
            expected_error=b"unsupported heading effect: 5",
        )


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHeadingEffectsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_heading_effects_against_original(self):
        fields = (
            "effect", "previous", "latched", "started", "duration", "mode", "runtime_state",
            "gate", "now", "handles", "position_y", "mutations",
        )
        defaults = dict.fromkeys(fields, 0)
        defaults.update(effect=1, previous=1, duration=100, now=100, handles=3)

        def case(**changes):
            values = defaults.copy()
            values.update(changes)
            return tuple(values[field] for field in fields)

        cases = []
        for effect in range(5):
            for previous in range(5):
                for latched in (0, 1, 2):
                    for started in (0, 1):
                        for mode in (0, 1):
                            for gate in (0, 1):
                                for handles in range(4):
                                    cases.append(case(
                                        effect=effect, previous=previous, latched=latched,
                                        started=started, mode=mode, gate=gate, handles=handles,
                                    ))
        for bit in range(14):
            cases.extend((
                case(effect=1, previous=1, started=1, mutations=1 << bit),
                case(effect=2, previous=2, started=0, mutations=1 << bit),
                case(effect=3, previous=2, started=1, mutations=1 << bit),
            ))
        for started in (1, 0xFFFFFFF0):
            for delta in (0, 99, 100, 101, 0xFFFFFFFF):
                for effect in (1, 4):
                    cases.append(case(
                        effect=effect, previous=effect, started=started, duration=100,
                        now=(started + delta) & 0xFFFFFFFF, position_y=0x7FFFFFFF,
                    ))
        generator = random.Random(0x39AB4)
        for _ in range(500):
            cases.append(case(
                effect=generator.randrange(5),
                previous=generator.randrange(5),
                latched=generator.choice((0, 1, 2, 255)),
                started=generator.choice((0, 1, 7, 0x7FFFFFFF, 0xFFFFFFF0, 0xFFFFFFFF)),
                duration=generator.choice((0, 1, 100, 1200, 0xFFFFFF, 0xFFFFFFFF)),
                mode=generator.choice((0, 1, 2, 255)),
                runtime_state=generator.choice((0, 2, 5, 255)),
                gate=generator.choice((0, 1, 0x80000000, 0xFFFFFFFF)),
                now=generator.choice((0, 1, 7, 100, 0x7FFFFFFF, 0xFFFFFFF0, 0xFFFFFFFF)),
                handles=generator.randrange(4),
                position_y=generator.choice((0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)),
                mutations=generator.getrandbits(14),
            ))

        stubbed_outputs = run_native_probe(self, [
            "src/game/func_80039AB4.c", "tests/test_heading_effects.c",
        ], cases, INPUT, OUTPUT)
        timed_outputs = run_native_probe(self, [
            "src/game/func_80039AB4.c", "src/game/func_8002CA88.c",
            "src/game/func_8002CB2C.c", "tests/test_heading_effects.c",
        ], cases, INPUT, OUTPUT, compiler_flags=("-DBUBSY3D_TEST_REAL_TIMED_RECORDS",))
        actor = 0x801E1000
        handles = (0x801E2000, 0x801E2100, 0x801E2200)
        transforms = (0x801E3000, 0x801E3100, 0x801E3200, 0x801E3400)
        components = (0x801E2400, 0x801E2500)
        input_transform = 0x801E3300
        active, previous, latched = 0x80186463, 0x800BE11C, 0x800BE11D
        started, duration, mode = 0x800BE0D0, 0x800BE0CC, 0x800BE548
        runtime_state, gate, handle_globals = 0x80186461, 0x8009F58C, 0x800BE68C
        timed_record = 0x8009F444 + 5 * 0x38

        def put_byte(cpu, address, value):
            cpu.mem_write(physical(address), bytes((value & 0xFF,)))

        def get_byte(cpu, address):
            return cpu.mem_read(physical(address), 1)[0]

        def handle_code(handle):
            if handle == 0:
                return 0
            self.assertIn(handle, handles)
            return handles.index(handle) + 1

        comparisons = (
            [(entry, native, False) for entry, native in zip(cases, stubbed_outputs)]
            + [(entry, native, True) for entry, native in zip(cases, timed_outputs)]
        )
        for index, (entry, native, timed) in enumerate(comparisons):
            (effect, old, latch, timestamp, interval, runtime_mode, runtime, gate_value,
             now, handle_mask, y, mutations) = entry
            with self.subTest(index=index, timed=timed, input=entry):
                cpu = self.fresh_cpu()
                write_word(cpu, actor + 0x1C, components[0])
                write_word(cpu, handle_globals, handles[0] if handle_mask & 1 else 0)
                write_word(cpu, handle_globals + 4, handles[1] if handle_mask & 2 else 0)
                for handle, transform in zip(handles, transforms):
                    write_word(cpu, handle + 0x14, transform)
                for address, value in ((started, timestamp), (duration, interval), (gate, gate_value)):
                    write_word(cpu, address, value)
                if timed:
                    write_word(cpu, timed_record + 4, 0x10101010)
                    write_word(cpu, timed_record + 0xC, 0x11111111 if mutations & (1 << 9) else 0)
                for address, value in (
                    (active, effect), (previous, old), (latched, latch),
                    (mode, runtime_mode), (runtime_state, runtime),
                ):
                    put_byte(cpu, address, value)

                calls, position = [], [0, 0, 0]
                counters = {"reset": 0, "sequence": 0}

                def intercept(uc, address, size, data):
                    a0 = uc.reg_read(UC_MIPS_REG_A0)
                    a1 = uc.reg_read(UC_MIPS_REG_A1)
                    a2 = uc.reg_read(UC_MIPS_REG_A2)
                    a3 = uc.reg_read(UC_MIPS_REG_A3)
                    if address == 0x80052BDC:
                        calls.append(0x01000000 | handle_code(a0) << 8 | a1)
                        if counters["reset"] == 0 and mutations & 1:
                            write_word(uc, handle_globals + 4, handles[2])
                        if counters["reset"] == 1:
                            if mutations & (1 << 1):
                                put_byte(uc, active, 4)
                            if mutations & (1 << 2):
                                put_byte(uc, previous, 3)
                        counters["reset"] += 1
                    elif address == 0x8001C700:
                        calls.append(0x02000000 | a0 << 8 | a1)
                        if counters["sequence"] == 0:
                            if mutations & (1 << 3):
                                put_byte(uc, previous, 3)
                            if mutations & (1 << 4):
                                put_byte(uc, mode, 1)
                            if mutations & (1 << 8):
                                write_word(uc, actor + 0x1C, components[1])
                        counters["sequence"] += 1
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x8001C6BC:
                        self.assertEqual(a2, 0xFFFFFFFF)
                        calls.append(0x09000000 | a0 << 8 | a1)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x800532CC:
                        self.assertIn(a0, components)
                        self.assertEqual((a1, a2, a3), (1, 0, 0))
                        flags = read_word(uc, uc.reg_read(UC_MIPS_REG_SP) + 0x10)
                        self.assertIn(flags, (0, 0x40000000))
                        calls.append(0x03000000 | components.index(a0) + 1 | (0x100 if flags else 0))
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x8002CB2C and not timed:
                        self.assertEqual(a0, 5)
                        calls.append(0x04000000 | a0)
                    elif address == 0x8002CA88 and not timed:
                        self.assertEqual((a0, a1), (5, 20))
                        calls.append(0x05000000 | a0 << 8 | a1)
                    elif address == 0x8002C708 and timed:
                        self.assertEqual(a0, 5)
                        calls.append(0x0A000005)
                        if mutations & (1 << 10):
                            write_word(uc, gate, 0xFFFFFFFE)
                            write_word(uc, timed_record + 4, 0x22222222)
                    elif address == 0x8002C604 and timed:
                        self.assertEqual(a0, 5)
                        calls.append(0x0B000005)
                        if mutations & (1 << 11):
                            write_word(uc, timed_record + 4, 0x33333333)
                            write_word(uc, gate, 0x12345678)
                            write_word(uc, timed_record + 0xC, 0x33333333)
                    elif address == 0x8004FF50 and timed:
                        self.assertEqual((a0, a1, a2), (
                            read_word(uc, timed_record + 4), 0x3C, timed_record + 0xC,
                        ))
                        calls.append(0x0C000005)
                        if mutations & (1 << 12):
                            write_word(uc, a2, 0x44444444)
                        uc.reg_write(UC_MIPS_REG_V0, 3)
                    elif address == 0x8004FFEC and timed:
                        self.assertEqual(a0, 0x11111111)
                        calls.append(0x0D000005)
                        if mutations & (1 << 13):
                            write_word(uc, gate, 0x55555555)
                            write_word(uc, timed_record + 0xC, 0x66666666)
                        uc.reg_write(UC_MIPS_REG_V0, 1)
                    elif address == 0x80054B20:
                        self.assertEqual((a0, a1), (input_transform, 0))
                        calls.append(0x06000000)
                        uc.mem_write(physical(a2), struct.pack("<3I", 0x1234, y, 0xFFFFFFFD))
                        if mutations & (1 << 5):
                            write_word(uc, started, 7)
                        if mutations & (1 << 6):
                            for handle in handles:
                                write_word(uc, handle + 0x14, transforms[3])
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x80054C7C:
                        self.assertIn(a0, transforms)
                        self.assertEqual(a1, 0)
                        calls.append(0x07000000 | transforms.index(a0) + 1)
                        position[:] = struct.unpack("<3I", uc.mem_read(physical(a2), 12))
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x8001AE88:
                        timer_output = a0 == started
                        calls.append(0x08000000 | int(timer_output))
                        write_word(uc, a0, now)
                        if not timer_output and mutations & (1 << 7):
                            write_word(uc, started, 7)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_A0, actor)
                cpu.reg_write(UC_MIPS_REG_A1, input_transform)
                self.call(cpu, 0x80039AB4)
                self.assertLessEqual(len(calls), 24)
                component_value = read_word(cpu, actor + 0x1C)
                self.assertIn(component_value, components)
                original = (
                    get_byte(cpu, active), get_byte(cpu, previous), get_byte(cpu, latched),
                    read_word(cpu, started), get_byte(cpu, mode),
                    handle_code(read_word(cpu, handle_globals + 4)),
                    components.index(component_value) + 1,
                    *position, read_word(cpu, gate),
                    read_word(cpu, timed_record + 0xC) if timed else 0,
                    len(calls), *calls, *([0] * (24 - len(calls))),
                )
                self.assertEqual(native, original)
