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
        UC_MIPS_REG_S2, UC_MIPS_REG_S4, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<8I")
OUTPUT = struct.Struct("<10I")
CONTEXT = 0x801E4000
GAME = 0x801E4100
FLAG = GP + 0x505


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerConfirmTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_confirmation_gate_against_original(self):
        cases = [
            (0x12345680, flag, selection, first, second, mode, response, mutations)
            for flag, selection, first, second, mode, response, mutations in itertools.product(
                (0, 1, 2), (0, 1, 0xFFFF), (0, 1), (0, 1),
                (4, 5, 6), (0, 1, 0x100, 0x101, 0xFFFFFFFF), range(16)
            )
        ]
        cases.extend((0x12345680, 0, selection, 0, 0, 4, 1, mutations)
                     for selection, mutations in itertools.product((0, 1), (16, 18)))
        native_results = run_native_probe(
            self, ["src/game/secondary_controller_confirm.c", "tests/test_secondary_controller_confirm.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def intercept(uc, address, size, data):
            if address in (0x8004A0F4, 0x80049A24, 0x80049920):
                current["outcome"] = {0x8004A0F4: 0, 0x80049A24: 1, 0x80049920: 2}[address]
                uc.emu_stop()
                return
            call = {0x80056678: 1, 0x8003AA1C: 2, 0x8001C6BC: 3}.get(address)
            if call is None:
                return
            current["calls"].append(call)
            a0, a1, a2 = (uc.reg_read(reg) & 0xFFFFFFFF for reg in
                          (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2))
            if call == 1:
                self.assertEqual((a0, a1), (current["buttons"], 7))
                if current["mutations"] & 1:
                    flag = uc.mem_read(physical(FLAG), 1)[0]
                    uc.mem_write(physical(FLAG), bytes((flag ^ 1,)))
                uc.reg_write(UC_MIPS_REG_V0, current["response"])
            elif call == 2:
                self.assertEqual(a0, current["stack"] + 0x98)
                if not current["mutations"] & 16:
                    write32(a0, GAME)
                if current["mutations"] & 2:
                    uc.mem_write(physical(CONTEXT + 0x14), b"\0\0")
                if current["mutations"] & 4:
                    slot = CONTEXT + (0x3C if current["selection"] == 0 else 0x28)
                    write32(slot, read32(slot) ^ 1)
                if current["mutations"] & 8:
                    mode = uc.mem_read(physical(GAME + 0x11), 1)[0]
                    uc.mem_write(physical(GAME + 0x11), bytes((mode ^ 1,)))
            else:
                self.assertEqual((a0, a1, a2), (4, 5, 1))
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        stack = STACK - 0x128
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            buttons, flag, selection, first, second, mode, response, mutations = entry
            current.update(buttons=buttons, selection=selection, response=response,
                           mutations=mutations, stack=stack, calls=[], outcome=None)
            cpu.mem_write(physical(CONTEXT), bytes(0x80))
            cpu.mem_write(physical(GAME), bytes(0x80))
            write32(stack + 0x98, 0)
            cpu.mem_write(physical(FLAG), bytes((flag,)))
            cpu.mem_write(physical(CONTEXT + 0x14), struct.pack("<H", selection))
            write32(CONTEXT + 0x28, first)
            write32(CONTEXT + 0x3C, second)
            cpu.mem_write(physical(GAME + 0x11), bytes((mode,)))
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.reg_write(UC_MIPS_REG_S2, buttons)
            cpu.reg_write(UC_MIPS_REG_S4, CONTEXT)
            cpu.emu_start(0x8004989C, 0, count=1000)
            self.assertIsNotNone(current["outcome"], f"case {index} did not reach a boundary")
            calls = current["calls"] + [0] * (3 - len(current["calls"]))
            original = (
                current["outcome"], cpu.mem_read(physical(FLAG), 1)[0],
                struct.unpack("<H", cpu.mem_read(physical(CONTEXT + 0x14), 2))[0],
                read32(CONTEXT + 0x28), read32(CONTEXT + 0x3C),
                cpu.mem_read(physical(GAME + 0x11), 1)[0], len(current["calls"]), *calls,
            )
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
