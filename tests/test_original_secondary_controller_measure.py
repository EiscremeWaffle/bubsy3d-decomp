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
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_FP, UC_MIPS_REG_PC,
        UC_MIPS_REG_RA, UC_MIPS_REG_S3, UC_MIPS_REG_S7, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<16I")
OUTPUT = struct.Struct("<38I")
COMPONENT = 0x801E4000
CONTEXTS = (0x801E4100, 0x801E4200)
BASIS = 0x8018B978
FLAG = GP + 0x506


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerMeasureTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_measurement_against_original(self):
        self.compare_measurement(real_arithmetic=False)

    def test_measurement_with_real_distance_and_multiplication(self):
        self.compare_measurement(real_arithmetic=True)

    def compare_measurement(self, real_arithmetic):
        rng = random.Random(0x49B08)
        cases = []
        if real_arithmetic:
            for flag, toggle, mode, mutations in itertools.product(
                (0, 1, 2), (0, 1), (0, 2), (0, 1, 8, 16, 24, 32, 64, 96, 120)
            ):
                values = [rng.randint(-4096, 4096) & 0xFFFFFFFF for _ in range(9)]
                cases.append((*values, rng.randrange(8193), rng.getrandbits(32), rng.getrandbits(32),
                              flag, toggle, mode, mutations))
            for _ in range(800):
                values = [rng.randint(-4096, 4096) & 0xFFFFFFFF for _ in range(9)]
                cases.append((*values, rng.randrange(8193), rng.getrandbits(32), rng.getrandbits(32),
                              rng.randrange(3), rng.randrange(2), rng.choice((0, 2)),
                              rng.choice((0, 1, 8, 16, 24, 32, 64, 96, 120))))
        else:
            for flag, toggle, mode, mutations in itertools.product((0, 1, 2), (0, 1), (0, 2), range(128)):
                cases.append((*(rng.getrandbits(32) for _ in range(12)), flag, toggle, mode, mutations))
            for value in (-0x80000000, -5, -4, -1, 0, 1, 3, 4, 7, 8, 0x7FFFFFFF):
                for delta in (-1, 0, 1):
                    cases.append((*([0] * 9), 0, (value // 4 + delta) & 0xFFFFFFFF,
                                  value & 0xFFFFFFFF, 1, 0, 0, 0))
            cases.extend((*(rng.getrandbits(32) for _ in range(12)),
                          rng.randrange(3), rng.randrange(2), rng.choice((0, 2)), rng.randrange(128))
                         for _ in range(2000))
        sources = ["src/game/secondary_controller_measure.c", "tests/test_secondary_controller_measure.c"]
        flags = []
        if real_arithmetic:
            sources.extend(["src/game/func_80062D4C.c", "src/game/func_80053904.c",
                            "src/game/func_8001007C.c"])
            flags.append("-DSECONDARY_MEASURE_REAL_ARITHMETIC")
        native_results = run_native_probe(
            self, sources, cases, INPUT, OUTPUT, compiler_flags=flags,
        )
        cpu = self.fresh_cpu()
        current = {}

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def write_words(address, values):
            cpu.mem_write(physical(address), struct.pack("<%dI" % len(values), *values))

        def read_words(address, count):
            return struct.unpack("<%dI" % count, cpu.mem_read(physical(address), 4 * count))

        def xor_word(address, mask):
            write32(address, read32(address) ^ mask)

        def intercept(uc, address, size, data):
            if address in (0x80049CB8, 0x8004A008):
                current["branch"] = int(address == 0x80049CB8)
                uc.emu_stop()
                return
            a0, a1, a2 = (uc.reg_read(reg) & 0xFFFFFFFF for reg in
                          (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2))
            stack = current["stack"]
            if address == 0x8002CB2C:
                self.assertEqual(a0, 5)
                current["calls"].append(1)
            elif address == 0x8003AA1C:
                self.assertEqual(a0, stack + 0xD0)
                current["calls"].append(2)
                write32(a0, CONTEXTS[0])
            elif address == 0x80054B20:
                translation = current["translations"]
                is_actor = translation == 0 or translation == 3
                self.assertEqual((a0, a1, a2), (
                    COMPONENT if is_actor else BASIS, 0,
                    stack + (0xA0 if is_actor else 0x40),
                ))
                current["calls"].append(3 + translation if translation < 2 else 11 + translation - 2)
                write_words(a2, current["target"] if is_actor else current["position"])
                if translation == 0 and current["mutations"] & 1:
                    flag = uc.mem_read(physical(FLAG), 1)[0]
                    uc.mem_write(physical(FLAG), bytes((flag ^ 1,)))
                current["translations"] += 1
            elif address == 0x80054AF0:
                self.assertEqual((a0, a1), (BASIS, stack + 0x30))
                current["calls"].append(5)
                write_words(a1, current["direction"])
            elif address == 0x80062D4C:
                if real_arithmetic:
                    return
                distance = current["distances"]
                self.assertEqual((a0, a1, a2), (stack + 0x40, stack + 0xA0,
                                               stack + (0xD8 if distance == 1 else 0xD4)))
                current["calls"].append(6 + distance * 4 if distance < 2 else 13)
                write32(a2, current["distance_values"][distance])
                if distance == 0 and current["mutations"] & 2:
                    xor_word(stack + 0x44, 0x01010101)
                    xor_word(stack + 0xA0, 0x02020202)
                    xor_word(stack + 0x38, 0x04040404)
                if distance == 1:
                    if current["mutations"] & 8:
                        uc.mem_write(physical(FLAG), b"\0")
                        uc.mem_write(physical(CONTEXTS[0] + 0x13), b"\0")
                        uc.mem_write(physical(CONTEXTS[0] + 0x11), b"\2")
                    if current["mutations"] & 16:
                        write32(stack + 0xD0, CONTEXTS[1])
                current["distances"] += 1
            elif address == 0x8001007C:
                if real_arithmetic:
                    return
                multiply = current["multiplies"]
                self.assertEqual(a2, stack + 0xB0 + (multiply % 3) * 4)
                if multiply >= 3:
                    self.assertEqual(a1, 48)
                current["calls"].append(7 + multiply if multiply < 3 else 14 + multiply - 3)
                write32(a2, a0 ^ a1 ^ (0xABC00000 + multiply))
                if multiply == 0 and current["mutations"] & 4:
                    xor_word(stack + 0x34, 0x08080808)
                    xor_word(stack + 0xD4, 0x10101010)
                current["multiplies"] += 1
            elif address == 0x800535CC and real_arithmetic:
                distance = current["distances"]
                self.assertEqual(a1, stack + (0xD8 if distance == 1 else 0xD4))
                current["calls"].extend((6 + distance * 4 if distance < 2 else 13, a0))
                write32(a1, current["distance_values"][distance])
                if distance == 1:
                    if current["mutations"] & 8:
                        uc.mem_write(physical(FLAG), b"\0")
                        uc.mem_write(physical(CONTEXTS[0] + 0x13), b"\0")
                        uc.mem_write(physical(CONTEXTS[0] + 0x11), b"\2")
                    if current["mutations"] & 16:
                        write32(stack + 0xD0, CONTEXTS[1])
                current["distances"] += 1
            elif address == 0x8001BC08:
                current["calls"].append(17)
                if current["mutations"] & 32:
                    xor_word(stack + 0xD8, 0x20202020)
            elif address == 0x8001C6BC:
                self.assertEqual((a0, a1, a2), (5, 2, 1))
                current["calls"].append(18)
                if current["mutations"] & 64:
                    xor_word(stack + 0xD4, 0x40404040)
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        stack = STACK - 0x128
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            target, position, direction = entry[:3], entry[3:6], entry[6:9]
            distance_values, flag, toggle, mode, mutations = entry[9:12], *entry[12:16]
            current.update(stack=stack, target=target, position=position, direction=direction,
                           distance_values=distance_values, mutations=mutations, calls=[],
                           translations=0, distances=0, multiplies=0, branch=None)
            cpu.mem_write(physical(stack), bytes(0x128))
            for context in CONTEXTS:
                cpu.mem_write(physical(context), bytes(0x80))
            cpu.mem_write(physical(CONTEXTS[0] + 0x13), bytes((toggle,)))
            cpu.mem_write(physical(CONTEXTS[0] + 0x11), bytes((mode,)))
            cpu.mem_write(physical(FLAG), bytes((flag,)))
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.reg_write(UC_MIPS_REG_S7, COMPONENT)
            cpu.reg_write(UC_MIPS_REG_S3, 1)
            cpu.reg_write(UC_MIPS_REG_FP, 0)
            cpu.emu_start(0x80049B08, 0, count=2000)
            self.assertIsNotNone(current["branch"], f"case {index} did not reach a boundary")
            original = (
                *read_words(stack + 0xA0, 3), *read_words(stack + 0x40, 3),
                *read_words(stack + 0x30, 3), *read_words(stack + 0xB0, 3),
                read32(stack + 0xD4), read32(stack + 0xD8), uc_fp(cpu), current["branch"],
                cpu.mem_read(physical(FLAG), 1)[0],
                cpu.mem_read(physical(CONTEXTS[0] + 0x13), 1)[0],
                cpu.mem_read(physical(CONTEXTS[0] + 0x11), 1)[0],
                len(current["calls"]), *current["calls"], *([0] * (18 - len(current["calls"]))),
            )
            self.assertEqual(native, original, f"case {index}: {entry}")


def uc_fp(cpu):
    return cpu.reg_read(UC_MIPS_REG_FP) & 0xFFFFFFFF


if __name__ == "__main__":
    unittest.main()
