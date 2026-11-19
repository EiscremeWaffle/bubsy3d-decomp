import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import (
    HAS_UNICORN,
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
        UC_MIPS_REG_S3,
        UC_MIPS_REG_S5,
        UC_MIPS_REG_SP,
        UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<16I")
OUTPUT = struct.Struct("<33I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHeadingCommonTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_common_gates_against_native_model(self):
        fields = (
            "event_id", "changed", "state", "flags", "actor_01", "actor_2c",
            "shared_a", "shared_b", "runtime_mode", "deceleration_active",
            "runtime_state", "stop", "status", "deceleration_state", "event_byte", "mutations",
        )
        defaults = dict.fromkeys(fields, 0)
        defaults.update(event_id=8, changed=1, actor_01=7, shared_b=0xFFFFFFFF)

        def case(**changes):
            values = defaults.copy()
            values.update(changes)
            return tuple(values[field] for field in fields)

        cases = []
        for state in (0, 4, 0x40, 0x44, 0x100, 0x1000, 0x8000, 0x8100, 0x80000, 0x80100):
            for event_id in (8, 17, 18, 19, 20, 27, 46, 50):
                for flags in (0, 0x1000):
                    for changed in (0, 1):
                        for stopped in (0, 1):
                            for event_value in (0, 1):
                                cases.append(case(
                                    state=state, event_id=event_id, flags=flags, changed=changed,
                                    stop=stopped, event_byte=event_value,
                                ))
        for bit in range(10):
            if bit >= 8:
                cases.append(case(changed=0, state=0x40, flags=0x1000, mutations=1 << bit))
            else:
                cases.append(case(mutations=1 << bit))
        cases.extend((
            case(mutations=(1 << 2) | (1 << 7)),
            case(status=1, mutations=1 << 7),
            case(mutations=(1 << 0) | (1 << 1)),
            case(mutations=(1 << 3) | (1 << 4)),
            case(changed=256),
            case(actor_2c=1),
            case(runtime_state=5),
        ))
        generator = random.Random(0x36018)
        for _ in range(400):
            cases.append(case(
                event_id=generator.choice((8, 17, 18, 19, 20, 27, 46, 50)),
                changed=generator.choice((0, 1, 2, 255, 256)),
                state=generator.choice((0, 4, 0x40, 0x44, 0x100, 0x1000, 0x8100, 0x80000)),
                flags=generator.choice((0, 0x1000)),
                actor_01=generator.choice((0, 1, 7)),
                actor_2c=generator.choice((0, 1, 255)),
                shared_a=generator.choice((0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)),
                shared_b=generator.choice((0, 0x80000000, 0xFFFFFFFF)),
                runtime_mode=generator.choice((0, 1, 2)),
                deceleration_active=generator.choice((0, 1, 255)),
                runtime_state=generator.choice((0, 4, 5, 6)),
                stop=generator.choice((0, 1, 255)),
                status=generator.choice((0, 1, 2, 255)),
                deceleration_state=generator.choice((0, 1, 255)),
                event_byte=generator.choice((0, 1, 255)),
                mutations=generator.getrandbits(10),
            ))

        native_outputs = run_native_probe(self, [
            "src/game/heading_common_gates.c", "src/game/func_8002E7D8.c",
            "src/game/func_8002E824.c", "tests/test_heading_common_gates.c",
        ], cases, INPUT, OUTPUT)

        event, actor, component = 0x801E5000, 0x801E1000, 0x801E2000
        transform, shared_a, shared_b, context = 0x801E3000, 0x801E4000, 0x801E4100, 0x80186450
        runtime_mode, active, runtime_state = 0x800BE548, 0x801864B8, 0x80186461
        stop, status, deceleration_state = 0x8018647B, 0x80186478, 0x80186464
        deceleration_stop, stride, event_byte = 0x8018649C, 0x80186460, 0x800BE020

        def put_byte(cpu, address, value):
            cpu.mem_write(physical(address), bytes((value & 0xFF,)))

        def get_byte(cpu, address):
            return cpu.mem_read(physical(address), 1)[0]

        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            (event_id, changed, state, flags, actor_01, actor_2c, value_a, value_b, mode,
             deceleration, runtime, stopped, status_value, deceleration_value, case_event,
             mutations) = entry
            with self.subTest(index=index, input=entry):
                cpu = self.fresh_cpu()
                cpu.mem_write(physical(event), struct.pack("<HHIi", event_id, 0, 0, -3))
                put_byte(cpu, actor + 1, actor_01)
                put_byte(cpu, actor + 0x2C, actor_2c)
                for address, value in (
                    (actor + 4, flags), (actor + 0x10, state), (actor + 0x1C, component),
                    (actor + 0x28, shared_a), (component + 0x14, transform),
                    (shared_a + 0x7C, value_a), (shared_b + 0x7C, value_b),
                ):
                    write_word(cpu, address, value)
                for address, value in (
                    (runtime_mode, mode), (active, deceleration), (runtime_state, runtime),
                    (stop, stopped), (status, status_value), (deceleration_state, deceleration_value),
                    (deceleration_stop, 9), (stride, 7), (event_byte, case_event),
                ):
                    put_byte(cpu, address, value)

                calls, reached = [], []

                def intercept(uc, address, size, data):
                    if address in (0x800363DC, 0x80036950):
                        reached.append(address == 0x800363DC)
                        uc.emu_stop()
                        return
                    if address == 0x80028764:
                        self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 0xFFFFFFFD)
                        calls.append(1)
                        uc.reg_write(UC_MIPS_REG_V0, actor)
                    elif address == 0x80026648:
                        calls.append(2)
                        uc.reg_write(UC_MIPS_REG_V0, context)
                    elif address in (0x80039AB4, 0x800395E8):
                        self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), actor)
                        self.assertEqual(uc.reg_read(UC_MIPS_REG_A1), transform)
                        calls.append(3 if address == 0x80039AB4 else 4)
                        if address == 0x80039AB4 and mutations & 1:
                            put_byte(uc, runtime_mode, 1)
                            put_byte(uc, active, 1)
                        if address == 0x80039AB4 and mutations & 2:
                            write_word(uc, actor + 0x1C, 0)
                    elif address == 0x80038844:
                        self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), actor)
                        calls.append(5)
                    elif address == 0x8001B558:
                        first = uc.reg_read(UC_MIPS_REG_A0)
                        second = uc.reg_read(UC_MIPS_REG_A1)
                        self.assertEqual((uc.reg_read(UC_MIPS_REG_A2), uc.reg_read(UC_MIPS_REG_A3)),
                                         (0xFFFE, 0xFFFE))
                        stack = uc.reg_read(UC_MIPS_REG_SP)
                        self.assertEqual([read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C)],
                                         [actor + 0xC, 0, 0, 0])
                        if (first, second) == (4, 1):
                            calls.append(6)
                            if mutations & (1 << 2):
                                write_word(uc, actor + 0x28, shared_b)
                            if mutations & (1 << 3):
                                write_word(uc, actor + 0x10, 0x80000)
                                uc.mem_write(physical(event), struct.pack("<H", 27))
                            if mutations & (1 << 4):
                                write_word(uc, actor + 0x10, 0x80000)
                                uc.mem_write(physical(event), struct.pack("<H", 8))
                            if mutations & (1 << 5):
                                put_byte(uc, runtime_state, 5)
                                put_byte(uc, actor + 0x2C, 1)
                            if mutations & (1 << 6):
                                put_byte(uc, stop, 1)
                        else:
                            self.assertIn((first, second), ((5, 3), (5, 4)))
                            calls.append(7 if second == 3 else 8)
                            if mutations & (1 << 7):
                                put_byte(uc, status, 7)
                    elif address == 0x8001C700:
                        self.assertEqual((uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)), (4, 3))
                        calls.append(9)
                        if mutations & (1 << 8):
                            put_byte(uc, deceleration_state, 1)
                    elif address == 0x8003539C:
                        self.assertEqual((uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)), (actor, 1))
                        calls.append(10)
                        if mutations & (1 << 9):
                            write_word(uc, actor + 0x10, 0x1000)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_S1, changed)
                cpu.reg_write(UC_MIPS_REG_S3, event)
                cpu.reg_write(UC_MIPS_REG_SP, STACK - 0x50)
                cpu.reg_write(UC_MIPS_REG_RA, RETURN)
                cpu.emu_start(0x8003611C, RETURN, count=20000)
                self.assertEqual(len(reached), 1)
                self.assertLessEqual(len(calls), 12)
                original = (
                    int(reached[0]), int(cpu.reg_read(UC_MIPS_REG_S0) == actor),
                    int(read_word(cpu, STACK - 0x50 + 0x24) == transform),
                    int(cpu.reg_read(UC_MIPS_REG_S2) == shared_a),
                    int(cpu.reg_read(UC_MIPS_REG_S5) == context),
                    struct.unpack("<H", cpu.mem_read(physical(event), 2))[0],
                    read_word(cpu, actor + 0x10), read_word(cpu, actor + 4),
                    get_byte(cpu, actor + 1), get_byte(cpu, actor + 0x2C),
                    int(read_word(cpu, actor + 0x28) == shared_b),
                    get_byte(cpu, runtime_mode), get_byte(cpu, active),
                    get_byte(cpu, runtime_state), get_byte(cpu, stop), get_byte(cpu, status),
                    get_byte(cpu, deceleration_state), get_byte(cpu, deceleration_stop),
                    get_byte(cpu, stride), get_byte(cpu, event_byte), len(calls),
                    *calls, *([0] * (12 - len(calls))),
                )
                self.assertEqual(native, original)
