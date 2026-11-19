import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical

if HAS_UNICORN:
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_V0


INPUT = struct.Struct("<2I64H")
OUTPUT = struct.Struct("<I64H")
BUFFER = 0x801E0000


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalMatrixTransposeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_matrix_transpose_against_original(self):
        cases = [
            (source, destination, *range(64))
            for source, destination in itertools.product(range(56), repeat=2)
        ]
        generator = random.Random(0x77C00)
        cases.extend(
            (generator.randrange(56), generator.randrange(56),
             *(generator.getrandbits(16) for _ in range(64)))
            for _ in range(1000)
        )
        native_outputs = run_native_probe(
            self, ["src/game/func_80077C00.c", "tests/test_matrix_transpose.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            source, destination, *initial = entry
            cpu.mem_write(physical(BUFFER), struct.pack("<64H", *initial))
            cpu.reg_write(UC_MIPS_REG_A0, BUFFER + source * 2)
            cpu.reg_write(UC_MIPS_REG_A1, BUFFER + destination * 2)
            self.call(cpu, 0x80077C00)
            result = (cpu.reg_read(UC_MIPS_REG_V0) - BUFFER) // 2
            original = (result, *struct.unpack("<64H", cpu.mem_read(physical(BUFFER), 128)))
            with self.subTest(index=index, source=source, destination=destination):
                self.assertEqual(native, original)
