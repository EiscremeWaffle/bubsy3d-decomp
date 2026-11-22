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


INPUT = struct.Struct("<9I")
OUTPUT = struct.Struct("<26I")
EMPTY = 0xFFFFFFFF
CAMERAS = 0x8018CA0C


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerCameraTransitionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_camera_transition_against_original(self):
        self.compare_camera_transition(real_camera=False)

    def test_camera_transition_with_real_setter_against_original(self):
        self.compare_camera_transition(real_camera=True)

    def compare_camera_transition(self, real_camera):
        generator = random.Random(0x27114)
        cases = []
        for first, second, mutation, value in itertools.product(
            (0, 1, 2, 0x100, 0x101, 0xFFFFFFFF),
            (0, 1, 2, 0x101, 0xFFFFFFFF),
            range(4),
            (0, 1, 7, 8, 127, 128, 254, 255),
        ):
            cases.append((first, second, value, 255 - value, value ^ 0x55,
                          generator.randrange(256), generator.randrange(256), generator.randrange(256), mutation))
        cases.extend(
            (generator.getrandbits(32), generator.getrandbits(32),
             *(generator.randrange(256) for _ in range(6)), generator.randrange(4))
            for _ in range(256)
        )
        sources = ["src/game/func_80027114.c", "tests/test_controller_camera_transition.c"]
        if real_camera:
            sources.append("src/game/func_8004ED78.c")
        native = run_native_probe(self, sources, cases, INPUT, OUTPUT,
                                  compiler_flags=("-DREAL_CAMERA",) if real_camera else ())
        cpu = self.fresh_cpu()
        current = {}
        calls = {
            0x8005F374: 1, 0x8004EF10: 2, 0x8004EFD4: 3, 0x80028E50: 4,
            0x8004EE60: 5, 0x8004ED78: 6, 0x80050824: 8, 0x8005F398: 9,
            0x80028EE0: 10, 0x80028EA8: 11, 0x8004EF98: 12,
            0x80028E20: 13, 0x800280D0: 14,
        }

        def intercept(uc, address, size, data):
            call = calls.get(address)
            if real_camera and address == 0x8004ED78:
                return
            if real_camera and address == 0x80072798:
                call = 15
            if call is None:
                return
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            if call == 2:
                self.assertEqual((a0, a1, uc.reg_read(UC_MIPS_REG_A2)), (0, 0, 0))
            elif call == 5:
                self.assertEqual(a0, 0)
                uc.mem_write(physical(a1), bytes(current["current"]))
            elif call == 6 or call == 15:
                self.assertIn(a0, (0, 1))
                if call == 15:
                    self.assertEqual(a1, CAMERAS + a0 * 16)
                    rotation_address = a1 + 12
                else:
                    rotation_address = a1
                call = 6 + a0
                rotation = bytearray(uc.mem_read(physical(rotation_address), 3))
                current["snapshots"][a0 * 3:a0 * 3 + 3] = rotation
                if current["mutation"] & (1 << a0):
                    for index in range(3):
                        rotation[index] ^= 0x51 + a0 + index
                    uc.mem_write(physical(rotation_address), bytes(rotation))
            elif call == 8:
                current["snapshots"][6:9] = uc.mem_read(physical(a0), 3)
            elif call == 14:
                uc.mem_write(physical(a0), bytes(current["restored"]))
            current["calls"].append(call)
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, expected) in enumerate(zip(cases, native)):
            first, second, *rest = entry
            current.update(current=rest[:3], restored=rest[3:6], mutation=rest[6],
                           calls=[], snapshots=[EMPTY] * 9)
            if real_camera:
                cpu.mem_write(physical(CAMERAS), bytes([0xA5]) * 48)
            cpu.reg_write(UC_MIPS_REG_A0, first)
            cpu.reg_write(UC_MIPS_REG_A1, second)
            self.call(cpu, 0x80027114)
            camera_state = [EMPTY] * 6
            if real_camera:
                camera_state = list(cpu.mem_read(physical(CAMERAS + 12), 3))
                camera_state += list(cpu.mem_read(physical(CAMERAS + 28), 3))
            observed = (
                len(current["calls"]), *(current["calls"] + [EMPTY] * (10 - len(current["calls"]))),
                *current["snapshots"], *camera_state,
            )
            self.assertEqual(observed, expected, f"case {index}: {entry}")
