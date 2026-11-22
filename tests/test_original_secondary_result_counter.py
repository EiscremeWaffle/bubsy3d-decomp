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


INPUT = struct.Struct("<8I")
OUTPUT = struct.Struct("<8I")
CONTEXT = 0x801E0000
EVENT = 0x801E0100
EVENT_GLOBAL = 0x800BDFCC
SELECTION_GLOBAL = 0x800BE674


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryResultCounterTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_result_counter_against_original(self):
        generator = random.Random(0x2E348)
        values = (0, 1, 0x7FFF, 0x8000, 0xFFFF, 0xFFFFFFFF, 0x80000000)
        cases = [
            (delta, selection, generator.getrandbits(32), counter, event, mutation,
             replacement, generator.getrandbits(32))
            for delta, selection, counter, event, mutation, replacement in itertools.product(
                values, (0, 1, 0xFFFF), values, (0, 1), (0, 1), (0, 1)
            )
        ]
        native = run_native_probe(
            self, ["src/game/func_8002E348.c", "tests/test_secondary_result_counter.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read32(uc, address):
            return struct.unpack("<I", uc.mem_read(physical(address), 4))[0]

        def write32(uc, address, value):
            uc.mem_write(physical(address), struct.pack("<I", value))

        def read16(uc, address):
            return struct.unpack("<H", uc.mem_read(physical(address), 2))[0]

        def write16(uc, address, value):
            uc.mem_write(physical(address), struct.pack("<H", value & 0xFFFF))

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            if address == 0x80026648:
                self.assertEqual(current["calls"], 0)
                self.assertEqual(a0, current["delta"])
                status = CONTEXT
            elif address == 0x8004FF50:
                self.assertEqual(current["calls"], 1)
                self.assertEqual((a0, a1, a2), (0x8002E2DC, 600, EVENT_GLOBAL))
                self.assertEqual(read32(uc, EVENT_GLOBAL), 0)
                current["scheduled_selection"] = read16(uc, SELECTION_GLOBAL)
                write32(uc, EVENT_GLOBAL, EVENT)
                if current["mutation"]:
                    write16(uc, CONTEXT + 0x14, current["replacement"])
                status = current["status"]
            elif address == 0x8002BB64:
                self.assertIn(current["calls"], (1, 2))
                current["chosen"] = a0
                status = 0
            else:
                return
            current["calls"] += 1
            uc.reg_write(UC_MIPS_REG_V0, status)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native_result) in enumerate(zip(cases, native)):
            delta, selection, first, second, event, mutation, replacement, status = entry
            current.update(
                delta=delta, mutation=mutation, replacement=replacement, status=status,
                calls=0, chosen=0, scheduled_selection=0,
            )
            cpu.mem_write(physical(CONTEXT), bytes(0x40))
            write16(cpu, CONTEXT + 0x14, selection)
            write32(cpu, CONTEXT + 0x28, first)
            write32(cpu, CONTEXT + 0x3C, second)
            write32(cpu, EVENT_GLOBAL, EVENT if event else 0)
            write16(cpu, SELECTION_GLOBAL, -1234)
            cpu.reg_write(UC_MIPS_REG_A0, delta)
            cpu.reg_write(UC_MIPS_REG_A1, 0xA5A5A5A5)
            self.call(cpu, 0x8002E348)
            original = (
                read16(cpu, CONTEXT + 0x14), read32(cpu, CONTEXT + 0x28),
                read32(cpu, CONTEXT + 0x3C), int(read32(cpu, EVENT_GLOBAL) != 0),
                read16(cpu, SELECTION_GLOBAL), current["calls"], current["chosen"],
                current["scheduled_selection"],
            )
            self.assertEqual(native_result, original, f"case {index}: {entry}")
