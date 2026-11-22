import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_PC, UC_MIPS_REG_RA,
        UC_MIPS_REG_S2, UC_MIPS_REG_SP, UC_MIPS_REG_V0,
    )

INPUT = struct.Struct("<12I")
OUTPUT = struct.Struct("<29I")
HANDLE_GLOBAL = 0x800BE0C4
RESET = 0x801D5F30
HANDLES = (0x801E4000, 0x801E4100)
COMPONENTS = (0x801E4200, 0x801E4300)
INFOS = (0x801D4E28, 0x801E5000)


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerApproachTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_approach_against_original(self):
        self.compare_approach(real_multiply=False)

    def test_approach_with_real_multiply(self):
        self.compare_approach(real_multiply=True)

    def compare_approach(self, real_multiply):
        rng = random.Random(0x49CB8)
        cases = []
        mutations_set = (0, 4, 8, 12, 32, 64, 128, 224, 236) if real_multiply else range(256)
        for mutations in mutations_set:
            for value in (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF):
                cases.append((value, value, value, 0x12345678, value, value, value,
                              value, value, value, 99, mutations))
        cases.extend((*(rng.getrandbits(32) for _ in range(11)), rng.choice(mutations_set))
                     for _ in range(2000))
        sources = ["src/game/secondary_controller_approach.c", "tests/test_secondary_controller_approach.c"]
        flags = []
        if real_multiply:
            sources.append("src/game/func_8001007C.c")
            flags.append("-DSECONDARY_APPROACH_REAL_MULTIPLY")
        native_results = run_native_probe(
            self, sources, cases, INPUT, OUTPUT, compiler_flags=flags,
        )
        cpu = self.fresh_cpu()
        current = {}

        def write32(address, value):
            cpu.mem_write(physical(address), struct.pack("<I", value & 0xFFFFFFFF))

        def read32(address):
            return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]

        def read_words(address, count):
            return struct.unpack("<%dI" % count, cpu.mem_read(physical(address), 4 * count))

        def xor_word(address, mask):
            write32(address, read32(address) ^ mask)

        def intercept(uc, address, size, data):
            if address in (0x80049DD0, 0x80049FF8):
                current["branch"] = int(address == 0x80049DD0)
                uc.emu_stop()
                return
            a0, a1, a2 = (uc.reg_read(reg) & 0xFFFFFFFF for reg in
                          (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2))
            stack = current["stack"]
            if address == 0x8001007C:
                if real_multiply:
                    return
                multiply = current["multiplies"]
                self.assertEqual((a1, a2), (672, stack + 0xE0 + multiply * 4))
                if multiply == 0:
                    self.assertEqual(read32(RESET), 0)
                current["calls"].append(1 + multiply)
                write32(a2, a0 ^ a1 ^ (0x12340000 + multiply))
                if multiply == 0:
                    if current["mutations"] & 1:
                        xor_word(stack + 0x34, 0x01010101)
                    if current["mutations"] & 2:
                        write32(HANDLE_GLOBAL, HANDLES[1])
                    if current["mutations"] & 16:
                        write32(RESET, 0x12345678)
                current["multiplies"] += 1
            elif address == 0x80054C7C:
                self.assertEqual((a0, a1, a2), (COMPONENTS[0], 0, stack + 0xC0))
                current["calls"].append(4)
                if current["mutations"] & 4:
                    xor_word(stack + 0xC4, 0x02020202)
                    xor_word(stack + 0xE8, 0x04040404)
            elif address == 0x8005536C:
                self.assertEqual((a0, a1, a2), (COMPONENTS[0], 0, stack + 0xC0))
                current["calls"].append(5)
                current["before_sample"] = read_words(a2, 3)
                if current["mutations"] & 8:
                    xor_word(stack + 0xC0, 0x08080808)
                    xor_word(RESET, 0xA5A5A5A5)
            elif address == 0x80054B20:
                self.assertEqual((a0, a1, a2), (COMPONENTS[0], 0, stack + 0xC0))
                current["calls"].append(6)
                uc.mem_write(physical(a2), struct.pack("<3I", *current["sample"]))
            elif address == 0x80052BDC:
                self.assertEqual((a0, a1), (read32(HANDLE_GLOBAL), 0))
                current["calls"].append(7)
                if current["mutations"] & 32:
                    write32(HANDLE_GLOBAL, HANDLES[1])
            elif address == 0x800222EC:
                self.assertEqual((a0, a1), (RESET + 0x24, stack + 0xC0))
                self.assertEqual(read32(a0), INFOS[0])
                self.assertEqual(read32(RESET + 0x18), read32(HANDLE_GLOBAL))
                current["calls"].append(8)
                if current["mutations"] & 64:
                    xor_word(stack + 0xC4, 0x80000000)
                if current["mutations"] & 128:
                    write32(a0, INFOS[1])
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        stack = STACK - 0x128
        for index, (entry, native) in enumerate(zip(cases, native_results)):
            position, direction, sample, reset, mutations = entry[:4], entry[4:7], entry[7:10], entry[10], entry[11]
            current.update(stack=stack, sample=sample, mutations=mutations, calls=[], multiplies=0,
                           branch=None, before_sample=None)
            cpu.mem_write(physical(stack), bytes(0x128))
            cpu.mem_write(physical(stack + 0x40), struct.pack("<4I", *position))
            cpu.mem_write(physical(stack + 0x30), struct.pack("<3I", *direction))
            write32(HANDLE_GLOBAL, HANDLES[0])
            write32(HANDLES[0] + 0x14, COMPONENTS[0])
            write32(HANDLES[1] + 0x14, COMPONENTS[1])
            write32(RESET, reset)
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.reg_write(UC_MIPS_REG_A2, stack + 0xE0)
            cpu.emu_start(0x80049CB8, 0, count=1000)
            self.assertIsNotNone(current["branch"], f"case {index} did not reach a boundary")
            component = cpu.reg_read(UC_MIPS_REG_S2)
            handle = read32(HANDLE_GLOBAL)
            actor_handle = read32(RESET + 0x18)
            info = read32(RESET + 0x24)
            original = (
                *read_words(stack + 0xC0, 4), *read_words(stack + 0xE0, 3), *current["before_sample"],
                COMPONENTS.index(component) + 1 if component in COMPONENTS else 0,
                read32(RESET), *read_words(stack + 0x30, 3),
                HANDLES.index(handle) + 1 if handle in HANDLES else 0,
                HANDLES.index(actor_handle) + 1 if actor_handle in HANDLES else 0,
                INFOS.index(info) + 1 if info in INFOS else 0,
                read32(stack + 0xF4), current["branch"],
                len(current["calls"]), *current["calls"], *([0] * (8 - len(current["calls"]))),
            )
            self.assertEqual(native, original, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
