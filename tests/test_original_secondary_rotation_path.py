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


INPUT = struct.Struct("<22I")
OUTPUT = struct.Struct("<114I")
COMPONENT = 0x801E0000
OTHER = 0x801E0100
ROTATION = 0x801E0200


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryRotationPathTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_rotation_path_against_original(self):
        generator = random.Random(0x48E3048FA8)
        cases = [
            (mode, alias, mutation, *(generator.getrandbits(32) for _ in range(19)))
            for mode, alias, mutation in itertools.product(range(5), (0, 1), (0, 1))
            for _ in range(48)
        ]
        native = run_native_probe(
            self, [
                "src/game/func_80048E30.c", "src/game/func_80048FA8.c",
                "src/game/func_80049080.c", "src/game/func_80054488.c",
                "tests/test_secondary_rotation_path.c",
            ], cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_words(uc, address, count):
            return struct.unpack("<%dI" % count, uc.mem_read(physical(address), count * 4))

        def write_words(uc, address, words):
            uc.mem_write(physical(address), struct.pack("<%dI" % len(words), *words))

        def mutate_component(uc):
            data = bytearray(uc.mem_read(physical(COMPONENT), 32))
            struct.pack_into("<H", data, 0, struct.unpack_from("<H", data, 0)[0] ^ 0x1234)
            struct.pack_into("<H", data, 18, struct.unpack_from("<H", data, 18)[0] ^ 0xFFFF)
            struct.pack_into("<I", data, 20, struct.unpack_from("<I", data, 20)[0] ^ 0x87654321)
            uc.mem_write(physical(COMPONENT), bytes(data))

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            trace = current["trace"]
            status = 0
            if address == 0x80077C50:
                angles = struct.unpack("<3H", uc.mem_read(physical(a0), 6))
                trace.extend((1, *angles))
                values = [(angles[i % 3] ^ (i * 977)) & 0xFFFF for i in range(9)]
                uc.mem_write(physical(a1), struct.pack("<10H3i", *values, 0xABCD, 123, -456, 789))
                if current["mutation"]:
                    mutate_component(uc)
                status = a1
            elif address in (0x80076890, 0x8007699C):
                basis, rotation = (a1, a0) if address == 0x80076890 else (a0, a1)
                trace.extend((2 if address == 0x80076890 else 3, *read_words(uc, basis, 8)))
                value = struct.unpack("<H", uc.mem_read(physical(rotation), 2))[0] ^ 0x9234
                uc.mem_write(physical(rotation), struct.pack("<H", value))
                status = rotation
            elif address == 0x80053D68:
                values = read_words(uc, a0, 3)
                trace.extend((4, int(a0 == a1), *values))
                write_words(uc, a1, tuple(value ^ 0x13579BDF for value in values))
                if current["mode"] == 0 and current["mutation"] and current["condition_calls"] == 0:
                    mutate_component(uc)
                current["condition_calls"] += 1
                status = 0xFFFFFFFF
            elif address == 0x80010128:
                left, right = read_words(uc, a0, 3), read_words(uc, a1, 3)
                trace.extend((5, *left, *right))
                write_words(uc, a2, tuple(left[i] ^ ((right[i] + i + 1) & 0xFFFFFFFF) for i in range(3)))
                status = 0xFFFFFFFE
            elif address == 0x8005395C:
                values = read_words(uc, a0, 3)
                trace.extend((6, *values))
                write_words(uc, a1, tuple(value ^ 0x2468ACE0 for value in values))
                status = 0xFFFFFFFD
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, status)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native_result) in enumerate(zip(cases, native)):
            mode, alias, mutation = entry[:3]
            component, other, rotation = entry[3:11], entry[11:19], entry[19:22]
            current.update(mode=mode, mutation=mutation, trace=[], condition_calls=0)
            write_words(cpu, COMPONENT, component)
            write_words(cpu, OTHER, other)
            write_words(cpu, ROTATION, rotation)
            if mode == 0:
                address, a0, a1 = 0x80048E30, COMPONENT, COMPONENT if alias else OTHER
            elif mode < 3:
                address, a0 = (0x80048FA8 if mode == 1 else 0x80049080), COMPONENT
                a1 = COMPONENT + 20 if alias else ROTATION
            else:
                address, a0, a1 = (0x80048FA8 if mode == 3 else 0x80049080), 0, 0
            cpu.reg_write(UC_MIPS_REG_A0, a0)
            cpu.reg_write(UC_MIPS_REG_A1, a1)
            self.call(cpu, address)
            trace = current["trace"]
            self.assertLessEqual(len(trace), 96)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, len(trace),
                *read_words(cpu, COMPONENT, 8), *read_words(cpu, OTHER, 8),
                *trace, *([0] * (96 - len(trace))),
            )
            self.assertEqual(native_result, original, f"case {index}: {entry}")
