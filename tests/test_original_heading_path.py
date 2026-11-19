import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import (
    HAS_UNICORN,
    GP,
    ORIGINAL,
    RETURN,
    STACK,
    physical,
    read_word,
    write_word,
)

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0,
        UC_MIPS_REG_A1,
        UC_MIPS_REG_A2,
        UC_MIPS_REG_A3,
        UC_MIPS_REG_PC,
        UC_MIPS_REG_RA,
        UC_MIPS_REG_S0,
        UC_MIPS_REG_S1,
        UC_MIPS_REG_S2,
        UC_MIPS_REG_S5,
        UC_MIPS_REG_SP,
        UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<42I")
OUTPUT = struct.Struct("<86I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHeadingPathTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_entry_common_and_effects_against_original(self):
        self.compare_heading_path(full=False)

    def test_complete_modeled_callback_against_original(self):
        self.compare_heading_path(full=True)

    def test_exact_callback_against_original(self):
        self.compare_heading_path(full=True, unified=True)

    def test_exact_callback_with_sequence_wrappers_against_original(self):
        self.compare_heading_path(full=True, unified=True, real_sequences=True)

    def compare_heading_path(self, full, unified=False, real_sequences=False):
        fields = (
            "event_id", "counter", "previous_counter", "environment", "context_gate", "stop",
            "effect", "previous_effect", "latched", "started", "duration", "mode", "runtime_state",
            "gate", "now", "handles", "position_y", "actor_state", "actor_flags", "actor_2c",
            "shared_a", "shared_b", "status", "event_byte", "deceleration_state", "mutations",
            "event_flags", "context_lock", "context_gate_54", "level_flags",
            "resource_query", "resource_result", "sequence_result", "saved5", "saved15",
            "sequence_started", "stop_tick", "budget15",
            "sequence_scan_enabled", "sequence_limit_count", "sequence_key", "sequence_limit",
        )
        defaults = dict.fromkeys(fields, 0)
        defaults.update(
            event_id=8, counter=1, context_gate=1, effect=2, previous_effect=2,
            duration=100, now=100, handles=3, shared_b=0xFFFFFFFF,
            resource_result=1, saved5=0xFFFF, saved15=0xFFFF, stop_tick=5, budget15=20,
            sequence_scan_enabled=1, sequence_limit_count=1, sequence_key=5, sequence_limit=2,
        )

        def case(**changes):
            values = defaults.copy()
            values.update(changes)
            return tuple(values[field] for field in fields)

        cases = []
        for effect in range(5):
            for previous in range(5):
                for event_id in (8, 46, 50):
                    for changed in (0, 1):
                        for stopped in (0, 1):
                            for handles in (0, 3):
                                cases.append(case(
                                    effect=effect, previous_effect=previous, event_id=event_id,
                                    previous_counter=0 if changed else 1, stop=stopped, handles=handles,
                                    started=(effect + previous + event_id) & 1,
                                    mode=(effect + previous) & 1,
                                ))
        for bit in range(12):
            cases.extend((
                case(effect=3, previous_effect=2, mutations=1 << bit),
                case(effect=4, previous_effect=4, stop=1, mutations=1 << bit),
            ))
        generator = random.Random(0x3601839AB4)
        for _ in range(700):
            cases.append(case(
                event_id=generator.choice((8, 18, 19, 27, 46, 50)),
                counter=generator.choice((0, 1, 0xFFFFFFFF)),
                previous_counter=generator.choice((0, 1, 0xFFFFFFFF)),
                environment=generator.choice((0, 18, 19, 20, 0xFFFF)),
                context_gate=generator.choice((0, 1, 2)),
                stop=generator.choice((0, 0, 1)),
                effect=generator.randrange(5),
                previous_effect=generator.randrange(5),
                latched=generator.choice((0, 1, 2)),
                started=generator.choice((0, 1, 7, 0xFFFFFFF0)),
                duration=generator.choice((0, 1, 100, 0xFFFFFFFF)),
                mode=generator.choice((0, 1, 2)),
                runtime_state=generator.choice((0, 2, 5)),
                gate=generator.choice((0, 1, 0x80000000, 0xFFFFFFFF)),
                now=generator.choice((0, 1, 7, 100, 0xFFFFFFFF)),
                handles=generator.randrange(4),
                position_y=generator.choice((0, 1, 0x7FFFFFFF, 0xFFFFFFFF)),
                actor_state=generator.choice((0, 4, 0x40, 0x100, 0x1000, 0x80000)),
                actor_flags=generator.choice((0, 0x1000)),
                actor_2c=generator.choice((0, 1)),
                shared_a=generator.choice((0, 1, 0x80000000, 0xFFFFFFFF)),
                shared_b=generator.choice((0, 0xFFFFFFFF)),
                status=generator.choice((0, 1, 2)),
                event_byte=generator.choice((0, 1)),
                deceleration_state=generator.choice((0, 1)),
                mutations=generator.getrandbits(12),
            ))
        if full:
            for mode in (0, 1):
                for effect in (0, 2, 3, 4):
                    for sequence_result in (0, 1):
                        for gate in (0, 0xFFFF, 0x10000, 0xFFFFFFFF):
                            cases.append(case(
                                event_id=49, mode=mode, effect=effect, previous_effect=effect,
                                sequence_result=sequence_result, gate=gate, budget15=0x1234FFFF,
                            ))
            for saved5 in (0, 7, 0x7FFF, 0xFFFF):
                for saved15 in (0, 9, 0x7FFF, 0xFFFF):
                    for sequence_started in (0, 1):
                        cases.append(case(
                            event_id=50, saved5=saved5, saved15=saved15,
                            sequence_started=sequence_started, started=7, now=100, stop_tick=5,
                        ))
            for level_flags in (0, 0x10):
                for resource_query in (0, 1):
                    for resource_result in (0, 1):
                        cases.append(case(
                            event_id=42, level_flags=level_flags,
                            resource_query=resource_query, resource_result=resource_result,
                        ))
            for event_id in (37, 38, 41):
                for event_flags in (0, 0xFFFE, 0xFFFF, 0xFFFFFFFE):
                    for lock, gate_54, actor_flags in ((0, 0, 0), (1, 0, 0), (0, 1, 0), (0, 0, 0x100)):
                        cases.append(case(
                            event_id=event_id, event_flags=event_flags, runtime_state=5,
                            context_lock=lock, context_gate_54=gate_54, actor_flags=actor_flags,
                        ))
            for event_id in range(56):
                for state, runtime, runtime_mode in (
                    (0, 0, 0), (0x100000, 0, 0), (0x01000000, 5, 1),
                    (0x80000, 0, 0), (0x40, 0, 1),
                ):
                    cases.append(case(
                        event_id=event_id, effect=0, previous_effect=0, actor_state=state,
                        actor_flags=0x1000 if state == 0x40 else 0,
                        runtime_state=runtime, mode=runtime_mode,
                    ))
            for _ in range(1000):
                cases.append(case(
                    event_id=generator.randrange(56),
                    counter=generator.randrange(2), previous_counter=generator.randrange(2),
                    stop=generator.randrange(2), effect=generator.randrange(5),
                    previous_effect=generator.randrange(5), latched=generator.randrange(2),
                    started=generator.choice((0, 1, 7)), duration=generator.choice((0, 100)),
                    mode=generator.choice((0, 1, 2)), runtime_state=generator.choice((0, 2, 5)),
                    gate=generator.choice((0, 1, 0x80000000, 0xFFFFFFFF)),
                    now=generator.choice((0, 7, 100)), handles=generator.randrange(4),
                    position_y=generator.getrandbits(32),
                    actor_state=generator.choice((0, 4, 0x40, 0x100, 0x80000, 0x100000, 0x01000000)),
                    actor_flags=generator.choice((0, 0x100, 0x1000)),
                    actor_2c=generator.randrange(2), shared_a=generator.choice((0, 0xFFFFFFFF)),
                    shared_b=generator.choice((0, 0xFFFFFFFF)), status=generator.randrange(2),
                    event_byte=generator.randrange(2), deceleration_state=generator.randrange(2),
                    mutations=generator.getrandbits(12),
                    event_flags=generator.choice((0, 0xFFFE, 0xFFFF, 0xFFFFFFFE)),
                    context_lock=generator.randrange(2), context_gate_54=generator.randrange(2),
                    level_flags=generator.choice((0, 0x10)), resource_query=generator.randrange(2),
                    resource_result=generator.randrange(2), sequence_result=generator.randrange(2),
                    saved5=generator.choice((0, 7, 0xFFFF)), saved15=generator.choice((0, 9, 0xFFFF)),
                    sequence_started=generator.randrange(2), stop_tick=generator.choice((0, 5, 0xFFFFFFFF)),
                    budget15=generator.choice((0, 20, 0xFFFF, 0xFFFFFFFF)),
                ))
        if real_sequences:
            selector_cases = (
                (0, 1, 5, 2),
                (1, 1, 5, 0),
                (1, 1, 5, 1),
                (1, 1, 5, 2),
                (1, 1, 5, 0x7FFF),
                (1, 1, 6, 2),
                (1, 0, 5, 2),
                (1, 0, 6, 2),
                (255, 0xFFFF, 5, 2),
                (255, 1, 5, 0xFFFF),
            )
            for enabled, count, key, limit in selector_cases:
                for event_id in (49, 50):
                    cases.append(case(
                        event_id=event_id, effect=0, previous_effect=0,
                        sequence_result=0, sequence_started=1,
                        sequence_scan_enabled=enabled, sequence_limit_count=count,
                        sequence_key=key, sequence_limit=limit,
                    ))

        sources = [
            "src/game/heading_entry_gates.c", "src/game/heading_common_gates.c",
            "src/game/func_80039AB4.c", "src/game/func_8002B620.c",
            "src/game/func_8002E7D8.c", "src/game/func_8002E824.c",
            "tests/test_heading_path.c",
        ]
        if full:
            sources.append("src/game/heading_state_dispatch.c")
        flags = ["-DBUBSY3D_TEST_FULL_HEADING"] if full else []
        if unified:
            sources.append("src/game/func_80036018.c")
            flags.append("-DBUBSY3D_TEST_UNIFIED_HEADING")
        if real_sequences:
            sources.extend((
                "src/game/func_8005E708.c", "src/game/func_8005E760.c",
                "src/game/func_8005D5A8.c",
            ))
            flags.append("-DBUBSY3D_TEST_REAL_SEQUENCE_WRAPPERS")
        native_outputs = run_native_probe(
            self, sources, cases, INPUT, OUTPUT,
            compiler_flags=tuple(flags),
        )

        event, actor = 0x801E5000, 0x801E1000
        components = (0x801E2000, 0x801E2100)
        handles = (0x801E2200, 0x801E2300, 0x801E2400)
        transforms = (0x801E3000, 0x801E3100, 0x801E3200, 0x801E3300, 0x801E3400)
        shared_a, shared_b, context = 0x801E4000, 0x801E4100, 0x80186450
        counter, previous_counter = 0x800BDE98, 0x800BE0EC
        environment, context_gate = 0x801D36F0, 0x800BDFC7
        stop, effect, previous_effect, latched = 0x8018647B, 0x80186463, 0x800BE11C, 0x800BE11D
        started, duration, mode = 0x800BE0D0, 0x800BE0CC, 0x800BE548
        runtime_state, gate, handle_globals = 0x80186461, 0x8009F58C, 0x800BE68C
        status, event_byte, deceleration_state = 0x80186478, 0x800BE020, 0x80186464
        deceleration_active, deceleration_stop, stride = 0x801864B8, 0x8018649C, 0x80186460
        level, level_global = 0x801F0000, 0x800BE698

        def put_byte(cpu, address, value):
            cpu.mem_write(physical(address), bytes((value & 0xFF,)))

        def get_byte(cpu, address):
            return cpu.mem_read(physical(address), 1)[0]

        def handle_code(handle):
            if handle == 0:
                return 0
            self.assertIn(handle, handles)
            return handles.index(handle) + 1

        sequence_coverage = {"start": 0, "stop": 0, "accepted": 0, "rejected": 0}
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            (event_id, current, previous, kind, context_value, stopped, active_value, old_effect,
             latch, timestamp, interval, runtime_mode, runtime, gate_value, current_time, handle_mask,
             y, state, flags, actor_2c, shared_value, shared_next, heading_status, case_event,
             deceleration_value, mutations, event_flags, context_lock, gate_54, level_flags,
             resource_query, resource_result, sequence_result, saved5, saved15,
             sequence_started, stop_tick, budget15, sequence_scan_enabled,
             sequence_limit_count, sequence_key, sequence_limit) = entry
            with self.subTest(index=index, input=entry):
                cpu = self.fresh_cpu()
                cpu.mem_write(physical(event), struct.pack("<HHIi", event_id, 0, event_flags, -3))
                cpu.mem_write(physical(environment), struct.pack("<H", kind & 0xFFFF))
                put_byte(cpu, actor + 1, 7)
                put_byte(cpu, actor + 0x2C, actor_2c)
                for address, value in (
                    (counter, current), (previous_counter, previous), (actor + 4, flags),
                    (actor + 0x10, state), (actor + 0x1C, components[0]), (actor + 0x28, shared_a),
                    (components[0] + 0x14, transforms[0]), (components[1] + 0x14, transforms[4]),
                    (shared_a + 0x7C, shared_value), (shared_b + 0x7C, shared_next),
                    (handle_globals, handles[0] if handle_mask & 1 else 0),
                    (handle_globals + 4, handles[1] if handle_mask & 2 else 0),
                    (started, timestamp), (duration, interval), (gate, gate_value),
                ):
                    write_word(cpu, address, value)
                for handle, transform in zip(handles, transforms[1:4]):
                    write_word(cpu, handle + 0x14, transform)
                if full:
                    for address, value in (
                        (0x8009F7BC, budget15), (0x800BE538, stop_tick), (level_global, level),
                        (actor + 8, 0x11223344), (shared_a + 4, 0x12345678),
                        (shared_a + 0x30, 7), (shared_a + 0x34, 0xFFFFFFFB),
                        (0x80186458, 7), (0x8018645C, 77),
                    ):
                        write_word(cpu, address, value)
                    for offset, value in (
                        (0x38, 1), (0x3C, 2), (0x40, 3), (0x48, 11), (0x4C, 12),
                        (0x50, 13), (0x78, 21), (0x80, 23), (0x98, 31), (0x9C, 32), (0xA0, 33),
                    ):
                        write_word(cpu, shared_a + offset, value)
                    for address, value in (
                        (0x800BE0F2, saved15), (0x800BE0F4, saved5),
                        (0x80186450, 0x1234), (0x80186452, 0x5678), (0x80186454, gate_54),
                    ):
                        cpu.mem_write(physical(address), struct.pack("<H", value & 0xFFFF))
                    for address, value in (
                        (0x800BE0F0, sequence_started), (0x80186479, context_lock),
                        (0x80186462, 8), (0x8018647A, 9), (level + 0x2A2A, level_flags),
                    ):
                        put_byte(cpu, address, value)
                if real_sequences:
                    put_byte(cpu, GP + 0x624, sequence_scan_enabled)
                    cpu.mem_write(physical(GP + 0x62C), struct.pack("<H", sequence_limit_count & 0xFFFF))
                    cpu.mem_write(
                        physical(0x801CCE14),
                        struct.pack("<HH", sequence_key & 0xFFFF, sequence_limit & 0xFFFF) + bytes(140),
                    )
                for address, value in (
                    (context_gate, context_value), (stop, stopped), (effect, active_value),
                    (previous_effect, old_effect), (latched, latch), (mode, runtime_mode),
                    (runtime_state, runtime), (status, heading_status), (event_byte, case_event),
                    (deceleration_state, deceleration_value), (deceleration_active, 0),
                    (deceleration_stop, 9), (stride, 7),
                ):
                    put_byte(cpu, address, value)

                calls, reached, entry_changed = [], [], []
                sequence_wrappers, sequence_selectors, sequence_backends = [], [], []
                values = {"reset": 0, "sequence": 0, "position_y": 0, "captured": None}

                def intercept(uc, address, size, data):
                    if real_sequences and address in (0x8005E708, 0x8005E760):
                        sequence_wrappers.append(address)
                        return
                    if real_sequences and address == 0x8005D5A8:
                        sequence_selectors.append(address)
                        return
                    if address == 0x8003611C:
                        entry_changed.append(uc.reg_read(UC_MIPS_REG_S1))
                        return
                    if address == 0x800363DC or address == 0x80036950:
                        if not reached:
                            reached.append(address == 0x800363DC)
                        if values["captured"] is None:
                            values["captured"] = (
                                uc.reg_read(UC_MIPS_REG_S0),
                                read_word(uc, STACK - 0x50 + 0x24),
                                uc.reg_read(UC_MIPS_REG_S2),
                                uc.reg_read(UC_MIPS_REG_S5),
                            )
                        if not full or address == 0x80036950:
                            uc.emu_stop()
                        return
                    a0 = uc.reg_read(UC_MIPS_REG_A0)
                    a1 = uc.reg_read(UC_MIPS_REG_A1)
                    a2 = uc.reg_read(UC_MIPS_REG_A2)
                    a3 = uc.reg_read(UC_MIPS_REG_A3)
                    if address == 0x8004733C:
                        self.assertEqual(a0, 1)
                        calls.append(0x10000001)
                        if mutations & (1 << 9):
                            put_byte(uc, stop, 1)
                    elif address == 0x80028764:
                        self.assertEqual(a0, 0xFFFFFFFD)
                        calls.append(0x0B000000)
                        uc.reg_write(UC_MIPS_REG_V0, actor)
                    elif address == 0x80026648:
                        calls.append(0x0A000000)
                        uc.reg_write(UC_MIPS_REG_V0, context)
                    elif address == 0x80052BDC:
                        calls.append(0x01000000 | handle_code(a0) << 8 | a1)
                        if values["reset"] == 0:
                            if mutations & 1:
                                write_word(uc, handle_globals + 4, handles[2])
                            if mutations & (1 << 1):
                                uc.mem_write(physical(event), struct.pack("<H", 46))
                            if mutations & (1 << 2):
                                uc.mem_write(physical(event), struct.pack("<H", 8))
                        if values["reset"] == 1 and mutations & (1 << 3):
                            put_byte(uc, effect, 4)
                            put_byte(uc, previous_effect, 3)
                        values["reset"] += 1
                    elif address == 0x8001C700:
                        calls.append(0x02000000 | a0 << 8 | a1)
                        if values["sequence"] == 0:
                            if mutations & (1 << 4):
                                put_byte(uc, mode, 1)
                                put_byte(uc, deceleration_active, 1)
                            if mutations & (1 << 5):
                                write_word(uc, actor + 0x1C, components[1])
                        values["sequence"] += 1
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x8001C6BC:
                        self.assertEqual(a2, 0xFFFFFFFF)
                        calls.append(0x09000000 | a0 << 8 | a1)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x800532CC:
                        self.assertIn(a0, components)
                        self.assertEqual((a1, a2, a3), (1, 0, 0))
                        extra = read_word(uc, uc.reg_read(UC_MIPS_REG_SP) + 0x10)
                        self.assertIn(extra, (0, 0x40000000))
                        calls.append(0x03000000 | (components.index(a0) + 1) | (0x100 if extra else 0))
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x8002CB2C:
                        self.assertEqual(a0, 5)
                        calls.append(0x04000005)
                    elif address == 0x8002CA88:
                        self.assertIn(a0, (5, 15))
                        self.assertLessEqual(a1, 0x7FFF)
                        calls.append(0x05000000 | a0 << 16 | a1)
                    elif address == 0x80054B20:
                        self.assertEqual((a0, a1), (transforms[0], 0))
                        calls.append(0x06000000)
                        uc.mem_write(physical(a2), struct.pack("<3I", 0x1234, y, 0xFFFFFFFD))
                        if mutations & (1 << 6):
                            write_word(uc, started, 7)
                        if mutations & (1 << 7):
                            for handle in handles:
                                write_word(uc, handle + 0x14, transforms[4])
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x80054C7C:
                        self.assertIn(a0, transforms)
                        self.assertEqual(a1, 0)
                        calls.append(0x07000000 | (transforms.index(a0) + 1))
                        values["position_y"] = read_word(uc, a2 + 4)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x8001AE88:
                        timer_output = a0 == started
                        calls.append(0x08000000 | int(timer_output))
                        write_word(uc, a0, current_time)
                        if not timer_output and mutations & (1 << 10):
                            write_word(uc, started, 7)
                    elif address == 0x800395E8:
                        self.assertEqual((a0, a1), (actor, transforms[0]))
                        calls.append(0x0C000000)
                    elif address == 0x80038844:
                        self.assertEqual(a0, actor)
                        calls.append(0x0D000000)
                    elif address == 0x8001B558:
                        stack = uc.reg_read(UC_MIPS_REG_SP)
                        if full and a0 in (22, 40):
                            self.assertIn((a0, a1), ((22, 4), (40, 5)))
                            expected = (0xFFFE, 0xFFFE) if a0 == 40 else (
                                read_word(uc, actor + 0xC), read_word(uc, actor + 0xC)
                            )
                            self.assertEqual((a2, a3), expected)
                            self.assertEqual(
                                [read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C)],
                                [0, 0, 0, 0],
                            )
                            calls.append(0x11000000 | a0 << 8 | a1)
                            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                            return
                        self.assertIn((a0, a1), ((4, 1), (5, 3), (5, 4)))
                        self.assertEqual((a2, a3), (0xFFFE, 0xFFFE))
                        self.assertEqual(
                            [read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C)],
                            [actor + 0xC, 0, 0, 0],
                        )
                        calls.append(0x0E000000 | a0 << 8 | a1)
                        if a0 == 4 and mutations & (1 << 8):
                            write_word(uc, actor + 0x28, shared_b)
                            uc.mem_write(physical(event), struct.pack("<H", 27))
                    elif address == 0x8003539C:
                        self.assertEqual(a0, actor)
                        self.assertIn(a1, (0, 1) if full else (1,))
                        calls.append(0x0F000000 | a1)
                        if a1 == 1 and mutations & (1 << 11):
                            write_word(uc, actor + 0x10, 0x1000)
                    elif full and address == 0x8001C7C4:
                        self.assertEqual((a0, a1), (5, 1))
                        calls.append(0x12000000)
                        uc.reg_write(UC_MIPS_REG_V0, sequence_result)
                    elif full and not real_sequences and address in (0x8005E708, 0x8005E760):
                        self.assertEqual((a0, a1), (5, 1))
                        calls.append(0x13000000 if address == 0x8005E708 else 0x14000000)
                    elif full and real_sequences and address in (0x8008DBF8, 0x8008DF28):
                        self.assertEqual((a0, a1), (5, 1))
                        sequence_backends.append(address)
                        calls.append(0x13000000 if address == 0x8008DBF8 else 0x14000000)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif full and address == 0x80055680:
                        self.assertEqual(a0, 0x800BE0F8)
                        calls.append(0x15000000)
                    elif full and address == 0x8003697C:
                        self.assertEqual((a0, read_word(uc, a1), a2), (actor, transforms[0], shared_a))
                        calls.append(0x16000000)
                    elif full and address == 0x80052AEC:
                        self.assertIn(a0, components)
                        self.assertIn(a1, (0x35, 0x9A))
                        calls.append(0x17000000 | (components.index(a0) + 1) << 16 | a1)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif full and address == 0x80052E18:
                        self.assertIn(a0, components)
                        calls.append(0x18000000 | components.index(a0) + 1)
                    elif full and address == 0x8001B9C8:
                        calls.append(0x19000000)
                    elif full and address == 0x8003AC60:
                        calls.append(0x1A000000)
                    elif full and address == 0x800283E0:
                        calls.append(0x1B000000)
                        uc.reg_write(UC_MIPS_REG_V0, resource_query)
                    elif full and address == 0x80029548:
                        expected_resource = (0x8009F90C, 0x11) if resource_query == 0 else (0x8009F950, 0x0F)
                        self.assertEqual((a0, a1, a2), (2, *expected_resource))
                        self.assertEqual(struct.unpack("<2h", uc.mem_read(physical(a3), 4)), (0x340, 0x100))
                        calls.append(0x1C000000)
                        uc.reg_write(UC_MIPS_REG_V0, resource_result)
                    elif full and address == 0x800556D0:
                        self.assertEqual((a0, a1, a2, a3), (0, 0x8001299C, 0x8001298C, 0x6EE))
                        calls.append(0x25000000)
                    elif full and address == 0x80039F34:
                        self.assertEqual((a0, a1, a2), (event, actor, transforms[0]))
                        calls.append(0x1D000000)
                    elif full and address == 0x8003737C:
                        self.assertEqual((a0, a1), (actor, transforms[0]))
                        calls.append(0x1E000000)
                    elif full and address == 0x80037D14:
                        self.assertEqual((a0, a1, a2), (actor, transforms[0], event_flags))
                        calls.append(0x1F000000)
                    elif full and address in (0x8004D26C, 0x8004D994):
                        self.assertIn(a0, (0, 1))
                        self.assertEqual((a1, a2), (actor, transforms[0]))
                        calls.append((0x20000000 if address == 0x8004D26C else 0x21000000) | a0)
                    elif full and address == 0x800347EC:
                        self.assertEqual((a0, a1, a2), (actor, transforms[0], 1))
                        calls.append(0x22000000)
                    elif full and address in (0x8004B35C, 0x8004BFF8):
                        self.assertEqual((a0, a1, a2, a3), (event, actor, transforms[0], context))
                        self.assertEqual(read_word(uc, uc.reg_read(UC_MIPS_REG_SP) + 0x10), shared_a)
                        calls.append(0x23000000 if address == 0x8004B35C else 0x24000000)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_A0, event)
                cpu.reg_write(UC_MIPS_REG_RA, RETURN)
                cpu.emu_start(0x80036018, RETURN, count=30000)
                self.assertEqual(len(reached), 1)
                self.assertLessEqual(len(calls), 32)
                if real_sequences:
                    self.assertLessEqual(len(sequence_wrappers), 1)
                    self.assertEqual(len(sequence_selectors), len(sequence_wrappers))
                    if sequence_wrappers:
                        wrapper = sequence_wrappers[0]
                        kind = "start" if wrapper == 0x8005E708 else "stop"
                        sequence_coverage[kind] += 1
                        key = struct.unpack("<h", struct.pack("<H", sequence_key & 0xFFFF))[0]
                        limit = struct.unpack("<h", struct.pack("<H", sequence_limit & 0xFFFF))[0]
                        accepted = bool(sequence_scan_enabled & 0xFF) and key == 5 and 1 < limit
                        sequence_coverage["accepted" if accepted else "rejected"] += 1
                        expected_backend = 0x8008DBF8 if kind == "start" else 0x8008DF28
                        self.assertEqual(sequence_backends, [expected_backend] if accepted else [])
                        self.assertEqual(get_byte(cpu, 0x800BE0F0), int(kind == "start"))
                common_called = 0x0B000000 in calls
                captured = values["captured"]
                component_value = read_word(cpu, actor + 0x1C)
                self.assertIn(component_value, components)
                switch_state = (
                    read_word(cpu, actor + 8), read_word(cpu, shared_a + 4),
                    read_word(cpu, shared_a + 0x30), read_word(cpu, shared_a + 0x34),
                    *(read_word(cpu, shared_a + offset) for offset in (
                        0x38, 0x3C, 0x40, 0x48, 0x4C, 0x50, 0x78, 0x7C, 0x80, 0x98, 0x9C, 0xA0,
                    )),
                    read_word(cpu, 0x800BE538),
                    struct.unpack("<H", cpu.mem_read(physical(0x800BE0F4), 2))[0],
                    struct.unpack("<H", cpu.mem_read(physical(0x800BE0F2), 2))[0],
                    get_byte(cpu, 0x800BE0F0), read_word(cpu, 0x80186458),
                    struct.unpack("<H", cpu.mem_read(physical(0x80186450), 2))[0],
                    struct.unpack("<H", cpu.mem_read(physical(0x80186452), 2))[0],
                    read_word(cpu, 0x8018645C), get_byte(cpu, 0x80186462), get_byte(cpu, 0x8018647A),
                ) if full else (0,) * 26
                original = (
                    int(reached[0]), entry_changed[0] if entry_changed else cpu.reg_read(UC_MIPS_REG_S1),
                    read_word(cpu, previous_counter),
                    struct.unpack("<H", cpu.mem_read(physical(event), 2))[0],
                    get_byte(cpu, effect), get_byte(cpu, previous_effect), get_byte(cpu, latched),
                    read_word(cpu, started), get_byte(cpu, mode), get_byte(cpu, stop),
                    handle_code(read_word(cpu, handle_globals + 4)), components.index(component_value) + 1,
                    read_word(cpu, actor + 0x10), read_word(cpu, actor + 4),
                    get_byte(cpu, actor + 1), get_byte(cpu, actor + 0x2C),
                    int(read_word(cpu, actor + 0x28) == shared_b), get_byte(cpu, status),
                    get_byte(cpu, event_byte), get_byte(cpu, deceleration_active),
                    get_byte(cpu, deceleration_stop), get_byte(cpu, stride), values["position_y"],
                    int(common_called and captured[0] == actor),
                    int(common_called and captured[1] == transforms[0]),
                    int(common_called and captured[2] == shared_a),
                    int(common_called and captured[3] == context),
                    len(calls), *calls, *([0] * (32 - len(calls))), *switch_state,
                )
                if unified:
                    self.assertEqual(native[2:23], original[2:23])
                    self.assertEqual(native[27:], original[27:])
                else:
                    self.assertEqual(native, original)
        if real_sequences:
            for kind, count in sequence_coverage.items():
                self.assertGreater(count, 0, f"no integrated sequence {kind} cases")
