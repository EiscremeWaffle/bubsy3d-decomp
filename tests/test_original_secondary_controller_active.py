import itertools
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_PC, UC_MIPS_REG_RA,
        UC_MIPS_REG_S2, UC_MIPS_REG_S6, UC_MIPS_REG_S7, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<7I")
OUTPUT = struct.Struct("<11I")
GATE = 0x8009F58C
COUNT = 0x801D8994
RECORDS = 0x801D89A4
RECORD = 0x801E4400
OBJECTS = (0x801E4000, 0x801E4100)
OWNERS = (0x801E4200, 0x801E4300)
COMPONENTS = (0x801E4500, 0x801E4600)
FLAG506 = GP + 0x506
FLAG507 = GP + 0x507


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerActiveTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_active_gate_against_original(self):
        cases = [
            (0x12345680, response, gate, count, 3, flag507, mutations)
            for response, gate, count, flag507, mutations in itertools.product(
                (0, 1, 0x100, 0x101), (0, 1, 2), (0xFFFFFFFF, 0, 1),
                (0, 1, 2), range(16)
            )
        ]
        native_results = run_native_probe(
            self, ["src/game/secondary_controller_active.c", "tests/test_secondary_controller_active.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def intercept(uc, address, size, data):
            if address in (0x8004A0F4, 0x80049B08):
                current["outcome"] = int(address == 0x80049B08)
                uc.emu_stop()
                return
            a0, a1, a2 = (uc.reg_read(reg) & 0xFFFFFFFF for reg in
                          (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2))
            if address == 0x80056678:
                self.assertEqual((a0, a1), (current["buttons"], 7))
                current["calls"].append(1)
                if current["mutations"] & 1:
                    write32(GATE, read32(GATE) ^ 1)
                if current["mutations"] & 2:
                    write32(COUNT, 1)
                uc.reg_write(UC_MIPS_REG_V0, current["response"])
            elif address == 0x80040FC0:
                self.assertEqual(a0, 0)
                current["calls"].append(2)
                if current["mutations"] & 4:
                    write32(GATE, read32(GATE) ^ 1)
                    write32(OBJECTS[0] + 0x1C, OWNERS[1])
                uc.reg_write(UC_MIPS_REG_V0, OBJECTS[0])
            elif address == 0x8001C6BC:
                self.assertEqual((a0, a1, a2), (4, 5, 1))
                current["calls"].append(3)
                if current["mutations"] & 8:
                    uc.mem_write(physical(FLAG507), b"\1")
                    for owner in OWNERS:
                        write32(owner + 0x14, COMPONENTS[1])
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        stack = STACK - 0x128
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            buttons, response, gate, count, flag506, flag507, mutations = entry
            current.update(buttons=buttons, response=response, mutations=mutations, calls=[], outcome=None)
            write32(GATE, gate)
            write32(COUNT, count)
            write32(RECORDS, RECORD)
            write32(RECORD + 0x1C, OBJECTS[1])
            for number in range(2):
                write32(OBJECTS[number] + 0x1C, OWNERS[number])
                write32(OWNERS[number] + 0x14, COMPONENTS[number])
            cpu.mem_write(physical(FLAG506), bytes((flag506,)))
            cpu.mem_write(physical(FLAG507), bytes((flag507,)))
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.reg_write(UC_MIPS_REG_A0, buttons)
            cpu.reg_write(UC_MIPS_REG_S2, buttons)
            cpu.reg_write(UC_MIPS_REG_S6, 0)
            cpu.reg_write(UC_MIPS_REG_S7, 0)
            cpu.emu_start(0x80049A24, 0, count=1000)
            self.assertIsNotNone(current["outcome"], f"case {index} did not reach a boundary")
            object_value = cpu.reg_read(UC_MIPS_REG_S6)
            component_value = cpu.reg_read(UC_MIPS_REG_S7)
            calls = current["calls"] + [0] * (3 - len(current["calls"]))
            original = (
                current["outcome"], OBJECTS.index(object_value) + 1 if object_value in OBJECTS else 0,
                COMPONENTS.index(component_value) + 1 if component_value in COMPONENTS else 0,
                read32(GATE), read32(COUNT), cpu.mem_read(physical(FLAG506), 1)[0],
                cpu.mem_read(physical(FLAG507), 1)[0], len(current["calls"]), *calls,
            )
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
