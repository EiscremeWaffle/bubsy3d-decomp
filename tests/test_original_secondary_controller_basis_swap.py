import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, RETURN, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_PC,
        UC_MIPS_REG_RA, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<22I")
OUTPUT = struct.Struct("<33I")
REFERENCE = 0x801E0000
BASIS = 0x8018B978
DISPLAY = 0x8018B950
TICK = GP + 0x508
TICK_RESULT = GP + 0x50C
FLAG = GP + 0x507
ARGUMENTS = (0, 0x801E6000, 0x801E6010)
HELPERS = {
    0x800516F8: 1,
    0x80032D7C: 2,
    0x800439EC: 3,
    0x80044268: 4,
    0x8001B0A0: 5,
    0x80050534: 6,
    0x80032C8C: 7,
    0x80051678: 8,
    0x800509A4: 9,
    0x80050650: 10,
}


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerBasisSwapTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_basis_swap_against_original(self):
        generator = random.Random(0x49158)
        masks = (0, 1, 2, 4, 8, 16, 32, 64, 128, 255)
        cases = []
        for tick, tick_result, flag, mutation in itertools.product(
            (0, 1, 2, 0xFFFFFFFE), (0, 1, 0xFF), (0, 1, 2, 0xFF), masks,
        ):
            cases.append((
                tick, tick_result, flag, generator.getrandbits(32), mutation,
                generator.randrange(3),
                *(generator.getrandbits(32) for _ in range(16)),
            ))
        native = run_native_probe(
            self, [
                "src/game/func_80049158.c",
                "tests/test_secondary_controller_basis_swap.c",
            ], cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_words(address, count):
            return struct.unpack("<%dI" % count, cpu.mem_read(physical(address), count * 4))

        def write_words(address, words):
            cpu.mem_write(physical(address), struct.pack("<%dI" % len(words), *words))

        def read_byte(address):
            return cpu.mem_read(physical(address), 1)[0]

        def write_byte(address, value):
            cpu.mem_write(physical(address), bytes((value & 0xFF,)))

        def mutate_reference(mask):
            words = list(read_words(REFERENCE, 8))
            words[0] ^= mask
            write_words(REFERENCE, words)

        def intercept(uc, address, size, data):
            call = HELPERS.get(address)
            if call is None:
                return
            current["calls"].append(call)
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            mutations = current["mutations"]
            status = 0xFFFFFFFF
            if call == 1:
                self.assertTrue(STACK - 0x1000 <= a0 < STACK)
                uc.mem_write(physical(a0), struct.pack("<I", REFERENCE))
            elif call == 2:
                current["queue_calls"] += 1
                if current["queue_calls"] == 1 and mutations & 1:
                    mutate_reference(0x01020304)
                if current["queue_calls"] == 2 and mutations & 64:
                    mutate_reference(0xA5A55A5A)
            elif call == 3:
                if mutations & 2:
                    write_byte(FLAG, 0x22)
                status = current["result"]
            elif call == 5:
                if mutations & 4:
                    write_byte(FLAG, 1)
            elif call == 6:
                self.assertEqual(a0, DISPLAY)
                if mutations & 8:
                    write_byte(FLAG, 0)
            elif call == 7:
                self.assertEqual(a0, 0)
                current["dispatch_calls"] += 1
                if current["dispatch_calls"] == 1 and mutations & 16:
                    write_byte(FLAG, 1)
                if current["dispatch_calls"] == 2 and mutations & 128:
                    write_byte(FLAG, 0x77)
            elif call == 9:
                self.assertEqual((a0, a1, a2), (1, ARGUMENTS[current["argument"]], 1))
                if mutations & 32:
                    write_byte(FLAG, 1)
            elif call == 10:
                self.assertEqual(a0, DISPLAY)
            uc.reg_write(UC_MIPS_REG_V0, status)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native_result) in enumerate(zip(cases, native)):
            tick, tick_result, flag, result, mutations, argument = entry[:6]
            reference = entry[6:14]
            basis = entry[14:22]
            current.update(
                calls=[], tick=tick, tick_result=tick_result, flag=flag, result=result,
                mutations=mutations, argument=argument, queue_calls=0, dispatch_calls=0,
            )
            cpu.mem_write(physical(TICK), struct.pack("<I", tick))
            write_byte(TICK_RESULT, tick_result)
            write_byte(FLAG, flag)
            write_words(REFERENCE, reference)
            write_words(BASIS, basis)
            cpu.reg_write(UC_MIPS_REG_A0, ARGUMENTS[argument])
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.emu_start(0x80049158, RETURN, count=2000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
            calls = current["calls"]
            self.assertLessEqual(len(calls), 12)
            original = (
                struct.unpack("<I", cpu.mem_read(physical(TICK), 4))[0],
                read_byte(TICK_RESULT), read_byte(FLAG), len(calls),
                *calls, *([0] * (12 - len(calls))),
                *read_words(REFERENCE, 8), *read_words(BASIS, 8), argument,
            )
            self.assertEqual(native_result, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
