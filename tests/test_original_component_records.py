import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical, write_word

if HAS_UNICORN:
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_V0


INPUT = struct.Struct("<35I")
OUTPUT = struct.Struct("<33I")
OWNER = 0x801E0000
TABLE = 0x801E0100
RECORDS = 0x801E1000


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalComponentRecordTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_record_copies_against_original(self):
        words = tuple(range(32))
        cases = [
            (operation, index, offset, *words)
            for operation, index, offset in itertools.product(range(2), range(4), range(25))
        ]
        generator = random.Random(0x52094528A4)
        for _ in range(500):
            cases.append((
                generator.randrange(2), generator.randrange(4), generator.randrange(25),
                *(generator.getrandbits(32) for _ in range(32)),
            ))
        native_outputs = run_native_probe(self, [
            "src/game/func_80052094.c", "src/game/func_800528A4.c",
            "tests/test_component_records.c",
        ], cases, INPUT, OUTPUT)
        for case_index, (entry, native) in enumerate(zip(cases, native_outputs)):
            operation, index, offset, *initial = entry
            with self.subTest(index=case_index, input=entry[:3]):
                cpu = self.fresh_cpu()
                write_word(cpu, OWNER + 0x18, TABLE)
                write_word(cpu, TABLE, RECORDS)
                cpu.mem_write(physical(RECORDS), struct.pack("<32I", *initial))
                cpu.reg_write(UC_MIPS_REG_A0, OWNER)
                cpu.reg_write(UC_MIPS_REG_A1, index)
                cpu.reg_write(UC_MIPS_REG_A2, RECORDS + offset * 4)
                self.call(cpu, 0x80052094 if operation == 0 else 0x800528A4)
                original = (
                    cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                    *struct.unpack("<32I", cpu.mem_read(physical(RECORDS), 128)),
                )
                self.assertEqual(native, original)
