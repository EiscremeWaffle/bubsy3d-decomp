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
        UC_MIPS_REG_A0,
        UC_MIPS_REG_A1,
        UC_MIPS_REG_A2,
        UC_MIPS_REG_A3,
        UC_MIPS_REG_PC,
        UC_MIPS_REG_RA,
        UC_MIPS_REG_SP,
        UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<20I")
OUTPUT = struct.Struct("<70I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHeadingModeOneTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_mode_one_backend_against_original(self):
        fields = (
            "paused", "budget", "previous_phase", "now", "previous_tick", "interval",
            "handle_flags", "position_x", "position_y", "position_z", "counter", "camera_present",
            "direction", "oscillation", "sequence_active", "random_a", "random_b",
            "allocation_result", "mutations", "seed",
        )
        defaults = dict.fromkeys(fields, 0)
        defaults.update(
            budget=20, now=100, interval=20, position_x=10, position_y=0x500, position_z=30,
            camera_present=1, random_a=1, random_b=1, seed=0x44332211,
        )

        def case(**changes):
            values = defaults.copy()
            values.update(changes)
            return tuple(values[field] for field in fields)

        budgets = (0xFFFFFFFF, 0, 1, 20, 21, 999, 1000, 0x7FFFFFFF)
        heights = (0xFFFFFFFF, 0, 0x4FF, 0x500, 0x580, 0x581, 0x7FFFFFFF)
        cases = [
            case(budget=budget, position_y=height, handle_flags=flags, camera_present=camera, counter=counter)
            for budget, height, flags, camera, counter in itertools.product(
                budgets, heights, (0, 1), (0, 1), (0, 14)
            )
        ]
        for bit in range(13):
            cases.extend((
                case(budget=0, camera_present=0, allocation_result=1, mutations=1 << bit),
                case(budget=1000, handle_flags=1, position_y=0x580, mutations=1 << bit),
                case(budget=21, position_y=0x581, counter=14, mutations=1 << bit),
            ))
        generator = random.Random(0x395E8)
        for _ in range(1000):
            cases.append(case(
                paused=generator.choice((0, 0, 1, 255)),
                budget=generator.choice(budgets + (0x80000000,)),
                previous_phase=generator.choice((0, 4, 0xFFFFFFFF)),
                now=generator.choice((0, 1, 7, 100, 0xFFFFFFFF)),
                previous_tick=generator.choice((0, 1, 7, 0xFFFFFFF0)),
                interval=generator.choice((0, 1, 10, 100, 0xFFFFFFFF)),
                handle_flags=generator.choice((0, 1, 2, 3, 4, 255)),
                position_x=generator.choice((0, 1, 0x7FFFFFFF, 0xFFFFFFFF)),
                position_y=generator.choice(heights + (0x80000000,)),
                position_z=generator.choice((0, 1, 0x80000000, 0xFFFFFFFF)),
                counter=generator.choice((0, 13, 14, 15, 0xFFFFFFFF)),
                camera_present=generator.randrange(2),
                direction=generator.choice((0, 1, 255)),
                oscillation=generator.choice((0, 89, 90, 91, 0xFFFFFFA5, 0xFFFFFFA6, 0x80000000)),
                sequence_active=generator.choice((0, 1, 2, 255)),
                random_a=generator.choice((0, 1, 7, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)),
                random_b=generator.choice((0, 1, 0xFFFFFFFF)),
                allocation_result=generator.choice((0, 0, 1)),
                mutations=generator.getrandbits(13),
                seed=generator.getrandbits(32),
            ))

        stubbed_outputs = run_native_probe(self, [
            "src/game/func_800395E8.c", "tests/test_heading_mode_one.c",
        ], cases, INPUT, OUTPUT)
        integrated_outputs = run_native_probe(self, [
            "src/game/func_800395E8.c", "src/game/func_80028100.c",
            "src/game/func_8004EDEC.c", "src/game/func_8004EF10.c",
            "tests/test_heading_mode_one.c",
        ], cases, INPUT, OUTPUT, compiler_flags=("-DBUBSY3D_TEST_REAL_MODE_CAMERA",))
        timed_outputs = run_native_probe(self, [
            "src/game/func_800395E8.c", "src/game/func_80028100.c",
            "src/game/func_8004EDEC.c", "src/game/func_8004EF10.c",
            "src/game/func_8002CA88.c", "src/game/func_8002CB2C.c",
            "tests/test_heading_mode_one.c",
        ], cases, INPUT, OUTPUT, compiler_flags=(
            "-DBUBSY3D_TEST_REAL_MODE_CAMERA", "-DBUBSY3D_TEST_REAL_TIMED_RECORDS",
        ))
        actor = 0x801E1000
        handles = (0x801E2000, 0x801E2100)
        transforms = (0x801E3000, 0x801E3100, 0x801E3200)
        cameras = (0x801E6000, 0x801E6100)
        mode_handle, budget, phase = 0x800BE558, 0x8009F7BC, 0x80186458
        camera, interval, previous_tick, seed = 0x800BE10C, 0x800BE110, 0x800BE114, 0x800BE118
        direction, counter, oscillation = 0x800BE109, 0x800BE540, 0x800BE544
        camera_slots, channels, channel_inputs = 0x8018CA0C, 0x800BE5A4, 0x801D89C0
        timed_record = 0x8009F444 + 15 * 0x38

        def put_byte(cpu, address, value):
            cpu.mem_write(physical(address), bytes((value & 0xFF,)))

        def handle_code(handle):
            self.assertIn(handle, handles)
            return handles.index(handle) + 1

        def camera_code(value):
            if value == 0:
                return 0
            self.assertIn(value, cameras)
            return cameras.index(value) + 1

        comparisons = (
            [(entry, native, False, False) for entry, native in zip(cases, stubbed_outputs)]
            + [(entry, native, True, False) for entry, native in zip(cases, integrated_outputs)]
            + [(entry, native, True, True) for entry, native in zip(cases, timed_outputs)]
        )
        for index, (entry, native, integrated, timed) in enumerate(comparisons):
            (paused, budget_value, old_phase, current_time, old_tick, old_interval, handle_flags,
             x, y, z, counter_value, camera_present, direction_value, oscillation_value,
             sequence_value, random_a, random_b, allocation_result, mutations, seed_value) = entry
            with self.subTest(index=index, integrated=integrated, timed=timed, input=entry):
                cpu = self.fresh_cpu()
                write_word(cpu, actor + 0xC, 0x1234)
                write_word(cpu, mode_handle, handles[0])
                write_word(cpu, handles[0] + 0x14, transforms[0])
                write_word(cpu, handles[1] + 0x14, transforms[1])
                put_byte(cpu, handles[0] + 4, handle_flags)
                put_byte(cpu, handles[1] + 4, 1)
                for address, value in (
                    (budget, budget_value), (phase, old_phase), (previous_tick, old_tick),
                    (interval, old_interval), (counter, counter_value), (oscillation, oscillation_value),
                    (camera, cameras[0] if camera_present else 0), (seed, seed_value),
                ):
                    write_word(cpu, address, value)
                put_byte(cpu, direction, direction_value)
                if integrated:
                    cpu.mem_write(physical(camera_slots), bytes(48))
                    cpu.mem_write(physical(channels), bytes(3))
                    cpu.mem_write(physical(channel_inputs), struct.pack("<3I", 0x10, 0x20, 0x30))
                if timed:
                    write_word(cpu, timed_record + 4, 0x10101010)
                    write_word(cpu, timed_record + 0xC, 0x11111111 if mutations & (1 << 8) else 0)
                calls = []
                data = {
                    "random": 0, "actor_position": [0, 0, 0], "mode_position": [0, 0, 0],
                    "rotation": [0, 0, 0], "camera_position": [0, 0, 0], "camera_seed": 0,
                    "final": [0, 0, 0], "queue": [0, 0, 0, 0],
                }

                def vector(uc, address):
                    return list(struct.unpack("<3I", uc.mem_read(physical(address), 12)))

                def intercept(uc, address, size, user_data):
                    a0 = uc.reg_read(UC_MIPS_REG_A0)
                    a1 = uc.reg_read(UC_MIPS_REG_A1)
                    a2 = uc.reg_read(UC_MIPS_REG_A2)
                    a3 = uc.reg_read(UC_MIPS_REG_A3)
                    if address == 0x8001AE88:
                        calls.append(1)
                        write_word(uc, a0, current_time)
                        if mutations & 1:
                            write_word(uc, mode_handle, handles[1])
                    elif address == 0x8001BCE4:
                        calls.append(2)
                        uc.reg_write(UC_MIPS_REG_V0, paused)
                    elif address == 0x8002CB2C and not timed:
                        self.assertEqual(a0, 15)
                        calls.append(3)
                    elif address == 0x8002CA88 and not timed:
                        self.assertEqual((a0, a1), (15, 999))
                        calls.append(4)
                    elif address == 0x8002C708 and timed:
                        self.assertEqual(a0, 15)
                        calls.append(29)
                        if mutations & (1 << 9):
                            write_word(uc, budget, 0xFFFFFFFE)
                            write_word(uc, timed_record + 4, 0x22222222)
                    elif address == 0x8002C604 and timed:
                        self.assertEqual(a0, 15)
                        calls.append(30)
                        if mutations & (1 << 10):
                            write_word(uc, timed_record + 4, 0x33333333)
                            write_word(uc, budget, 0x12345678)
                            write_word(uc, timed_record + 0xC, 0x33333333)
                    elif address == 0x8004FF50 and timed:
                        self.assertEqual((a0, a1, a2), (
                            read_word(uc, timed_record + 4), 0x3C, timed_record + 0xC,
                        ))
                        calls.append(31)
                        if mutations & (1 << 11):
                            write_word(uc, a2, 0x44444444)
                        uc.reg_write(UC_MIPS_REG_V0, 3)
                    elif address == 0x8004FFEC and timed:
                        self.assertEqual(a0, 0x11111111)
                        calls.append(32)
                        if mutations & (1 << 12):
                            write_word(uc, budget, 0x55555555)
                            write_word(uc, timed_record + 0xC, 0x66666666)
                        uc.reg_write(UC_MIPS_REG_V0, 1)
                    elif address == 0x8001C7C4:
                        self.assertEqual((a0, a1), (4, 12))
                        calls.append(5)
                        uc.reg_write(UC_MIPS_REG_V0, sequence_value)
                    elif address == 0x8001C700:
                        self.assertEqual((a0, a1), (4, 12))
                        calls.append(6)
                    elif address == 0x8001C6BC:
                        self.assertEqual((a0, a1, a2), (4, 12, 0xFFFFFFFF))
                        calls.append(7)
                    elif address == 0x8001B558:
                        self.assertEqual(a1, 4)
                        stack = uc.reg_read(UC_MIPS_REG_SP)
                        arguments = [read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C)]
                        if a0 == 26:
                            self.assertEqual((a2, a3), (0xFFFE, 0xFFFE))
                            self.assertEqual(arguments[3], arguments[1] + 8)
                            calls.append(11)
                            data["queue"] = [read_word(uc, pointer) for pointer in arguments]
                            if mutations & (1 << 2):
                                write_word(uc, arguments[1] + 4, 0x600)
                                write_word(uc, arguments[1] + 8, 0xFFFFFF85)
                        else:
                            self.assertIn(a0, (27, 46))
                            self.assertEqual((a2, a3), (0x1234, 0x1234))
                            self.assertEqual(arguments, [0, 0, 0, 0])
                            calls.append(8 if a0 == 27 else 18)
                    elif address == 0x80054B20:
                        self.assertEqual((a0, a1), (transforms[2], 0))
                        calls.append(9)
                        uc.mem_write(physical(a2), struct.pack("<3I", x, y, z))
                        if mutations & (1 << 1):
                            write_word(uc, previous_tick, 7)
                            write_word(uc, interval, 10)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x80082ECC:
                        self.assertLess(data["random"], 2)
                        calls.append(10)
                        uc.reg_write(UC_MIPS_REG_V0, (random_a, random_b)[data["random"]])
                        data["random"] += 1
                    elif address == 0x80054C7C:
                        self.assertEqual(a1, 0)
                        if a0 == transforms[0]:
                            calls.append(12)
                            data["mode_position"] = vector(uc, a2)
                        else:
                            self.assertEqual(a0, transforms[2])
                            calls.append(13)
                            data["actor_position"] = vector(uc, a2)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x80052AEC:
                        self.assertIn(a1, (0, 3))
                        calls.append((140 if a1 == 0 else 150) + handle_code(a0))
                        if mutations & (1 << 3):
                            write_word(uc, mode_handle, handles[1])
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x80052E18:
                        calls.append(160 + handle_code(a0))
                    elif address == 0x80052BDC:
                        self.assertEqual(a1, 0)
                        calls.append(170 + handle_code(a0))
                    elif address == 0x80022650:
                        self.assertEqual((a0, a1), (actor, 20))
                        calls.append(19)
                    elif address == 0x80054624:
                        self.assertEqual(a0, camera)
                        calls.append(20)
                        write_word(uc, a0, cameras[0])
                        if mutations & (1 << 4):
                            write_word(uc, seed, 0xEEEEEEEE)
                        uc.reg_write(UC_MIPS_REG_V0, allocation_result)
                    elif address == 0x800556D0:
                        self.assertEqual((a0, a1, a2, a3), (0, 0x80012974, 0x8001298C, 0xDDC))
                        calls.append(21)
                        if mutations & (1 << 7):
                            write_word(uc, camera, cameras[1])
                    elif address in (0x800547C8, 0x800548A0):
                        calls.append((220 if address == 0x800547C8 else 230) + camera_code(a0))
                        data["rotation"] = vector(uc, a1)
                        if mutations & (1 << 5):
                            write_word(uc, camera, cameras[1])
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x80054AF0:
                        calls.append(240 + camera_code(a0))
                        uc.mem_write(physical(a1), struct.pack("<3I", 11, 22, 33))
                        if mutations & (1 << 6):
                            write_word(uc, camera, cameras[1])
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x8004EC20:
                        self.assertEqual(a0, 2)
                        calls.append(25)
                        data["camera_seed"] = int.from_bytes(uc.mem_read(physical(a1), 3), "little")
                        data["camera_position"] = vector(uc, a2)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x8004EDEC and not integrated:
                        self.assertEqual(a0, 2)
                        calls.append(26)
                        data["camera_position"] = vector(uc, a1)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x80072798 and integrated:
                        self.assertEqual((a0, a1), (2, camera_slots + 32))
                        calls.append(26)
                        data["camera_position"] = vector(uc, a1)
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    elif address == 0x80028100 and not integrated:
                        calls.append(27)
                        for pointer, value in zip((a0, a1, a2), (0x10, 0x20, 0x30)):
                            write_word(uc, pointer, value)
                    elif (address == 0x8004EF10 and not integrated) or (
                        address == 0x80072DF0 and integrated
                    ):
                        calls.append(28)
                        data["final"] = [a0, a1, a2]
                        uc.reg_write(UC_MIPS_REG_V0, 0)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_A0, actor)
                cpu.reg_write(UC_MIPS_REG_A1, transforms[2])
                self.call(cpu, 0x800395E8)
                self.assertLessEqual(len(calls), 32)
                original = (
                    read_word(cpu, phase), read_word(cpu, previous_tick), read_word(cpu, interval),
                    read_word(cpu, counter), cpu.mem_read(physical(direction), 1)[0],
                    read_word(cpu, oscillation), cpu.mem_read(physical(actor), 1)[0],
                    handle_code(read_word(cpu, mode_handle)), camera_code(read_word(cpu, camera)),
                    *data["actor_position"], *data["mode_position"], *data["rotation"],
                    *data["camera_position"], data["camera_seed"], *data["final"], *data["queue"],
                    *(vector(cpu, camera_slots + 32) if integrated else [0, 0, 0]),
                    *(cpu.mem_read(physical(channels), 3) if integrated else [0, 0, 0]),
                    read_word(cpu, budget), read_word(cpu, timed_record + 0xC) if timed else 0,
                    len(calls), *calls, *([0] * (32 - len(calls))),
                )
                self.assertEqual(native, original)
