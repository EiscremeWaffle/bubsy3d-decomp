import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_PC,
        UC_MIPS_REG_RA, UC_MIPS_REG_S0, UC_MIPS_REG_S2, UC_MIPS_REG_SP,
    )

INPUT = struct.Struct("<13I")
OUTPUT = struct.Struct("<14I")
ANCHOR = 0x8018B978
COMPONENT = 0x801E4200


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerApproachStepTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_step_against_original(self):
        rng = random.Random(0x49DD0)
        cases = []
        for iteration, height, delta, mutations in itertools.product(
            (0, 18, 19, 20, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF),
            (0, 159, 160, 161, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF),
            (0, 1, 0x80000000, 0xFFFFFFFF),
            range(16),
        ):
            words = [rng.getrandbits(32) for _ in range(11)]
            words[1] = height
            words[9] = delta
            cases.append((*words, iteration, mutations))
        cases.extend((*(rng.getrandbits(32) for _ in range(12)), rng.randrange(16))
                     for _ in range(1500))
        native_results = run_native_probe(
            self, ["src/game/secondary_controller_approach.c", "tests/test_secondary_controller_approach.c"],
            cases, INPUT, OUTPUT, compiler_flags=["-DSECONDARY_APPROACH_STEP_PROBE"],
        )
        cpu = self.fresh_cpu()
        stack = STACK - 0x128
        current = {}

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def words(address, count):
            return struct.unpack("<%dI" % count, cpu.mem_read(physical(address), 4 * count))

        def xor_word(address, mask):
            write32(address, read32(address) ^ mask)

        def intercept(uc, address, size, data):
            if address == 0x80049E78:
                current["reached"] = True
                uc.emu_stop()
                return
            if address != 0x80054C7C:
                return
            a0, a1, a2 = (uc.reg_read(reg) & 0xFFFFFFFF
                          for reg in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2))
            self.assertEqual(a1, 0)
            if a0 == ANCHOR:
                self.assertEqual(a2, stack + 0x40)
                current["calls"].append(1)
                if current["mutations"] & 1:
                    xor_word(stack + 0xB4, 0x80000000)
                if current["mutations"] & 2:
                    xor_word(stack + 0xC0, 0x10101010)
                if current["mutations"] & 4:
                    xor_word(stack + 0x44, 0x04040404)
            else:
                self.assertEqual((a0, a2), (COMPONENT, stack + 0xC0))
                current["calls"].append(2)
                if current["mutations"] & 8:
                    xor_word(stack + 0xC8, 0x80808080)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            current.update(mutations=entry[12], calls=[], reached=False)
            cpu.mem_write(physical(stack), bytes(0x128))
            cpu.mem_write(physical(stack + 0x40), struct.pack("<4I", *entry[:4]))
            cpu.mem_write(physical(stack + 0xC0), struct.pack("<4I", *entry[4:8]))
            cpu.mem_write(physical(stack + 0xB0), struct.pack("<3I", *entry[8:11]))
            write32(stack + 0xF4, entry[11])
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.reg_write(UC_MIPS_REG_S0, stack + 0xC0)
            cpu.reg_write(UC_MIPS_REG_S2, COMPONENT)
            cpu.emu_start(0x80049DD0, 0, count=200)
            self.assertTrue(current["reached"], f"case {index} did not reach the boundary")
            original = (*words(stack + 0x40, 4), *words(stack + 0xC0, 4), *words(stack + 0xB0, 3),
                        len(current["calls"]), *current["calls"], *([0] * (2 - len(current["calls"]))))
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
