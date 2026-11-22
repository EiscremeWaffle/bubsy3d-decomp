import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<22I")
OUTPUT = struct.Struct("<54I")
MOVEMENT_OUTPUT = struct.Struct("<64I")
ACTOR, COMPONENT, SHARED = 0x801E4000, 0x801E4100, 0x801E4200
TRANSFORM, REFERENCE, POSITION = 0x801E4400, 0x801E4500, 0x801E4600
POSITION_COMPONENT, POSITION_COMPONENT_GLOBAL = 0x801E4700, 0x800BE528
SAVED, LOCK, MODE, EFFECT = 0x80186440, 0x80186479, 0x800BE548, 0x800BE54C
LEVEL, OFFSET, SCALE, BASE = 0x801D36F0, 0x800BDEE8, 0x800BDEE4, 0x800BDEE0
EVENT_PARAMETERS = 0x801D05C4
INITIAL_VECTOR = 0x800129B8
MOVEMENT_ENABLED, MOVEMENT_PERMIT, MOVEMENT_MODE_COPY = 0x800BE024, 0x800BE020, 0x800BE022
MOVEMENT_ACTIVE, MOVEMENT_MODE, MOVEMENT_DURATION = 0x80186108, 0x80186109, 0x80186110
MOVEMENT_ELAPSED, MOVEMENT_STEP, MOVEMENT_ADJUSTMENT = 0x80186114, 0x80186118, 0x80186148
MOVEMENT_CALLBACK = 0x80186144


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerCycleWarpTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_cycle_warp_against_original(self):
        self.compare_cycle_warp(real_position_update=False, real_distance=False, real_leaves=False)

    def test_cycle_warp_with_real_position_update(self):
        self.compare_cycle_warp(real_position_update=True, real_distance=False, real_leaves=False)

    def test_cycle_warp_with_real_position_and_distance(self):
        self.compare_cycle_warp(real_position_update=True, real_distance=True, real_leaves=False)

    def test_cycle_warp_with_real_leaves(self):
        self.compare_cycle_warp(real_position_update=True, real_distance=True, real_leaves=True)

    def test_cycle_warp_with_movement_setup(self):
        self.compare_cycle_warp(real_position_update=True, real_distance=True,
                                real_leaves=True, real_movement=True)

    def test_cycle_warp_with_movement_target(self):
        self.compare_cycle_warp(real_position_update=True, real_distance=True, real_leaves=True,
                                real_movement=True, real_movement_target=True)

    def compare_cycle_warp(self, real_position_update, real_distance, real_leaves,
                           real_movement=False, real_movement_target=False):
        rng = random.Random(0x38430)
        cases = []
        for lock, flags, mode, level in itertools.product(
            (0, 1, 2, 3, 4, 255), (0, 0x2000, 0x100000, 0x102000),
            (0, 1), (10, 11, 12, 13, 14, 15, 16),
        ):
            words = [rng.getrandbits(32) for _ in range(22)]
            words[10:13] = flags, lock, mode
            words[16] = level
            words[21] = rng.randrange(8192)
            cases.append(tuple(words))
        for distance, level in itertools.product(
            (-0x80000000, -321, -160, -159, -1, 0, 159, 160, 319, 320, 0x7FFFFFFF),
            (10, 11, 12, 13, 14, 15, 16),
        ):
            words = [rng.getrandbits(32) for _ in range(22)]
            words[10:13] = 0, 3, rng.randrange(2)
            words[16], words[20], words[21] = level, distance & 0xFFFFFFFF, rng.randrange(8192)
            cases.append(tuple(words))
        for _ in range(2000):
            words = [rng.getrandbits(32) for _ in range(22)]
            words[10:13] = rng.choice((0, 0x2000)), 3, rng.randrange(2)
            words[16] = rng.choice((10, 11, 12, 13, 14, 15, 16))
            words[21] = rng.randrange(8192)
            cases.append(tuple(words))
        if real_distance:
            cases = [(*entry[:21], entry[21] & ~(64 | 128 | 256 | 512)) for entry in cases]
        if real_movement:
            cases = [(*entry[:21], entry[21] & ~(1024 | 2048)) for entry in cases]
        sources = ["src/game/func_80038430.c", "tests/test_controller_cycle_warp.c"]
        flags = []
        if real_position_update:
            sources.append("src/game/func_8002E910.c")
            flags.append("-DCYCLE_REAL_POSITION_UPDATE")
        if real_distance:
            sources.append("src/game/func_80022B5C.c")
            flags.append("-DCYCLE_REAL_DISTANCE")
        if real_leaves:
            sources.extend(["src/game/func_800226C0.c", "src/game/controller_cycle_event_parameters.c",
                            "src/game/func_8005668C.c", "src/game/func_80056678.c"])
            flags.append("-DCYCLE_REAL_LEAVES")
        if real_movement:
            sources.append("src/game/cycle_movement_setup.c")
            flags.append("-DCYCLE_MODEL_MOVEMENT")
        if real_movement_target:
            sources.append("src/game/grounded_transform.c")
            flags.append("-DCYCLE_MODEL_MOVEMENT_REAL_TARGET")
        native_results = run_native_probe(
            self, sources, cases, INPUT, MOVEMENT_OUTPUT if real_movement else OUTPUT, compiler_flags=flags
        )
        cpu = self.fresh_cpu()
        current = {}

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def write16(address, value):
            cpu.mem_write(physical(address), struct.pack("<H", value & 0xFFFF))

        def read16(address):
            return struct.unpack("<H", cpu.mem_read(physical(address), 2))[0]

        def write8(address, value):
            cpu.mem_write(physical(address), bytes((value & 0xFF,)))

        def read8(address):
            return cpu.mem_read(physical(address), 1)[0]

        def words(address, count):
            return struct.unpack("<%dI" % count, cpu.mem_read(physical(address), 4 * count))

        def xor_word(address, mask):
            write32(address, read32(address) ^ mask)

        def intercept(uc, address, size, data):
            args = tuple(uc.reg_read(reg) & 0xFFFFFFFF
                         for reg in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3))
            a0, a1, a2, a3 = args
            stack = uc.reg_read(UC_MIPS_REG_SP)
            result = 0
            calls = current["calls"]
            mutations = current["input"][21]
            if address == 0x80028764:
                self.assertEqual(a0, 0)
                calls.append(1)
                if mutations & 4096:
                    write32(INITIAL_VECTOR, 0x12345678)
                    write32(INITIAL_VECTOR + 12, 0x87654321)
                result = ACTOR
            elif address == 0x800226C0:
                self.assertEqual((a0, a1, read32(ACTOR + 4)), (ACTOR, 0x10, 0))
                if real_leaves:
                    return
                calls.append(2)
            elif address == 0x8001C700:
                self.assertIn((a0, a1), ((4, 3), (5, 5 if read8(MODE) else 8)))
                calls.append(3 if a0 == 4 else 4)
            elif address == 0x8001B558:
                self.assertEqual(a1, 4)
                self.assertEqual(words(stack + 0x10, 4), (0, 0, 0, 0))
                if a0 == 0x2E:
                    self.assertEqual((a2, a3), (0, 0))
                    calls.append(5)
                else:
                    self.assertEqual((a0, a2, a3), (0x13, read32(ACTOR + 0x0C), read32(ACTOR + 0x0C)))
                    calls.append(15)
            elif address == 0x80054AF0:
                self.assertEqual(a0, TRANSFORM)
                calls.append(6)
                uc.mem_write(physical(a1), struct.pack("<3I", *current["input"][4:7]))
                if mutations & 1:
                    xor_word(OFFSET, 0x80808080)
            elif address == 0x80054C7C:
                if real_position_update and a0 == POSITION_COMPONENT:
                    self.assertEqual((a1, a2), (0, POSITION))
                    calls.append(8)
                    current["adjusted_y"] = read32(POSITION + 4)
                    if mutations & 8:
                        xor_word(POSITION + 4, 0x80000000)
                    if mutations & 16:
                        xor_word(OFFSET, 0x02020202)
                    if mutations & 32:
                        xor_word(POSITION, 0x40404040)
                    uc.reg_write(UC_MIPS_REG_V0, 0)
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                    return
                self.assertEqual((a0, a1, a2), (TRANSFORM, 0, POSITION))
                calls.append(7)
                if mutations & 2:
                    xor_word(POSITION + 4, 0x10101010)
                if mutations & 4:
                    xor_word(OFFSET, 0x01010101)
            elif address == 0x8002E910:
                if real_position_update:
                    return
                self.assertEqual(a0, POSITION)
                calls.append(8)
                current["adjusted_y"] = read32(POSITION + 4)
                if mutations & 8:
                    xor_word(POSITION + 4, 0x80000000)
                if mutations & 16:
                    xor_word(OFFSET, 0x02020202)
                if mutations & 32:
                    xor_word(POSITION, 0x40404040)
            elif address == 0x80044C1C:
                self.assertEqual(a0, 1)
                if real_leaves:
                    return
                calls.append(9)
                result = 0x12345678
            elif address == 0x80044C04:
                self.assertEqual((a0, a1), (1, 12))
                if real_leaves:
                    return
                calls.append(10)
                write32(EVENT_PARAMETERS + 4, a1)
            elif address == 0x800516F8:
                calls.append(11)
                write32(a0, REFERENCE)
            elif address == 0x80054B20:
                self.assertEqual(a1, 0)
                if real_movement_target and a0 == POSITION_COMPONENT:
                    calls.append(19)
                else:
                    self.assertEqual(a0, REFERENCE)
                    calls.append(12)
                uc.mem_write(physical(a2), struct.pack("<3I", *current["input"][7:10]))
            elif address == 0x80022B5C:
                self.assertEqual(a0, POSITION)
                self.assertEqual(words(a1, 3), current["input"][7:10])
                if real_distance:
                    return
                calls.append(13)
                if mutations & 64:
                    xor_word(SCALE, 0x80808080)
                if mutations & 128:
                    write16(LEVEL, 14)
                if mutations & 256:
                    xor_word(ACTOR + 0x0C, 0x20202020)
                if mutations & 512:
                    xor_word(POSITION + 4, 0x40404040)
                result = current["input"][20]
            elif address == 0x8002EB70:
                self.assertEqual(words(stack + 0x10, 4), (0, 0x80038368, 0, 0))
                calls.append(14)
                current["direction"] = words(a0, 3)
                current["motion"] = (a1, a2, a3)
                if real_movement:
                    return
                if mutations & 1024:
                    xor_word(ACTOR + 0x0C, 0x10101010)
                if mutations & 2048:
                    write8(MODE, read8(MODE) ^ 1)
            elif address == 0x8002E858 and real_movement:
                if real_movement_target:
                    return
                calls.append(19)
                uc.mem_write(physical(a3), struct.pack("<3I", a2, a1, read32(a0)))
            elif address == 0x80053D00 and real_movement_target:
                calls.append(20)
                uc.mem_write(physical(a2), struct.pack("<3I", *words(a0, 3)))
            elif address == 0x80028FC8:
                calls.append(16)
            elif address == 0x8001C8A0:
                self.assertEqual((a0, a1), (0, 0))
                calls.append(17)
            elif address == 0x8001C6BC:
                self.assertEqual((a0, a1, a2), (4, 14 if read8(MODE) else 15, 0xFFFFFFFF))
                calls.append(18)
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, result)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            current.update(input=entry, calls=[], adjusted_y=0, direction=(0, 0, 0), motion=(0, 0, 0))
            cpu.mem_write(physical(ACTOR), bytes(0x30))
            cpu.mem_write(physical(INITIAL_VECTOR), bytes(16))
            cpu.mem_write(physical(SHARED), bytes([0xA5]) * 0xA8)
            cpu.mem_write(physical(POSITION), struct.pack("<4I", *entry[:4]))
            cpu.mem_write(physical(SAVED), bytes([0xA5]) * 12)
            write32(ACTOR + 0x10, entry[10])
            write8(LOCK, entry[11])
            write8(MODE, entry[12])
            write8(ACTOR, entry[13])
            write32(ACTOR + 4, entry[14])
            write32(ACTOR + 0x0C, entry[15])
            write32(ACTOR + 0x1C, COMPONENT)
            write32(ACTOR + 0x28, SHARED)
            write32(COMPONENT + 0x14, TRANSFORM)
            write32(POSITION_COMPONENT_GLOBAL, POSITION_COMPONENT)
            write16(LEVEL, entry[16])
            write32(OFFSET, entry[17])
            write32(SCALE, entry[18])
            write32(BASE, entry[19])
            write16(EFFECT, 0xA5A5)
            cpu.mem_write(physical(EVENT_PARAMETERS), bytes([0xA5]) * 32)
            write32(EVENT_PARAMETERS + 4, 0x12345678)
            if real_movement:
                for address, value in ((MOVEMENT_ENABLED, 1), (MOVEMENT_PERMIT, 1),
                                       (MOVEMENT_MODE_COPY, 9), (MOVEMENT_ACTIVE, 0), (MOVEMENT_MODE, 0)):
                    write8(address, value)
                for address, value in ((MOVEMENT_DURATION, 0x11111111),
                                       (MOVEMENT_ELAPSED, 0x22222222), (MOVEMENT_CALLBACK, 0)):
                    write32(address, value)
                for i in range(3):
                    write32(MOVEMENT_STEP + 4 * i, 0x33333333 + i)
                    write32(MOVEMENT_ADJUSTMENT + 4 * i, 0x44444444 + i)
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_A0, POSITION)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.emu_start(0x80038430, RETURN, count=3000 if real_movement else 1000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN, f"case {index} did not return")
            calls = current["calls"]
            original = (*words(POSITION, 4), read8(ACTOR), read32(ACTOR + 4), *words(SAVED, 3),
                        read8(LOCK), read16(EFFECT), *words(SHARED + 0x68, 3),
                        *words(SHARED + 0x38, 4), *words(SHARED + 0x78, 4), *words(SHARED + 0x98, 4),
                        *current["direction"], *current["motion"], current["adjusted_y"], len(calls),
                        *calls, *([0] * (18 - len(calls))), read32(ACTOR + 0x10), read32(EVENT_PARAMETERS + 4))
            if real_movement:
                original += (*words(MOVEMENT_STEP, 3), *words(MOVEMENT_ADJUSTMENT, 3),
                             read32(MOVEMENT_DURATION), read8(MOVEMENT_ACTIVE), read8(MOVEMENT_MODE),
                             int(read32(MOVEMENT_CALLBACK) == 0x80038368))
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
