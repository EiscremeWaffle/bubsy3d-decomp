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
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<9I")
OUTPUT = struct.Struct("<23I")
ACTOR = 0x801E4000
PARAMETER_SOURCE, PARAMETER_POINTER, PARAMETERS = 0x801E4100, 0x801D8958, 0x801D05C4
INITIAL_PARAMETERS = (0, 1, 2, 3, 0x80000000, 5, 6, 0xFFFFFFFF)
LOCK, MODE = 0x80186479, 0x800BE548
STOP, UNKNOWN, DECELERATION, ACTIVE = 0x8018649C, 0x80186460, 0x80186464, 0x801864B8


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerCycleCompleteTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_completion_against_original(self):
        self.compare_completion(real_reset=False)

    def test_completion_with_real_reset(self):
        self.compare_completion(real_reset=True)

    def compare_completion(self, real_reset):
        rng = random.Random(0x38368)
        cases = []
        for mode, mutations in itertools.product((0, 1, 2, 255), range(32)):
            words = [rng.getrandbits(32) for _ in range(9)]
            words[3], words[8] = mode, mutations
            cases.append(tuple(words))
        cases.extend((*(rng.getrandbits(32) for _ in range(8)), rng.randrange(32)) for _ in range(1000))
        if real_reset:
            cases = [(*entry[:8], entry[8] & ~1) for entry in cases]
        sources = ["src/game/func_80038368.c", "tests/test_controller_cycle_complete.c"]
        flags = []
        if real_reset:
            sources.extend(["src/game/func_80028E80.c", "src/game/func_80044C34.c"])
            flags.append("-DCYCLE_REAL_RESET")
        native_results = run_native_probe(self, sources, cases, INPUT, OUTPUT, compiler_flags=flags)
        cpu = self.fresh_cpu()
        current = {}

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def write8(address, value):
            cpu.mem_write(physical(address), bytes((value & 0xFF,)))

        def read8(address):
            return cpu.mem_read(physical(address), 1)[0]

        def intercept(uc, address, size, data):
            a0, a1, a2, a3 = (uc.reg_read(reg) & 0xFFFFFFFF
                              for reg in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3))
            stack = uc.reg_read(UC_MIPS_REG_SP)
            mutations = current["mutations"]
            calls = current["calls"]
            result = 0
            if address == 0x80028764:
                self.assertEqual(a0, 0)
                calls.append(1)
                result = ACTOR
            elif address == 0x80028E80:
                if real_reset:
                    return
                calls.append(2)
                if mutations & 1:
                    write8(MODE, read8(MODE) ^ 1)
            elif address == 0x8001C700:
                self.assertEqual((a0, a1), (4, 14 if read8(MODE) else 15))
                calls.append(3)
                if mutations & 2:
                    write8(LOCK, 99)
            elif address == 0x8001B558:
                self.assertEqual((a0, a1, a2, a3), (0x2E, 4, 0, 0))
                self.assertEqual(bytes(uc.mem_read(physical(stack + 0x10), 16)), bytes(16))
                self.assertEqual(read8(LOCK), 2)
                calls.append(4)
                if mutations & 4:
                    write32(ACTOR + 0x10, read32(ACTOR + 0x10) ^ 0x80808080)
                    write8(STOP, read8(STOP) ^ 1)
            elif address == 0x8002269C:
                self.assertEqual((a0, a1), (ACTOR, 0x10))
                calls.append(5)
                if mutations & 8:
                    write8(ACTOR + 1, read8(ACTOR + 1) ^ 0x80)
                    write8(UNKNOWN, read8(UNKNOWN) ^ 1)
            elif address == 0x8003539C:
                self.assertEqual((a0, a1, read32(ACTOR + 0x10), read8(ACTOR + 1)), (ACTOR, 1, 0, 2))
                self.assertEqual(tuple(read8(addr) for addr in (STOP, UNKNOWN, DECELERATION, ACTIVE)),
                                 (0, 0, 0, 0))
                calls.append(6)
                if mutations & 16:
                    write32(ACTOR + 0x10, 0x12345678)
                    write8(ACTIVE, 7)
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, result)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            current.update(mutations=entry[8], calls=[])
            cpu.mem_write(physical(ACTOR), bytes(0x30))
            if real_reset:
                cpu.mem_write(physical(PARAMETER_SOURCE), struct.pack("<8I", *INITIAL_PARAMETERS))
                cpu.mem_write(physical(PARAMETERS), bytes([0xA5]) * 32)
                write32(PARAMETER_POINTER, PARAMETER_SOURCE)
            write32(ACTOR + 0x10, entry[0])
            for address, value in zip((ACTOR + 1, LOCK, MODE, STOP, UNKNOWN, DECELERATION, ACTIVE), entry[1:8]):
                write8(address, value)
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.emu_start(0x80038368, RETURN, count=200)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN, f"case {index} did not return")
            original = (read32(ACTOR + 0x10), *(read8(addr) for addr in
                        (ACTOR + 1, LOCK, MODE, STOP, UNKNOWN, DECELERATION, ACTIVE)),
                        len(current["calls"]), *current["calls"], *([0] * (6 - len(current["calls"]))),
                        *(struct.unpack("<8I", cpu.mem_read(physical(PARAMETERS), 32)) if real_reset else (0,) * 8))
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
