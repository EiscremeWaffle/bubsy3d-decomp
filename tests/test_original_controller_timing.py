import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RETURN, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_SP,
        UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<7I")
OUTPUT = struct.Struct("<10I")
STATUS = 0x801E0000
TICK = 0x801E0010
STATUS_POINTER = 0x800A436C
TICK_POINTER = 0x800A4370
PREVIOUS = 0x800A4374
POSITION = 0x800A4378
CURRENT = 0x800A5438


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerTimingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_timing_helper_against_original(self):
        generator = random.Random(0x6BB84)
        cases = []
        for count, status, mutations in itertools.product(
            (0xFFFFFFFF, 0x80000000, 0, 1, 2, 3, 0x7FFFFFFF),
            (0, 0x80000000, 0x00080000, 0x80080000),
            (0, 1, 2, 3, 4, 5, 6, 7),
        ):
            if status & 0x00080000 and not mutations & 4 and count in (0, 2, 3, 0x7FFFFFFF):
                continue
            cases.append((
                count, status, generator.getrandbits(32), generator.getrandbits(32),
                generator.choice((0, 1, 0xFFFFFFFF, 0x7FFFFFFF)),
                generator.choice((0, 1, 0xFFFFFFFF, 0x7FFFFFFF)), mutations,
            ))
        native = run_native_probe(
            self, ["src/game/func_8006BB84.c", "tests/test_controller_timing.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def intercept(uc, address, size, data):
            if address != 0x8006BCCC:
                return
            current["arguments"].extend((
                uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF,
                uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF,
            ))
            current["calls"] += 1
            mutations = current["mutations"]
            if current["calls"] == 1:
                if mutations & 1:
                    write32(CURRENT, read32(CURRENT) + 7)
                    write32(TICK, read32(TICK) + 0x12345)
                if mutations & 2:
                    write32(STATUS, read32(STATUS) ^ 0x80000000)
            elif mutations & 4:
                write32(STATUS, read32(STATUS) ^ 0x80000000)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native_result) in enumerate(zip(cases, native)):
            count, status, tick, previous, position, value, mutations = entry
            current.update(calls=0, arguments=[], mutations=mutations)
            for address, word in (
                (STATUS_POINTER, STATUS), (TICK_POINTER, TICK), (STATUS, status),
                (TICK, tick), (PREVIOUS, previous), (POSITION, position), (CURRENT, value),
            ):
                write32(address, word)
            cpu.reg_write(UC_MIPS_REG_A0, count)
            self.call(cpu, 0x8006BB84)
            args = current["arguments"]
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                read32(PREVIOUS), read32(POSITION), current["calls"],
                *args, *([0] * (4 - len(args))), read32(STATUS), read32(TICK),
            )
            self.assertEqual(native_result, original, f"case {index}: {entry}")

    def test_waits_until_status_high_bit_changes(self):
        cpu = self.fresh_cpu()
        visits = []

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def intercept(uc, address, size, data):
            if address == 0x8006BCCC:
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
            elif address == 0x8006BC74:
                visits.append(address)
                if len(visits) == 3:
                    write32(STATUS, 0x80080000)

        for address, word in (
            (STATUS_POINTER, STATUS), (TICK_POINTER, TICK), (STATUS, 0x00080000),
            (TICK, 25), (PREVIOUS, 10), (POSITION, 2), (CURRENT, 4),
        ):
            write32(address, word)
        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.reg_write(UC_MIPS_REG_A0, 0)
        self.call(cpu, 0x8006BB84)
        self.assertEqual(len(visits), 3)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 15)

    def test_timing_helper_with_real_wait_backend(self):
        cpu = self.fresh_cpu()
        calls = []

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def intercept(uc, address, size, data):
            if address not in (0x80082E4C, 0x8006BB74, 0x8006BD68):
                return
            calls.append(address)
            if address == 0x80082E4C:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 0x80013EA8)
                write32(CURRENT, 2)
                write32(TICK, 100)
            elif address == 0x8006BB74:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 0)
            else:
                self.assertEqual((uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)), (3, 0))
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        for address, word in (
            (STATUS_POINTER, STATUS), (TICK_POINTER, TICK), (STATUS, 0),
            (TICK, 50), (PREVIOUS, 46), (POSITION, 0), (CURRENT, 0),
        ):
            write32(address, word)
        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.reg_write(UC_MIPS_REG_A0, 0)
        cpu.reg_write(UC_MIPS_REG_SP, STACK)
        cpu.reg_write(UC_MIPS_REG_RA, RETURN)
        cpu.emu_start(0x8006BB84, RETURN, count=1000000)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
        self.assertEqual(calls, [0x80082E4C, 0x8006BB74, 0x8006BD68])
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 4)
        self.assertEqual((read32(POSITION), read32(PREVIOUS)), (2, 100))


if __name__ == "__main__":
    unittest.main()
