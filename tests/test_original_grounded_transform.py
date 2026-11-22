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
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<17I")
OUTPUT = struct.Struct("<17I")
DIRECTION = 0x801E0000
OUTPUT_ADDRESS = 0x801E0100
COMPONENT_A = 0x801E0200
COMPONENT_B = 0x801E0300
COMPONENT_C = 0x801E0400
COMPONENT_GLOBAL = 0x800BE528
GATE = 0x800BE024


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalGroundedTransformTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_transform_helpers_against_original(self):
        rng = random.Random(0x2E858)
        cases = []
        for mode, alias, mutation in itertools.product(range(3), range(2), range(2)):
            gates = (0, 1, 2, 255, 256) if mode == 2 else (1,)
            for gate in gates:
                for _ in range(160):
                    cases.append((mode, gate, alias, mutation, *(rng.getrandbits(32) for _ in range(13))))
        native_results = run_native_probe(
            self, ["src/game/grounded_transform.c", "tests/test_grounded_transform.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_words(uc, address, count):
            return struct.unpack("<%dI" % count, uc.mem_read(physical(address), 4 * count))

        def write_words(uc, address, values):
            uc.mem_write(physical(address), struct.pack("<%dI" % len(values), *values))

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            trace = current["trace"]
            if address == 0x80054B20:
                self.assertEqual((a0, a1), (COMPONENT_A, 0))
                trace.append(1)
                write_words(uc, a2, current["translation"])
                if current["mutation"]:
                    value = read_words(uc, DIRECTION + 4, 1)[0] ^ 0x12345678
                    write_words(uc, DIRECTION + 4, (value,))
            elif address == 0x80053D00:
                values = read_words(uc, a0, 3)
                trace.extend((2, a1, *values))
                write_words(uc, a2, (values[0] ^ a1, (values[1] + 0x23456789) & 0xFFFFFFFF,
                                     values[2] ^ 0xABCDEF01))
            elif address == 0x80054C7C:
                self.assertEqual((a0, a1), (COMPONENT_A, 0))
                trace.extend((3, *read_words(uc, a2, 3)))
                if current["mutation"]:
                    value = read_words(uc, DIRECTION + 4, 1)[0] ^ 0x87654321
                    write_words(uc, DIRECTION + 4, (value,))
            elif address == 0x800516F8:
                trace.append(4)
                write_words(uc, a0, (COMPONENT_B,))
                if current["mutation"]:
                    write_words(uc, COMPONENT_GLOBAL, (COMPONENT_C,))
            elif address == 0x800550A4:
                trace.extend((5, int(a0 == COMPONENT_B), 3 if a1 == COMPONENT_C else 1 if a1 == COMPONENT_A else 0))
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFB)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            mode, gate, alias, mutation = entry[:4]
            direction, output, translation = entry[4:8], entry[8:12], entry[12:15]
            distance, parameter = entry[15:17]
            current.update(mutation=mutation, translation=translation, trace=[])
            write_words(cpu, DIRECTION, direction)
            write_words(cpu, OUTPUT_ADDRESS, output)
            write_words(cpu, COMPONENT_GLOBAL, (COMPONENT_A,))
            cpu.mem_write(physical(GATE), bytes((gate & 0xFF,)))
            cpu.reg_write(UC_MIPS_REG_A0, DIRECTION)
            if mode == 0:
                cpu.reg_write(UC_MIPS_REG_A1, distance)
                cpu.reg_write(UC_MIPS_REG_A2, parameter)
                cpu.reg_write(UC_MIPS_REG_A3, DIRECTION if alias else OUTPUT_ADDRESS)
                address = 0x8002E858
            elif mode == 1:
                cpu.reg_write(UC_MIPS_REG_A1, parameter)
                address = 0x8002E954
            else:
                address = 0x8002EA80
            self.call(cpu, address)
            trace = current["trace"]
            self.assertLessEqual(len(trace), 8)
            original = (len(trace), *trace, *([0] * (8 - len(trace))),
                        *read_words(cpu, DIRECTION, 4), *read_words(cpu, OUTPUT_ADDRESS, 4))
            self.assertEqual(native, original, f"case {index}: {entry}")

    def test_transform_with_real_scaling_and_division(self):
        rng = random.Random(0x2E85853D00)
        cases = []
        for alias, mutation in itertools.product(range(2), range(2)):
            for index in range(600):
                values = [rng.getrandbits(32) for _ in range(13)]
                if index % 20 == 0:
                    values[9] = 0
                cases.append((0, 1, alias, mutation, *values))
        native_results = run_native_probe(
            self, [
                "src/game/grounded_transform.c", "src/game/func_80053D00.c",
                "src/game/func_80053B70.c", "src/game/func_80053350.c",
                "tests/test_grounded_transform.c",
            ], cases, INPUT, OUTPUT, compiler_flags=("-DGROUNDED_TRANSFORM_REAL_SCALE",),
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_words(uc, address, count):
            return struct.unpack("<%dI" % count, uc.mem_read(physical(address), 4 * count))

        def write_words(uc, address, values):
            uc.mem_write(physical(address), struct.pack("<%dI" % len(values), *values))

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            trace = current["trace"]
            if address == 0x80054B20:
                self.assertEqual((a0, a1), (COMPONENT_A, 0))
                trace.append(1)
                write_words(uc, a2, current["translation"])
                if current["mutation"]:
                    value = read_words(uc, DIRECTION + 4, 1)[0] ^ 0x12345678
                    write_words(uc, DIRECTION + 4, (value,))
            elif address == 0x8005385C:
                trace.extend((2, *read_words(uc, a0, 3)))
                write_words(uc, a1, (current["translation"][1],))
            elif address == 0x80053A08:
                values = read_words(uc, a0, 3)
                trace.extend((6, a1))
                write_words(uc, a2, (values[0] ^ a1, (values[1] + 0x23456789) & 0xFFFFFFFF,
                                     values[2] ^ 0xABCDEF01))
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFE)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            _, _, alias, mutation = entry[:4]
            direction, output, translation = entry[4:8], entry[8:12], entry[12:15]
            distance, height = entry[15:17]
            current.update(mutation=mutation, translation=translation, trace=[])
            write_words(cpu, DIRECTION, direction)
            write_words(cpu, OUTPUT_ADDRESS, output)
            write_words(cpu, COMPONENT_GLOBAL, (COMPONENT_A,))
            cpu.reg_write(UC_MIPS_REG_A0, DIRECTION)
            cpu.reg_write(UC_MIPS_REG_A1, distance)
            cpu.reg_write(UC_MIPS_REG_A2, height)
            cpu.reg_write(UC_MIPS_REG_A3, DIRECTION if alias else OUTPUT_ADDRESS)
            self.call(cpu, 0x8002E858)
            trace = current["trace"]
            self.assertLessEqual(len(trace), 8)
            original = (len(trace), *trace, *([0] * (8 - len(trace))),
                        *read_words(cpu, DIRECTION, 4), *read_words(cpu, OUTPUT_ADDRESS, 4))
            self.assertEqual(native, original, f"case {index}: {entry}")

    def test_transform_with_real_vector_arithmetic(self):
        self.compare_vector_arithmetic(real_sqrt=False)

    def test_transform_with_readable_sqrt_backend(self):
        self.compare_vector_arithmetic(real_sqrt=True)

    def compare_vector_arithmetic(self, real_sqrt):
        rng = random.Random(0x5385C53A08)
        cases = []
        if real_sqrt:
            offset = controller_route.SQRT_LOOKUP_DOMAIN - self.load_address
            original_domain = struct.unpack_from("<512H", self.payload, offset)
            synthetic_domain = [(index * 271 + 0x1234) & 0xFFFF for index in range(512)]
            synthetic_domain[320] = 4096
        for alias, mutation in itertools.product(range(2), range(2)):
            for index in range(600):
                values = [rng.getrandbits(32) for _ in range(13)]
                if index % 20 == 0:
                    values[9] = 0
                if index % 30 == 0:
                    values[:3] = [0, 0, 0]
                entry = (0, 1, alias, mutation, *values)
                if real_sqrt:
                    entry += tuple(original_domain if index % 2 == 0 else synthetic_domain)
                cases.append(entry)
        sources = [
            "src/game/grounded_transform.c", "src/game/func_80053D00.c",
            "src/game/func_80053B70.c", "src/game/func_80053350.c",
            "src/game/func_80053A08.c", "src/game/func_8001007C.c",
            "src/game/sqrt_backend_wrappers.c",
            "src/player_model/player_vector_transform.c", "tests/test_grounded_transform.c",
        ]
        flags = ["-DGROUNDED_TRANSFORM_REAL_ARITHMETIC", "-DBUBSY3D_MATCH_FUNC_8005385C", "-fwrapv"]
        if real_sqrt:
            sources.append("src/game/gte_sqrt_backends.c")
            flags.append("-DGROUNDED_TRANSFORM_REAL_SQRT")
        input_layout = struct.Struct("<17I512H") if real_sqrt else INPUT
        native_results = run_native_probe(self, sources, cases, input_layout, OUTPUT, compiler_flags=flags)
        cpu = self.fresh_cpu()
        current = {}
        gte = controller_route.RotationGteModel()
        if real_sqrt:
            cpu.hook_add(UC_HOOK_CODE, gte.hook)

        def read_words(uc, address, count):
            return struct.unpack("<%dI" % count, uc.mem_read(physical(address), 4 * count))

        def write_words(uc, address, values):
            uc.mem_write(physical(address), struct.pack("<%dI" % len(values), *values))

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            if address == 0x80054B20:
                self.assertEqual((a0, a1), (COMPONENT_A, 0))
                current["trace"].append(1)
                write_words(uc, a2, current["translation"])
                if current["mutation"]:
                    value = read_words(uc, DIRECTION + 4, 1)[0] ^ 0x12345678
                    write_words(uc, DIRECTION + 4, (value,))
            elif address == 0x80076554:
                if real_sqrt:
                    return
                current["trace"].extend((2, a0))
                uc.reg_write(UC_MIPS_REG_V0, current["translation"][1])
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                return
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFE)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            _, _, alias, mutation = entry[:4]
            direction, output, translation = entry[4:8], entry[8:12], entry[12:15]
            distance, height = entry[15:17]
            if real_sqrt:
                cpu.mem_write(physical(controller_route.SQRT_LOOKUP_DOMAIN), struct.pack("<512H", *entry[17:]))
            current.update(mutation=mutation, translation=translation, trace=[])
            write_words(cpu, DIRECTION, direction)
            write_words(cpu, OUTPUT_ADDRESS, output)
            write_words(cpu, COMPONENT_GLOBAL, (COMPONENT_A,))
            cpu.reg_write(UC_MIPS_REG_A0, DIRECTION)
            cpu.reg_write(UC_MIPS_REG_A1, distance)
            cpu.reg_write(UC_MIPS_REG_A2, height)
            cpu.reg_write(UC_MIPS_REG_A3, DIRECTION if alias else OUTPUT_ADDRESS)
            self.call(cpu, 0x8002E858)
            trace = current["trace"]
            self.assertLessEqual(len(trace), 8)
            original = (len(trace), *trace, *([0] * (8 - len(trace))),
                        *read_words(cpu, DIRECTION, 4), *read_words(cpu, OUTPUT_ADDRESS, 4))
            self.assertEqual(native, original, f"case {index}: {entry}")
        if real_sqrt:
            self.assertEqual(gte.leading_counts, len(cases))


if __name__ == "__main__":
    unittest.main()
