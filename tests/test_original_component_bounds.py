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


INPUT = struct.Struct("<20I")
OUTPUT = struct.Struct("<11I")
OWNER = 0x801E0000
BOUNDS = 0x801E0100


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalComponentBoundsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_bounds_against_original(self):
        generator = random.Random(0x58C74)
        cases = [
            (first_status, second_status, generator.getrandbits(32), mutation,
             *(generator.getrandbits(32) for _ in range(16)))
            for first_status, second_status, mutation in itertools.product((0, 1, 0xFFFFFFFF), repeat=3)
            for _ in range(16)
        ]
        for value in (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF):
            cases.append((0, 0, value, 0, *([0] * 8), 0, value, 0, 0, 0, value, 0, 0))
        native_outputs = run_native_probe(
            self, ["src/game/func_80058C74.c", "tests/test_component_bounds.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            if address == 0x800529F8:
                self.assertEqual((current["calls"], a0), (0, OWNER))
                uc.mem_write(physical(a1), struct.pack("<I", current["index"]))
                if current["mutation"]:
                    value = struct.unpack("<I", uc.mem_read(physical(BOUNDS), 4))[0]
                    uc.mem_write(physical(BOUNDS), struct.pack("<I", value ^ 0xA5A5A5A5))
                status = current["first_status"]
            elif address == 0x80052094:
                self.assertEqual((current["calls"], a0), (1, OWNER))
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A2), BOUNDS)
                current["observed_index"] = a1
                uc.mem_write(physical(BOUNDS), struct.pack("<8I", *current["record"]))
                status = current["second_status"]
            else:
                return
            current["calls"] += 1
            uc.reg_write(UC_MIPS_REG_V0, status)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            first_status, second_status, record_index, mutation = entry[:4]
            initial, record = entry[4:12], entry[12:20]
            current.update(
                calls=0, first_status=first_status, second_status=second_status,
                index=record_index, mutation=mutation, record=record, observed_index=0xFFFFFFFF,
            )
            cpu.mem_write(physical(BOUNDS), struct.pack("<8I", *initial))
            cpu.reg_write(UC_MIPS_REG_A0, OWNER)
            cpu.reg_write(UC_MIPS_REG_A1, BOUNDS)
            self.call(cpu, 0x80058C74)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, current["calls"], current["observed_index"],
                *struct.unpack("<8I", cpu.mem_read(physical(BOUNDS), 32)),
            )
            self.assertEqual(native, original, f"case {index}: {entry}")
