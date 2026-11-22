import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_PC,
        UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<15I")
OUTPUT = struct.Struct("<12I")
OBJECT = 0x801E0000
RESULT = 0x801E0100


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryExtentTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_extent_against_original(self):
        generator = random.Random(0x59274)
        values = (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        cases = []
        for alias, difference in itertools.product((0, 1), itertools.product(values, repeat=3)):
            cases.append((
                *(generator.getrandbits(32) for _ in range(8)), *difference,
                generator.getrandbits(32), generator.getrandbits(32), alias,
                generator.getrandbits(32),
            ))
        for _ in range(128):
            cases.append(tuple(generator.getrandbits(32) for _ in range(13)) +
                         (generator.randrange(2), generator.getrandbits(32)))
        sources = ["src/game/func_80059274.c", "tests/test_secondary_extent.c"]
        native = run_native_probe(self, sources, cases, INPUT, OUTPUT)
        target_form = run_native_probe(
            self, sources, cases, INPUT, OUTPUT, compiler_flags=("-DBUBSY3D_MATCH_ORIGINAL_L0",),
        )
        cpu = self.fresh_cpu()
        current = {}

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            if address == 0x80058C74:
                self.assertEqual(current["calls"], 0)
                self.assertEqual(a0, OBJECT)
                current["bounds_address"] = a1
                uc.mem_write(physical(a1), struct.pack("<8I", *current["bounds"]))
                uc.mem_write(physical(OBJECT), struct.pack("<I", current["object"] ^ 0x12345678))
                uc.reg_write(UC_MIPS_REG_V0, current["bounds_status"])
            elif address == 0x80010000:
                self.assertEqual(current["calls"], 1)
                self.assertEqual(a0, current["bounds_address"] + 16)
                self.assertEqual(a1, current["bounds_address"])
                current["seen"] = struct.unpack("<8I", uc.mem_read(physical(a1), 32))
                output = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
                uc.mem_write(physical(output), struct.pack("<3I", *current["difference"]))
                uc.reg_write(UC_MIPS_REG_V0, current["subtract_status"])
            else:
                return
            current["calls"] += 1
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native_result, target_result) in enumerate(zip(cases, native, target_form)):
            bounds, difference = entry[:8], entry[8:11]
            bounds_status, subtract_status, alias, object_word = entry[11:]
            output_address = OBJECT if alias else RESULT
            current.update(
                bounds=bounds, difference=difference, bounds_status=bounds_status,
                subtract_status=subtract_status, object=object_word, calls=0,
            )
            cpu.mem_write(physical(OBJECT), struct.pack("<I", object_word))
            cpu.mem_write(physical(RESULT), bytes(4))
            cpu.reg_write(UC_MIPS_REG_A0, OBJECT)
            cpu.reg_write(UC_MIPS_REG_A1, output_address)
            self.call(cpu, 0x80059274)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                struct.unpack("<I", cpu.mem_read(physical(output_address), 4))[0],
                current["calls"],
                struct.unpack("<I", cpu.mem_read(physical(OBJECT), 4))[0],
                *current["seen"],
            )
            self.assertEqual(native_result, original, f"native case {index}: {entry}")
            self.assertEqual(target_result, original, f"target-form case {index}: {entry}")

    def test_extent_with_real_bounds_and_subtraction(self):
        generator = random.Random(0x5927458C74)
        cases = [
            tuple(generator.randint(-1000, 1000) & 0xFFFFFFFF for _ in range(8)) +
            (0, 0, 0, 0, 0, alias, generator.getrandbits(32))
            for alias in (0, 1) for _ in range(64)
        ]
        native = run_native_probe(
            self, ["src/game/func_80059274.c", "src/game/func_80058C74.c",
                   "tests/test_secondary_extent.c"],
            cases, INPUT, OUTPUT, compiler_flags=("-DBUBSY3D_EXTENT_REAL_BOUNDS",),
        )
        cpu = self.fresh_cpu()
        current = {}

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            if address == 0x800529F8:
                self.assertEqual(current["calls"], 0)
                self.assertEqual(a0, OBJECT)
                uc.mem_write(physical(a1), struct.pack("<I", 0))
                uc.reg_write(UC_MIPS_REG_V0, 0)
            elif address == 0x80052094:
                self.assertEqual(current["calls"], 1)
                self.assertEqual((a0, a1), (OBJECT, 0))
                output = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
                uc.mem_write(physical(output), struct.pack("<8I", *current["bounds"]))
                uc.mem_write(physical(OBJECT), struct.pack("<I", current["object"] ^ 0x12345678))
                uc.reg_write(UC_MIPS_REG_V0, 0)
            elif address == 0x80010000:
                self.assertEqual(current["calls"], 2)
                self.assertEqual(a0, a1 + 16)
                current["seen"] = struct.unpack("<8I", uc.mem_read(physical(a1), 32))
                for upper, lower in zip(current["seen"][4:7], current["seen"][:3]):
                    signed_upper = struct.unpack("<i", struct.pack("<I", upper))[0]
                    signed_lower = struct.unpack("<i", struct.pack("<I", lower))[0]
                    self.assertLessEqual(-0x80000000, signed_upper - signed_lower)
                    self.assertLessEqual(signed_upper - signed_lower, 0x7FFFFFFF)
                current["calls"] += 1
                return
            else:
                return
            current["calls"] += 1
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native_result) in enumerate(zip(cases, native)):
            bounds, alias, object_word = entry[:8], entry[13], entry[14]
            output_address = OBJECT if alias else RESULT
            current.update(bounds=bounds, object=object_word, calls=0)
            cpu.mem_write(physical(OBJECT), struct.pack("<I", object_word))
            cpu.mem_write(physical(RESULT), bytes(4))
            cpu.reg_write(UC_MIPS_REG_A0, OBJECT)
            cpu.reg_write(UC_MIPS_REG_A1, output_address)
            self.call(cpu, 0x80059274)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                struct.unpack("<I", cpu.mem_read(physical(output_address), 4))[0],
                current["calls"],
                struct.unpack("<I", cpu.mem_read(physical(OBJECT), 4))[0],
                *current["seen"],
            )
            self.assertEqual(native_result, original, f"case {index}: {entry}")
