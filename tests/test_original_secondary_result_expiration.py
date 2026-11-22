import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical
from test_original_secondary_result_counter import CONTEXT, EVENT, EVENT_GLOBAL, SELECTION_GLOBAL

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_PC,
        UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<8I")
OUTPUT = struct.Struct("<16I")
REPLACEMENT_EVENT = 0x801E0200


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryResultExpirationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_expiration_with_real_counter_against_original(self):
        generator = random.Random(0x2E2DC)
        values = (0, 1, 0x7FFF, 0x8000, 0xFFFFFFFF)
        cases = [
            (selection, first, second, expiration, event, mutation, replacement,
             generator.getrandbits(32))
            for selection, first, second, expiration, event, mutation, replacement
            in itertools.product((0, 1, 0xFFFF), values, values, (0, 1, 0xFFFF),
                                 (0, 1), range(4), (0, 1))
        ]
        native = run_native_probe(
            self, ["src/game/func_8002E2DC.c", "src/game/func_8002E348.c",
                   "tests/test_secondary_result_expiration.c"],
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
                self.assertEqual(a0, 0 if current["getters"] == 0 else 1)
                current["trace"].append(1)
                if current["getters"] == 1 and current["mutation"] & 1:
                    write16(uc, CONTEXT + 0x14, current["replacement"])
                current["getters"] += 1
                status = CONTEXT
            elif address == 0x8004FF50:
                self.assertEqual((a0, a1, a2), (0x8002E2DC, 600, EVENT_GLOBAL))
                self.assertEqual(read32(uc, EVENT_GLOBAL), 0)
                current["trace"].append(2)
                write32(uc, EVENT_GLOBAL, EVENT)
                status = 0xFFFFFFFF
            elif address == 0x8002BB64:
                current["trace"].append(3)
                current["chosen"] = a0
                status = 0
            elif address == 0x8004FFEC:
                current["trace"].append(4)
                current["removed"] = int(a0 != 0)
                if current["mutation"] & 2:
                    write32(uc, EVENT_GLOBAL, REPLACEMENT_EVENT)
                status = current["status"]
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, status)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native_result) in enumerate(zip(cases, native)):
            selection, first, second, expiration, event, mutation, replacement, status = entry
            current.update(
                getters=0, mutation=mutation, replacement=replacement, status=status,
                trace=[], chosen=0, removed=0,
            )
            cpu.mem_write(physical(CONTEXT), bytes(0x40))
            write16(cpu, CONTEXT + 0x14, selection)
            write32(cpu, CONTEXT + 0x28, first)
            write32(cpu, CONTEXT + 0x3C, second)
            write16(cpu, SELECTION_GLOBAL, expiration)
            write32(cpu, EVENT_GLOBAL, EVENT if event else 0)
            cpu.reg_write(UC_MIPS_REG_A0, 0)
            cpu.reg_write(UC_MIPS_REG_A1, 0)
            self.call(cpu, 0x8002E2DC)
            trace = current["trace"]
            original = (
                read16(cpu, CONTEXT + 0x14), read32(cpu, CONTEXT + 0x28),
                read32(cpu, CONTEXT + 0x3C), read16(cpu, SELECTION_GLOBAL),
                int(read32(cpu, EVENT_GLOBAL) != 0), len(trace), current["chosen"],
                current["removed"], *trace, *([0] * (8 - len(trace))),
            )
            self.assertEqual(native_result, original, f"case {index}: {entry}")
