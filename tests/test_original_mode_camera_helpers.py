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
        UC_MIPS_REG_PC,
        UC_MIPS_REG_RA,
        UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<7I")
OUTPUT = struct.Struct("<27I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalModeCameraHelperTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_helpers_against_original(self):
        cases = []
        for first, second, third in itertools.product(range(6), repeat=3):
            aliases = first | second << 3 | third << 6
            cases.append((0, 0, 0x80000000, 0x12345678, 0xFFFFFFFF, aliases, 0))
        for index, source, mutation in itertools.product(range(5), range(4), range(2)):
            cases.append((1, index, 0x80000000, 0x12345678, 0xFFFFFFFF, source, mutation))
        boundary = (0, 1, 15, 16, 0xFFF, 0x1000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        for first, second, third in itertools.product(boundary, repeat=3):
            cases.append((2, 0, first, second, third, 0, (first ^ second ^ third) & 1))
        generator = random.Random(0x281004EF10)
        for _ in range(200):
            cases.append((
                2, 0, generator.randrange(0x1200), generator.randrange(0x1200),
                generator.randrange(0x1200), 0, generator.randrange(2),
            ))

        stubbed_outputs = run_native_probe(self, [
            "src/game/func_80028100.c", "src/game/func_8004EDEC.c",
            "src/game/func_8004EF10.c", "tests/test_mode_camera_helpers.c",
        ], cases, INPUT, OUTPUT)
        backend_cases = [
            (3, 0, first, second, third, 0, (first ^ second ^ third) & 1)
            for first, second, third in itertools.product(boundary, repeat=3)
        ]
        for _ in range(200):
            backend_cases.append((
                3, 0, generator.getrandbits(32), generator.getrandbits(32),
                generator.getrandbits(32), 0, generator.randrange(2),
            ))
        integrated_cases = cases + backend_cases
        integrated_outputs = run_native_probe(self, [
            "src/game/func_80028100.c", "src/game/func_8004EDEC.c",
            "src/game/func_8004EF10.c", "src/game/func_80072DF0.c",
            "tests/test_mode_camera_helpers.c",
        ], integrated_cases, INPUT, OUTPUT, compiler_flags=("-DBUBSY3D_TEST_REAL_CAMERA_CHANNEL",))
        globals_address, channels, cameras = 0x801D89C0, 0x800BE5A4, 0x8018CA0C
        outputs, values = 0x801E5000, 0x801E5100
        comparisons = (
            [(entry, native, False) for entry, native in zip(cases, stubbed_outputs)]
            + [(entry, native, True) for entry, native in zip(integrated_cases, integrated_outputs)]
        )
        for index, (entry, native, integrated) in enumerate(comparisons):
            operation, camera_index, first, second, third, aliases, mutation = entry
            with self.subTest(index=index, integrated=integrated, input=entry):
                cpu = self.fresh_cpu()
                for component, value in enumerate((first, second, third)):
                    write_word(cpu, globals_address + component * 4, value)
                    write_word(cpu, values + component * 4, value)
                    write_word(cpu, outputs + component * 4, (component + 1) * 0x11111111)
                for camera in range(3):
                    cpu.mem_write(physical(cameras + camera * 16), struct.pack(
                        "<4I", 0x1000 + camera * 0x10, 0x1001 + camera * 0x10,
                        0x1002 + camera * 0x10, 0x44332211 + camera,
                    ))
                cpu.mem_write(physical(channels), bytes((0x34, 0x56, 0x78)))
                calls = []

                def intercept(uc, address, size, data):
                    a0 = uc.reg_read(UC_MIPS_REG_A0)
                    a1 = uc.reg_read(UC_MIPS_REG_A1)
                    a2 = uc.reg_read(UC_MIPS_REG_A2)
                    if address == 0x80072798:
                        self.assertEqual(a1, cameras + a0 * 16)
                        calls.append((1, a0, read_word(uc, a1 + 4), uc.mem_read(physical(a1 + 13), 1)[0]))
                        if mutation:
                            write_word(uc, a1 + 4, 0xFFFFFFF7)
                            uc.mem_write(physical(a1 + 13), b"\xA5")
                        uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFF9)
                    elif (address == 0x80072DF0 and not integrated) or (
                        address == 0x80077874 and integrated
                    ):
                        calls.append((2, a0, a1, a2))
                        if mutation:
                            uc.mem_write(physical(channels), b"\xEE\xEE\xEE")
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                if operation == 0:
                    pointers = [globals_address + component * 4 for component in range(3)]
                    pointers.extend(outputs + component * 4 for component in range(3))
                    selectors = (aliases & 7, aliases >> 3 & 7, aliases >> 6 & 7)
                    for register, selector in zip((UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2), selectors):
                        cpu.reg_write(register, pointers[selector])
                    self.call(cpu, 0x80028100)
                    result = 0xFFFFFFFF
                elif operation == 1:
                    cpu.reg_write(UC_MIPS_REG_A0, camera_index)
                    cpu.reg_write(UC_MIPS_REG_A1, values if aliases == 0 else cameras + (aliases - 1) * 16)
                    self.call(cpu, 0x8004EDEC)
                    result = cpu.reg_read(UC_MIPS_REG_V0)
                elif operation == 2:
                    cpu.reg_write(UC_MIPS_REG_A0, first)
                    cpu.reg_write(UC_MIPS_REG_A1, second)
                    cpu.reg_write(UC_MIPS_REG_A2, third)
                    self.call(cpu, 0x8004EF10)
                    result = cpu.reg_read(UC_MIPS_REG_V0)
                else:
                    self.assertTrue(integrated)
                    cpu.reg_write(UC_MIPS_REG_A0, first)
                    cpu.reg_write(UC_MIPS_REG_A1, second)
                    cpu.reg_write(UC_MIPS_REG_A2, third)
                    self.call(cpu, 0x80072DF0)
                    result = 0xFFFFFFFF
                self.assertLessEqual(len(calls), 1)
                call = calls[0] if calls else (0, 0, 0, 0)
                original = (
                    result,
                    *(read_word(cpu, globals_address + component * 4) for component in range(3)),
                    *(read_word(cpu, outputs + component * 4) for component in range(3)),
                    *(read_word(cpu, cameras + camera * 16 + component * 4)
                      for camera in range(3) for component in range(3)),
                    *(read_word(cpu, cameras + camera * 16 + 12) for camera in range(3)),
                    *cpu.mem_read(physical(channels), 3),
                    len(calls), *call,
                )
                self.assertEqual(native, original)
