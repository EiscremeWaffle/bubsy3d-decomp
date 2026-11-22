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
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<10I")
OUTPUT = struct.Struct("<8I")
FIRST = 0x801E0000
SECOND = 0x801E0100
BASIS = 0x801E0200
OUTPUT_ADDRESS = 0x801E0300


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryExtentComparisonTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_extent_comparison_against_original(self):
        self.compare_extent(real_extent=False)

    def test_extent_comparison_with_real_extent_against_original(self):
        self.compare_extent(real_extent=True)

    def test_extent_comparison_with_real_distance_and_extent(self):
        self.compare_extent(real_extent=True, real_distance=True)

    def compare_extent(self, real_extent, real_distance=False):
        values = (0, 1, 0x3FFFFFFF, 0x40000000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        generator = random.Random(0x5C3A4)
        cases = [
            (first, second, distance, 0, 0, 0, 0, 0, 0, 0)
            for first, second, distance in itertools.product(values, repeat=3)
        ]
        cases.extend(
            (
                generator.getrandbits(32), generator.getrandbits(32), generator.getrandbits(32),
                mutations, alias, null_basis, output_alias,
                generator.getrandbits(32), generator.getrandbits(32), generator.getrandbits(32),
            )
            for mutations, alias, null_basis, output_alias in itertools.product(
                range(8), (0, 1), (0, 1), (0, 1)
            )
            for _ in range(4)
        )
        sources = ["src/game/func_8005C3A4.c", "tests/test_secondary_extent_comparison.c"]
        flags = ()
        if real_extent:
            sources.insert(1, "src/game/func_80059274.c")
            flags = ("-DBUBSY3D_COMPARISON_REAL_EXTENT",)
        if real_distance:
            sources.insert(1, "src/game/func_800590B4.c")
            flags += ("-DBUBSY3D_COMPARISON_REAL_DISTANCE",)
        native_outputs = run_native_probe(self, sources, cases, INPUT, OUTPUT, compiler_flags=flags)
        cpu = self.fresh_cpu()
        current = {}

        def read_word(uc, address):
            return struct.unpack("<I", uc.mem_read(physical(address), 4))[0]

        def write_word(uc, address, value):
            uc.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            if real_distance and address == 0x80058E7C:
                self.assertLess(current["position_samples"], 2)
                self.assertEqual(a0, FIRST if current["position_samples"] == 0 else current["second"])
                self.assertEqual(a1, 0 if current["null_basis"] else BASIS)
                destination = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
                uc.mem_write(physical(destination), bytes(12))
                current["position_samples"] += 1
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                return
            elif (not real_distance and address == 0x800590B4) or (real_distance and address == 0x80062D4C):
                self.assertEqual(current["calls"], 0)
                if real_distance:
                    self.assertEqual(current["position_samples"], 2)
                    destination = uc.reg_read(UC_MIPS_REG_A2)
                else:
                    self.assertEqual((a0, a1), (FIRST, current["second"]))
                    self.assertEqual(uc.reg_read(UC_MIPS_REG_A2), 0 if current["null_basis"] else BASIS)
                    destination = uc.reg_read(UC_MIPS_REG_A3)
                write_word(uc, destination, current["distance"])
                if current["mutations"] & 1:
                    write_word(uc, FIRST, read_word(uc, FIRST) ^ 0x80000000)
                if current["mutations"] & 2:
                    write_word(uc, current["second"], read_word(uc, current["second"]) ^ 0x7FFFFFFF)
                status = current["distance_status"]
            elif address == (0x80058C74 if real_extent else 0x80059274):
                if current["calls"] == 1:
                    self.assertEqual(a0, FIRST)
                    current["sampled_first"] = read_word(uc, a0)
                    sample = current["sampled_first"]
                    if current["mutations"] & 4:
                        write_word(uc, current["second"], read_word(uc, current["second"]) ^ 0x12345678)
                    status = current["first_status"]
                else:
                    self.assertEqual((current["calls"], a0), (2, current["second"]))
                    current["sampled_second"] = read_word(uc, a0)
                    sample = current["sampled_second"]
                    status = current["second_status"]
                if real_extent:
                    uc.mem_write(physical(a1), struct.pack("<8I", 0, 0, 0, 0, sample, 0, 0, 0))
                else:
                    write_word(uc, a1, sample)
            elif real_extent and address == 0x80010000:
                self.assertEqual(a0, a1 + 16)
                sample = read_word(uc, a0)
                destination = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
                uc.mem_write(physical(destination), struct.pack("<3I", sample, 0x80000000, 0x80000000))
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                return
            else:
                return
            current["calls"] += 1
            uc.reg_write(UC_MIPS_REG_V0, status)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            (first, second, distance, mutations, alias, null_basis, output_alias,
             distance_status, first_status, second_status) = entry
            second_address = FIRST if alias else SECOND
            output_address = FIRST if output_alias else OUTPUT_ADDRESS
            current.update(
                second=second_address, distance=distance, mutations=mutations, null_basis=null_basis,
                distance_status=distance_status, first_status=first_status, second_status=second_status,
                sampled_first=0, sampled_second=0, calls=0, position_samples=0,
            )
            write_word(cpu, FIRST, first)
            write_word(cpu, SECOND, second)
            cpu.mem_write(physical(OUTPUT_ADDRESS), b"\xA5")
            cpu.reg_write(UC_MIPS_REG_A0, FIRST)
            cpu.reg_write(UC_MIPS_REG_A1, second_address)
            cpu.reg_write(UC_MIPS_REG_A2, 0 if null_basis else BASIS)
            cpu.reg_write(UC_MIPS_REG_A3, output_address)
            self.call(cpu, 0x8005C3A4)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                cpu.mem_read(physical(output_address), 1)[0],
                cpu.mem_read(physical(OUTPUT_ADDRESS), 1)[0], current["calls"],
                read_word(cpu, FIRST), read_word(cpu, second_address),
                current["sampled_first"], current["sampled_second"],
            )
            self.assertEqual(native, original, f"case {index}: {entry}")
