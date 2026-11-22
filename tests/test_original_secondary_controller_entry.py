import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_S4, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<9I")
OUTPUT = struct.Struct("<36I")
CONTEXT = 0x801E4000
ACTOR = 0x801E4100
ACTOR_OWNER = 0x801E4200
FIRST_OWNER = 0x801E4300
SECOND_OWNER = 0x801E4400
EXTRA_HANDLE = 0x800BE0C4
INITIALIZED = GP + 0x518
TIMER = GP + 0x51A
FLAG = GP + 0x506


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerEntryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_entry_branches_and_callback_mutations_against_original(self):
        rng = random.Random(0x49454)
        cases = []
        for initialized, timer, duration, mode, flag, buttons in itertools.product(
            (0, 1, 2), (0, 88, 89, 90, 0x7FFF, 0x8000, 0xFFFF),
            (0, 1), (0, 1), (0, 1, 2), (0, 0x80)
        ):
            cases.append((buttons, initialized, timer, duration, mode, flag, 0x1234, 0, 0x3456))
        cases.extend((
            rng.getrandbits(32), rng.choice((0, 1, 2, 255)),
            rng.choice((0, 88, 89, 0x7FFF, 0x8000, 0xFFFF)),
            rng.choice((0, 1, 0xFFFFFFFF)), rng.choice((0, 1, 2, 0x100)),
            rng.choice((0, 1, 2, 255)), rng.getrandbits(32), rng.randrange(32),
            rng.randrange(0xFFFFFFF8),
        ) for _ in range(600))
        native_results = run_native_probe(
            self, ["src/game/secondary_controller_entry.c", "tests/test_secondary_controller_entry.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def intercept(uc, address, size, data):
            if address in (0x800495F4, 0x8004A0F4):
                current["outcome"] = int(address == 0x800495F4)
                uc.emu_stop()
                return
            call = {
                0x80026648: 1, 0x8002CA4C: 2, 0x80052094: 3, 0x800528A4: 4,
                0x80028764: 5, 0x8004753C: 6, 0x8001B558: 7, 0x8001BC28: 8,
                0x8002BD0C: 9, 0x8003AA1C: 10, 0x8002DF24: 11,
                0x8001BC08: 12, 0x8001BCE4: 13, 0x80056678: 14,
            }.get(address)
            if call is None:
                return
            current["calls"].append(call)
            a0, a1, a2, a3 = (
                uc.reg_read(register) & 0xFFFFFFFF
                for register in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3)
            )
            if call == 1:
                self.assertEqual(a0, current["buttons"])
                uc.reg_write(UC_MIPS_REG_V0, CONTEXT)
            elif call == 2:
                self.assertEqual((a0, a1), (17, 240))
                if current["mutations"] & 1:
                    uc.mem_write(physical(TIMER), struct.pack("<H", 89))
            elif call == 3:
                self.assertEqual((a0, a1), (FIRST_OWNER, 0))
                self.assertEqual(uc.mem_read(physical(INITIALIZED), 1)[0], 1)
                current["read_owner"] = 1
                uc.mem_write(physical(a2), struct.pack("<8I", *(current["seed"] + i for i in range(8))))
                if current["mutations"] & 2:
                    write32(EXTRA_HANDLE, SECOND_OWNER)
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            elif call == 4:
                self.assertEqual(a1, 0)
                current["write_owner"] = 1 if a0 == FIRST_OWNER else 2
                self.assertIn(a0, (FIRST_OWNER, SECOND_OWNER))
                current["record"] = list(struct.unpack("<8I", uc.mem_read(physical(a2), 32)))
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            elif call == 5:
                self.assertEqual(a0, 0)
                uc.reg_write(UC_MIPS_REG_V0, ACTOR)
            elif call == 6:
                uc.reg_write(UC_MIPS_REG_V0, current["selection"])
            elif call == 7:
                self.assertEqual((a0, a1, a2, a3), (23, 5, 0, current["selection"]))
                self.assertEqual(read32(ACTOR_OWNER + 0x20), 0)
                sp = uc.reg_read(UC_MIPS_REG_SP)
                self.assertEqual(bytes(uc.mem_read(physical(sp + 0x10), 16)), bytes(16))
                current["heading"] = a3
            elif call == 9:
                self.assertEqual(a0, 17)
                if current["mutations"] & 4:
                    uc.mem_write(physical(FLAG), b"\x01")
                uc.reg_write(UC_MIPS_REG_V0, current["duration"])
            elif call == 10:
                write32(a0, CONTEXT)
                uc.reg_write(UC_MIPS_REG_V0, 0)
            elif call == 11:
                self.assertEqual(a0, 0x80013788)
                self.assertEqual(uc.mem_read(physical(CONTEXT + 0x2B), 1), b"\x01")
                self.assertEqual(uc.mem_read(physical(CONTEXT + 0x6D), 1), b"\x02")
                self.assertEqual(uc.mem_read(physical(TIMER), 2), b"\x00\x00")
            elif call == 13:
                if current["mutations"] & 8:
                    uc.mem_write(physical(FLAG), b"\x01")
                uc.reg_write(UC_MIPS_REG_V0, current["mode"] & 0xFF)
            elif call == 14:
                self.assertEqual((a0, a1), (current["buttons"], 7))
                if current["mutations"] & 16:
                    uc.mem_write(physical(FLAG), b"\x02")
                uc.reg_write(UC_MIPS_REG_V0, (a0 >> 7) & 1)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            buttons, initialized, timer, duration, mode, flag, selection, mutations, seed = entry
            current.update(
                buttons=buttons, duration=duration, mode=mode, selection=selection,
                mutations=mutations, seed=seed, calls=[], heading=0xFFFFFFFF,
                read_owner=0, write_owner=0, record=[0] * 8, outcome=None,
            )
            cpu.mem_write(physical(CONTEXT), bytes(0x80))
            cpu.mem_write(physical(CONTEXT + 0x2B), b"\x55")
            cpu.mem_write(physical(CONTEXT + 0x6D), b"\x66")
            write32(ACTOR + 0x1C, ACTOR_OWNER)
            write32(ACTOR_OWNER + 0x20, 0xABCDEF01)
            write32(EXTRA_HANDLE, FIRST_OWNER)
            cpu.mem_write(physical(INITIALIZED), bytes((initialized & 0xFF,)))
            cpu.mem_write(physical(TIMER), struct.pack("<H", timer & 0xFFFF))
            cpu.mem_write(physical(FLAG), bytes((flag & 0xFF,)))
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_A0, buttons)
            cpu.emu_start(0x80049454, 0, count=10000)
            self.assertIsNotNone(current["outcome"], f"case {index} did not reach a boundary")
            calls = current["calls"] + [0] * (16 - len(current["calls"]))
            original = (
                current["outcome"], cpu.mem_read(physical(INITIALIZED), 1)[0],
                struct.unpack("<H", cpu.mem_read(physical(TIMER), 2))[0],
                cpu.mem_read(physical(FLAG), 1)[0], read32(ACTOR_OWNER + 0x20),
                cpu.mem_read(physical(CONTEXT + 0x2B), 1)[0],
                cpu.mem_read(physical(CONTEXT + 0x6D), 1)[0], current["heading"],
                int(cpu.reg_read(UC_MIPS_REG_S4) == CONTEXT), len(current["calls"]), *calls,
                *current["record"], current["read_owner"], current["write_owner"],
            )
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
