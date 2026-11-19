import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical

if HAS_UNICORN:
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_V0


BUFFER_SIZE = 192
INPUT = struct.Struct(f"<4I{BUFFER_SIZE}s")
OUTPUT = struct.Struct(f"<I{BUFFER_SIZE}s")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalPackedChannelCopyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_expanders_against_original(self):
        generator = random.Random(0x10244)
        cases = []
        for kind in (0, 1):
            for count in (1, 2, 3, 5, 8):
                for source in (0, 2, 4, 12, 16, 31, 32, 63, 64, 100):
                    if kind == 0 and source % 2:
                        continue
                    for destination in (0, 4, 8, 16, 32, 64, 96, 128):
                        data = bytearray(generator.getrandbits(8) for _ in range(BUFFER_SIZE))
                        data[source:source + 4] = bytes((0x80, 0x7F, 0xFF, 0x80))
                        cases.append((kind, count, source, destination, bytes(data)))
            for _ in range(1200):
                count = generator.randrange(1, 9)
                source = generator.randrange(BUFFER_SIZE - count * 4 + 1)
                if kind == 0:
                    source &= ~1
                destination = 4 * generator.randrange((BUFFER_SIZE - count * 8) // 4 + 1)
                data = bytes(generator.getrandbits(8) for _ in range(BUFFER_SIZE))
                cases.append((kind, count, source, destination, data))

        native = run_native_probe(
            self,
            [
                "src/game/func_80010244.c", "src/game/func_80010278.c",
                "tests/test_packed_channel_copy.c",
            ],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        buffer = 0x801E2000
        covered = set()
        for index, (case, actual) in enumerate(zip(cases, native)):
            kind, count, source, destination, data = case
            cpu.mem_write(physical(buffer), data)
            cpu.reg_write(UC_MIPS_REG_A0, buffer + destination)
            cpu.reg_write(UC_MIPS_REG_A1, buffer + source)
            cpu.reg_write(UC_MIPS_REG_A2, count)
            self.call(cpu, (0x80010244, 0x80010278)[kind])
            original = (cpu.reg_read(UC_MIPS_REG_V0), bytes(cpu.mem_read(physical(buffer), BUFFER_SIZE)))
            with self.subTest(case=index, kind=kind, count=count, source=source, destination=destination):
                self.assertEqual(actual, original)

            if source < destination + count * 8 and destination < source + count * 4:
                covered.add((kind, "overlap"))
            else:
                covered.add((kind, "separate"))
            if kind == 1 and source % 2:
                covered.add((kind, "unaligned byte input"))
            if count == 8:
                covered.add((kind, "eight records"))
        self.assertEqual(covered, {
            (0, "overlap"), (0, "separate"), (0, "eight records"),
            (1, "overlap"), (1, "separate"), (1, "unaligned byte input"),
            (1, "eight records"),
        })


if __name__ == "__main__":
    unittest.main()
