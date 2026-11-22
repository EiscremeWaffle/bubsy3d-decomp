import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical

if HAS_UNICORN:
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_V0


INPUT = struct.Struct("<14I")
OUTPUT = struct.Struct("<13I")
STORAGE = 0x801E0000


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalVectorHalfTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_halving_against_original(self):
        generator = random.Random(0x53EA0)
        values = (0, 1, 2, 3, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFD, 0xFFFFFFFF)
        cases = [
            (input_offset, output_offset, *(generator.getrandbits(32) for _ in range(12)))
            for input_offset, output_offset in itertools.product(range(7), repeat=2)
            for _ in range(16)
        ]
        cases.extend((0, 3, *([value] * 12)) for value in values)
        native_outputs = run_native_probe(
            self, ["src/game/func_80053EA0.c", "tests/test_vector_half.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            input_offset, output_offset = entry[:2]
            cpu.mem_write(physical(STORAGE), struct.pack("<12I", *entry[2:]))
            cpu.reg_write(UC_MIPS_REG_A0, STORAGE + 4 * input_offset)
            cpu.reg_write(UC_MIPS_REG_A1, STORAGE + 4 * output_offset)
            self.call(cpu, 0x80053EA0)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                *struct.unpack("<12I", cpu.mem_read(physical(STORAGE), 48)),
            )
            self.assertEqual(native, original, f"case {index}: {entry}")
