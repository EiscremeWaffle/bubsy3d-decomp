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
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3, UC_MIPS_REG_PC,
        UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<13I")
OUTPUT = struct.Struct("<19I")
FIRST = 0x801E0000
SECOND = 0x801E0100
BASIS = 0x801E0200
RESULT = 0x801E0300


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryDistanceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_distance_against_original(self):
        generator = random.Random(0x590B4)
        cases = [
            (*(generator.getrandbits(32) for _ in range(9)), alias, null_basis, output_alias, mutations)
            for alias, null_basis, output_alias, mutations
            in itertools.product((0, 1), (0, 1), (0, 1), range(4))
            for _ in range(4)
        ]
        native_outputs = run_native_probe(
            self, ["src/game/func_800590B4.c", "tests/test_secondary_distance.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            if address == 0x80058E7C:
                self.assertEqual(a1, 0 if current["null_basis"] else BASIS)
                self.assertEqual(a0, FIRST if current["calls"] == 0 else current["second"])
                uc.mem_write(physical(a2), bytes(uc.mem_read(physical(a0), 16)))
                if current["calls"] == 0:
                    current["first_position"] = a2
                    if current["mutations"] & 1:
                        value = struct.unpack("<I", uc.mem_read(physical(current["second"]), 4))[0]
                        uc.mem_write(physical(current["second"]), struct.pack("<I", value ^ 0xA5A5A5A5))
                elif current["mutations"] & 2:
                    value = struct.unpack("<I", uc.mem_read(physical(current["first_position"]), 4))[0]
                    uc.mem_write(physical(current["first_position"]), struct.pack("<I", value ^ 0x12345678))
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            elif address == 0x80062D4C:
                self.assertEqual(current["calls"], 2)
                self.assertEqual(a0, current["first_position"])
                current["first_sample"] = struct.unpack("<4I", uc.mem_read(physical(a0), 16))
                current["second_sample"] = struct.unpack("<4I", uc.mem_read(physical(a1), 16))
                self.assertEqual(a2, current["output"])
                uc.mem_write(physical(a2), struct.pack("<I", current["result"]))
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFE)
            else:
                return
            current["calls"] += 1
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            first, second = entry[:4], entry[4:8]
            result, alias, null_basis, output_alias, mutations = entry[8:]
            second_address = FIRST if alias else SECOND
            result_address = FIRST if output_alias else RESULT
            current.update(
                calls=0, second=second_address, null_basis=null_basis, output=result_address,
                result=result, mutations=mutations, first_sample=[0] * 4, second_sample=[0] * 4,
            )
            cpu.mem_write(physical(FIRST), struct.pack("<4I", *first))
            cpu.mem_write(physical(SECOND), struct.pack("<4I", *second))
            cpu.mem_write(physical(RESULT), bytes(4))
            cpu.reg_write(UC_MIPS_REG_A0, FIRST)
            cpu.reg_write(UC_MIPS_REG_A1, second_address)
            cpu.reg_write(UC_MIPS_REG_A2, 0 if null_basis else BASIS)
            cpu.reg_write(UC_MIPS_REG_A3, result_address)
            self.call(cpu, 0x800590B4)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, current["calls"],
                struct.unpack("<I", cpu.mem_read(physical(result_address), 4))[0],
                *current["first_sample"], *current["second_sample"],
                *struct.unpack("<4I", cpu.mem_read(physical(FIRST), 16)),
                *struct.unpack("<4I", cpu.mem_read(physical(second_address), 16)),
            )
            self.assertEqual(native, original, f"case {index}: {entry}")
