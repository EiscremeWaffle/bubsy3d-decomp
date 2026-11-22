import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_PC, UC_MIPS_REG_RA,
        UC_MIPS_REG_S2, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<13I")
OUTPUT = struct.Struct("<33I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerRotationButtonsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_button_rotation_sequence_against_original(self):
        rng = random.Random(0x49664)
        directions = (
            0, 1, 359, 360, 361, 0x7FFF, 0x8000, 0xFFFF7FFF, 0xFFFFFFFF,
            0x7FFFFFFF, 0x80000000, 0x80000001, 0x00080000, 0xFFF80000,
        )
        cases = []
        for mask, direction in itertools.product(range(16), directions):
            buttons = sum(((mask >> index) & 1) << bit for index, bit in enumerate((13, 15, 12, 14)))
            cases.append((buttons, direction, 0, 0, 0, *range(8)))
        cases.extend((
            rng.getrandbits(32), rng.getrandbits(32),
            *(rng.getrandbits(32) for _ in range(3)),
            *(rng.getrandbits(32) for _ in range(8)),
        ) for _ in range(2000))
        native_results = run_native_probe(
            self,
            ["src/game/secondary_controller_rotation.c",
             "tests/test_secondary_controller_rotation_buttons.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def intercept(uc, address, size, data):
            if address == 0x800497A4:
                current["stopped"] = True
                uc.emu_stop()
                return
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            if address == 0x80056678:
                self.assertEqual(a0, current["buttons"])
                self.assertIn(a1, (12, 13, 14, 15))
                current["calls"].append(a1)
                uc.reg_write(UC_MIPS_REG_V0, (a0 >> a1) & 1)
            elif address in (0x80048FA8, 0x80049080):
                call = 0x100 if address == 0x80048FA8 else 0x101
                self.assertEqual((a0, a1), (current["stack"] + 0x78, current["stack"] + 0x20))
                current["calls"].append(call)
                current["rotations"].extend(struct.unpack("<3I", uc.mem_read(physical(a1), 12)))
                first = struct.unpack("<I", uc.mem_read(physical(a0), 4))[0]
                uc.mem_write(physical(a0), struct.pack("<I", first ^ call))
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            buttons, direction, *values = entry
            rotation, basis = values[:3], values[3:]
            stack = STACK - 0x128
            current.update(buttons=buttons, stack=stack, calls=[], rotations=[], stopped=False)
            cpu.mem_write(physical(stack), bytes(0x128))
            cpu.mem_write(physical(stack + 0x20), struct.pack("<3I", *rotation))
            cpu.mem_write(physical(stack + 0x34), struct.pack("<I", direction))
            cpu.mem_write(physical(stack + 0x78), struct.pack("<8I", *basis))
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.reg_write(UC_MIPS_REG_S2, buttons)
            cpu.emu_start(0x80049664, 0, count=5000)
            self.assertTrue(current["stopped"], f"case {index} did not reach the boundary")
            calls = current["calls"] + [0] * (8 - len(current["calls"]))
            rotations = current["rotations"] + [0] * (12 - len(current["rotations"]))
            original = (
                len(current["calls"]), *calls, len(current["rotations"]) // 3, *rotations,
                *struct.unpack("<3I", cpu.mem_read(physical(stack + 0x20), 12)),
                *struct.unpack("<8I", cpu.mem_read(physical(stack + 0x78), 32)),
            )
            self.assertEqual(native, original, f"case {index}: {entry}")

    def test_position_clamping_against_original(self):
        rng = random.Random(0x4980C)
        input_layout = struct.Struct("<6I")
        output_layout = struct.Struct("<4I")
        positions = (0, 1, 319, 320, 321, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        directions = (0, 949, 950, 951, 0xFFFFFC62, 0xFFFFFC63, 0xFFFFFC64, 0x80000000, 0x7FFFFFFF)
        cases = [(0xFFFFFFFF, position, 0x7FFFFFFF, 2, direction, 0x80000000)
                 for position, direction in itertools.product(positions, directions)]
        cases.extend(tuple(rng.getrandbits(32) for _ in range(6)) for _ in range(2000))
        native_results = run_native_probe(
            self,
            ["src/game/secondary_controller_rotation.c",
             "tests/test_secondary_controller_rotation_position.c"],
            cases, input_layout, output_layout,
        )
        cpu = self.fresh_cpu()
        current = {}

        def intercept(uc, address, size, data):
            if address in (0x80049878, 0x8004989C):
                current["update"] = int(address == 0x80049878)
                uc.emu_stop()

        cpu.hook_add(UC_HOOK_CODE, intercept)
        stack = STACK - 0x128
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            current["update"] = None
            cpu.mem_write(physical(stack + 0x40), struct.pack("<3I", *entry[:3]))
            cpu.mem_write(physical(stack + 0x30), struct.pack("<3I", *entry[3:]))
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.emu_start(0x8004980C, 0, count=5000)
            self.assertIsNotNone(current["update"], f"case {index} did not reach a boundary")
            original = (current["update"], *struct.unpack("<3I", cpu.mem_read(physical(stack + 0x40), 12)))
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
