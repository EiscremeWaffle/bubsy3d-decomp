import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, STACK, physical, read_word, write_word

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
        UC_MIPS_REG_S2,
        UC_MIPS_REG_S3,
        UC_MIPS_REG_S5,
        UC_MIPS_REG_S6,
        UC_MIPS_REG_SP,
        UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<23I")
OUTPUT = struct.Struct("<62I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHeadingStateDispatchTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_state_switch_against_original(self):
        fields = (
            "event_id", "event_flags", "actor_state", "actor_flags", "heading",
            "runtime_mode", "runtime_state", "active", "gate5", "budget15",
            "stop_tick", "started", "saved5", "saved15", "sequence_started", "sequence_active",
            "lock", "context_gate", "config_flags", "query_mode", "resource_result", "now", "mutations",
        )
        defaults = dict.fromkeys(fields, 0)
        defaults.update(
            event_id=3, event_flags=0xFFFE, heading=0x1234, gate5=1, budget15=20,
            stop_tick=5, started=10, saved5=0xFFFF, saved15=0xFFFF, now=100,
            resource_result=1,
        )

        def case(**changes):
            values = defaults.copy()
            values.update(changes)
            return tuple(values[field] for field in fields)

        cases = [case(event_id=index) for index in (*range(56), 0xFFFF)]
        for event_id, runtime_state, event_flags, actor_state, actor_flags, lock, gate, mode in itertools.product(
            (3, 37, 38), (0, 5), (0, 0xFFFE, 0xFFFFFFFF), (0, 0x100000),
            (0, 0x100), (0, 1), (0, 1), (0, 1),
        ):
            cases.append(case(
                event_id=event_id, runtime_state=runtime_state, event_flags=event_flags,
                actor_state=actor_state, actor_flags=actor_flags, lock=lock,
                context_gate=gate, runtime_mode=mode,
            ))
        for active, mode, sequence, mutations in itertools.product(
            (0, 1, 2, 3, 4), (0, 1), (0, 1), (0, 1, 2, 4, 63)
        ):
            cases.append(case(
                event_id=49, active=active, runtime_mode=mode, sequence_active=sequence,
                gate5=0xFFFF8000, budget15=0x1234FFFF, mutations=mutations,
            ))
        for started, saved5, saved15, sequence, mutations in itertools.product(
            (0, 1, 0xFFFFFFFF), (0, 20, 0xFFFF), (0, 15, 0x8000),
            (0, 1), (0, 1, 8, 16, 63),
        ):
            cases.append(case(
                event_id=50, started=started, saved5=saved5, saved15=saved15,
                sequence_started=sequence, mutations=mutations, now=0xFFFFFFFF,
            ))
        for flags, query, result in itertools.product((0, 0x10), (0, 1, 255), (0, 1, 2, 255)):
            cases.append(case(event_id=42, config_flags=flags, query_mode=query, resource_result=result))
        for event_id, actor_state, runtime_state, mode, mutations in itertools.product(
            (18, 19, 22, 23, 40, 43), (0, 1, 0x01000000), (0, 5), (0, 1), (0, 32)
        ):
            cases.append(case(
                event_id=event_id, actor_state=actor_state, runtime_state=runtime_state,
                runtime_mode=mode, mutations=mutations,
            ))
        generator = random.Random(0x36018363DC)
        for _ in range(1200):
            cases.append(case(
                event_id=generator.choice((*range(56), 0xFFFF)),
                event_flags=generator.choice((0, 1, 0xFFFE, 0xFFFFFFFF)),
                actor_state=generator.choice((0, 1, 0x100000, 0x01000000, 0xFFFFFFFF)),
                actor_flags=generator.choice((0, 0x100, 0xFFFFFFFF)),
                heading=generator.getrandbits(32), runtime_mode=generator.choice((0, 1, 255)),
                runtime_state=generator.choice((0, 5, 255)), active=generator.randrange(5),
                gate5=generator.getrandbits(32), budget15=generator.getrandbits(32),
                stop_tick=generator.getrandbits(32), started=generator.getrandbits(32),
                saved5=generator.getrandbits(16), saved15=generator.getrandbits(16),
                sequence_started=generator.randrange(2), sequence_active=generator.randrange(2),
                lock=generator.randrange(2), context_gate=generator.choice((0, 1, 0xFFFF)),
                config_flags=generator.choice((0, 0x10, 0xFF)), query_mode=generator.randrange(2),
                resource_result=generator.randrange(3), now=generator.getrandbits(32),
                mutations=generator.getrandbits(6),
            ))

        native_outputs = run_native_probe(self, [
            "src/game/heading_state_dispatch.c", "tests/test_heading_state_dispatch.c",
        ], cases, INPUT, OUTPUT)
        event, actor, shared = 0x801E5000, 0x801E1000, 0x801E4000
        components, transform, context = (0x801E2000, 0x801E2100), 0x801E3000, 0x801E6000
        level, level_global = 0x801F0000, 0x800BE698
        stop_tick, started, saved5, saved15, sequence_flag = (
            0x800BE538, 0x800BE0D0, 0x800BE0F4, 0x800BE0F2, 0x800BE0F0
        )
        gate5, budget15, mode, runtime_state, active = (
            0x8009F58C, 0x8009F7BC, 0x800BE548, 0x80186461, 0x80186463
        )
        context_0, context_2, context_gate, phase, context_c = (
            0x80186450, 0x80186452, 0x80186454, 0x80186458, 0x8018645C
        )
        stride, state_62, flag_7a, decel_state, status, decel_active, decel_stop, lock = (
            0x80186460, 0x80186462, 0x8018647A, 0x80186464, 0x80186478,
            0x801864B8, 0x8018649C, 0x80186479,
        )
        stack = STACK - 0x50

        def put_byte(cpu, address, value):
            cpu.mem_write(physical(address), bytes((value & 0xFF,)))

        def put_half(cpu, address, value):
            cpu.mem_write(physical(address), struct.pack("<H", value & 0xFFFF))

        def get_byte(cpu, address):
            return cpu.mem_read(physical(address), 1)[0]

        def get_half(cpu, address):
            return struct.unpack("<H", cpu.mem_read(physical(address), 2))[0]

        def component_code(value):
            self.assertIn(value, components)
            return components.index(value) + 1

        for case_index, (entry, native) in enumerate(zip(cases, native_outputs)):
            (event_id, event_flags, actor_state, actor_flags, heading, runtime_mode, runtime_value,
             active_value, gate_value, budget_value, old_stop, old_started, old_saved5, old_saved15,
             old_sequence, sequence_active, lock_value, context_gate_value, config_flags, query_mode,
             resource_result, now, mutations) = entry
            with self.subTest(index=case_index, input=entry):
                cpu = self.fresh_cpu()
                cpu.mem_write(physical(shared), bytes((0x55,)) * 0xA4)
                put_half(cpu, event, event_id)
                write_word(cpu, event + 4, event_flags)
                for address, value in (
                    (actor + 4, actor_flags), (actor + 8, 0xBEEFBEEF), (actor + 0xC, heading),
                    (actor + 0x10, actor_state), (actor + 0x1C, components[0]), (actor + 0x28, shared),
                    (gate5, gate_value), (budget15, budget_value), (stop_tick, old_stop),
                    (started, old_started), (phase, 0x1234), (context_c, 0x2345), (level_global, level),
                ):
                    write_word(cpu, address, value)
                for address, value in (
                    (actor + 1, 7), (mode, runtime_mode), (runtime_state, runtime_value), (active, active_value),
                    (sequence_flag, old_sequence), (stride, 3), (state_62, 4), (flag_7a, 5),
                    (decel_state, 6), (status, 7), (decel_active, 8), (decel_stop, 9),
                    (lock, lock_value), (level + 0x2A2A, config_flags),
                ):
                    put_byte(cpu, address, value)
                for address, value in (
                    (saved5, old_saved5), (saved15, old_saved15),
                    (context_0, 0x1111), (context_2, 0x2222), (context_gate, context_gate_value),
                ):
                    put_half(cpu, address, value)
                write_word(cpu, stack + 0x24, transform)
                write_word(cpu, stack + 0x4C, RETURN)
                cpu.reg_write(UC_MIPS_REG_SP, stack)
                cpu.reg_write(UC_MIPS_REG_S0, actor)
                cpu.reg_write(UC_MIPS_REG_S2, shared)
                cpu.reg_write(UC_MIPS_REG_S3, event)
                cpu.reg_write(UC_MIPS_REG_S5, context)
                cpu.reg_write(UC_MIPS_REG_S6, 1)
                calls, arguments = [], [0, 0, 0, 0]

                def intercept(uc, address, size, user_data):
                    a0 = uc.reg_read(UC_MIPS_REG_A0)
                    a1 = uc.reg_read(UC_MIPS_REG_A1)
                    a2 = uc.reg_read(UC_MIPS_REG_A2)
                    a3 = uc.reg_read(UC_MIPS_REG_A3)
                    if address == 0x8001AE88:
                        calls.append(1)
                        write_word(uc, a0, now)
                        if mutations & 1:
                            put_byte(uc, active, 3)
                            write_word(uc, gate5, 0x3456)
                            write_word(uc, budget15, 0x5678)
                            put_byte(uc, mode, 1)
                            write_word(uc, started, 7)
                            write_word(uc, stop_tick, 9)
                    elif address == 0x8001C7C4:
                        self.assertEqual((a0, a1), (5, 1))
                        calls.append(2)
                        if mutations & 2:
                            put_byte(uc, sequence_flag, 7)
                        uc.reg_write(UC_MIPS_REG_V0, sequence_active)
                    elif address == 0x8005E708:
                        self.assertEqual((a0, a1), (5, 1))
                        calls.append(3)
                        if mutations & 4:
                            put_byte(uc, sequence_flag, 8)
                    elif address == 0x8005E760:
                        self.assertEqual((a0, a1), (5, 1))
                        calls.append(4)
                        if mutations & 8:
                            put_half(uc, saved5, 11)
                            put_half(uc, saved15, 12)
                    elif address == 0x8002CA88:
                        self.assertIn(a0, (5, 15))
                        calls.append(0x05000000 | a0 << 16 | (a1 & 0xFFFF))
                        if a0 == 5 and mutations & 16:
                            put_half(uc, saved15, 13)
                    elif address == 0x80055680:
                        self.assertEqual(a0, 0x800BE0F8)
                        calls.append(6)
                    elif address == 0x8003697C:
                        self.assertEqual((a0, a2, read_word(uc, a1)), (actor, shared, transform))
                        calls.append(7)
                    elif address == 0x80052AEC:
                        calls.append(8)
                        arguments[0], arguments[1] = a1, component_code(a0)
                        if mutations & 32:
                            write_word(uc, actor + 0x1C, components[1])
                            put_byte(uc, mode, 7)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x80052E18:
                        calls.append(9)
                        arguments[2] = component_code(a0)
                    elif address == 0x8001B9C8:
                        calls.append(10)
                    elif address == 0x8001B558:
                        self.assertEqual([read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C)], [0] * 4)
                        calls.append(11)
                        arguments[:] = [a0, a1, a2, a3]
                    elif address == 0x8003AC60:
                        calls.append(12)
                    elif address == 0x800283E0:
                        calls.append(13)
                        uc.reg_write(UC_MIPS_REG_V0, query_mode)
                    elif address == 0x80029548:
                        self.assertEqual(a0, 2)
                        self.assertIn(a1, (0x8009F90C, 0x8009F950))
                        calls.append(14)
                        arguments[:] = [1 if a1 == 0x8009F90C else 2, a2, get_half(uc, a3), get_half(uc, a3 + 2)]
                        uc.reg_write(UC_MIPS_REG_V0, resource_result)
                    elif address == 0x800556D0:
                        self.assertEqual((a0, a1, a2, a3), (0, 0x8001299C, 0x8001298C, 0x6EE))
                        calls.append(15)
                        arguments[0], arguments[1] = a0, a3
                    elif address == 0x80039F34:
                        self.assertEqual((a0, a1, a2), (event, actor, transform))
                        calls.append(16)
                    elif address == 0x8003737C:
                        self.assertEqual((a0, a1), (actor, transform))
                        calls.append(17)
                    elif address == 0x8003539C:
                        self.assertEqual(a0, actor)
                        calls.append(18)
                        arguments[0] = a1
                    elif address == 0x80037D14:
                        self.assertEqual((a0, a1), (actor, transform))
                        calls.append(19)
                        arguments[0] = a2
                    elif address in (0x8004D26C, 0x8004D994):
                        self.assertEqual((a1, a2), (actor, transform))
                        calls.append(20 if address == 0x8004D26C else 21)
                        arguments[0] = a0
                    elif address == 0x800347EC:
                        self.assertEqual((a0, a1), (actor, transform))
                        calls.append(22)
                        arguments[0] = a2
                    elif address in (0x8004B35C, 0x8004BFF8):
                        self.assertEqual((a0, a1, a2, a3), (event, actor, transform, context))
                        self.assertEqual(read_word(uc, stack + 0x10), shared)
                        calls.append(23 if address == 0x8004B35C else 24)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.emu_start(0x800363DC, RETURN, count=30000)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
                self.assertLessEqual(len(calls), 16)
                shared_offsets = (4, 0x30, 0x34, 0x38, 0x3C, 0x40, 0x48, 0x4C, 0x50,
                                  0x78, 0x7C, 0x80, 0x98, 0x9C, 0xA0)
                original = (
                    read_word(cpu, actor + 0x10), read_word(cpu, actor + 4), read_word(cpu, actor + 8),
                    get_byte(cpu, actor + 1), component_code(read_word(cpu, actor + 0x1C)),
                    *(read_word(cpu, shared + offset) for offset in shared_offsets),
                    get_byte(cpu, mode), get_byte(cpu, runtime_state), get_byte(cpu, active),
                    read_word(cpu, gate5), read_word(cpu, budget15), read_word(cpu, stop_tick),
                    read_word(cpu, started), get_half(cpu, saved5), get_half(cpu, saved15),
                    get_byte(cpu, sequence_flag), get_half(cpu, context_0), get_half(cpu, context_2),
                    read_word(cpu, phase), read_word(cpu, context_c), get_byte(cpu, stride),
                    get_byte(cpu, state_62), get_byte(cpu, flag_7a), get_byte(cpu, decel_state),
                    get_byte(cpu, status), get_byte(cpu, decel_active), get_byte(cpu, decel_stop),
                    *arguments, len(calls), *calls, *([0] * (16 - len(calls))),
                )
                self.assertEqual(native, original)
