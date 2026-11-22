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

INPUT = struct.Struct("<13I")
OUTPUT = struct.Struct("<30I")
VECTOR = 0x801E0000
OUTPUT_ADDRESS = 0x801E0100


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalGroundedVectorScaleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_scale_helpers_against_original(self):
        rng = random.Random(0x53D00)
        cases = [
            (mode, alias, mutation, *(rng.getrandbits(32) for _ in range(10)))
            for mode, alias, mutation in itertools.product(range(2), range(2), (0, 1, 2, 4, 7))
            for _ in range(120)
        ]
        native_results = run_native_probe(
            self, ["src/game/func_80053B70.c", "src/game/func_80053D00.c",
                   "tests/test_grounded_vector_scale.c"],
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
            if address == 0x8005385C:
                trace.extend((1, *read_words(uc, a0, 3)))
                write_words(uc, a1, (current["length"],))
                current["length_pointer"] = a1
                if current["mutation"] & 1:
                    value = read_words(uc, VECTOR + 4, 1)[0] ^ 0x12345678
                    write_words(uc, VECTOR + 4, (value,))
            elif address == 0x80053A08:
                values = read_words(uc, a0, 3)
                trace.extend((2, a1, *values))
                write_words(uc, a2, tuple(value ^ a1 for value in values))
                if current["mutation"] & 2:
                    pointer = current["length_pointer"]
                    value = read_words(uc, pointer, 1)[0] ^ 0x87654321
                    write_words(uc, pointer, (value,))
            elif address == 0x80053350:
                trace.extend((3, a0, a1))
                write_words(uc, a2, ((a0 + a1) & 0xFFFFFFFF,))
                if current["mode"] == 0 and current["mutation"] & 4 and current["divide_calls"] == 0:
                    value = read_words(uc, VECTOR + 4, 1)[0] ^ 0xABCDEF01
                    write_words(uc, VECTOR + 4, (value,))
                current["divide_calls"] += 1
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFD)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            mode, alias, mutation = entry[:3]
            vector, output, scale, length = entry[3:7], entry[7:11], entry[11], entry[12]
            current.update(mode=mode, mutation=mutation, length=length, trace=[], divide_calls=0)
            write_words(cpu, VECTOR, vector)
            write_words(cpu, OUTPUT_ADDRESS, output)
            cpu.reg_write(UC_MIPS_REG_A0, VECTOR)
            cpu.reg_write(UC_MIPS_REG_A1, scale)
            cpu.reg_write(UC_MIPS_REG_A2, VECTOR if alias else OUTPUT_ADDRESS)
            self.call(cpu, 0x80053B70 if mode == 0 else 0x80053D00)
            trace = current["trace"]
            self.assertLessEqual(len(trace), 20)
            original = (cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, len(trace),
                        *trace, *([0] * (20 - len(trace))),
                        *read_words(cpu, VECTOR, 4), *read_words(cpu, OUTPUT_ADDRESS, 4))
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
