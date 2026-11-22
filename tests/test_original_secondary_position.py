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


INPUT = struct.Struct("<36I")
OUTPUT = struct.Struct("<6I")
OWNER = 0x801E0000
COMPONENTS = (0x801E0100, 0x801E0200)
BASIS = 0x801E0300
RESULT = 0x801E0400


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryPositionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_position_against_original(self):
        generator = random.Random(0x58E7C)
        cases = [
            (*(generator.getrandbits(32) for _ in range(6)), mutation, null_basis,
             *(generator.getrandbits(32) for _ in range(28)))
            for mutation, null_basis in itertools.product((0, 1), repeat=2)
            for _ in range(32)
        ]
        native_outputs = run_native_probe(
            self, ["src/game/func_80058E7C.c", "tests/test_secondary_position.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_words(uc, address, count):
            return struct.unpack(f"<{count}I", uc.mem_read(physical(address), 4 * count))

        def write_words(uc, address, words):
            uc.mem_write(physical(address), struct.pack(f"<{len(words)}I", *words))

        def intercept(uc, address, size, data):
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            a2 = uc.reg_read(UC_MIPS_REG_A2) & 0xFFFFFFFF
            call_index = current["calls"]
            if address == 0x80058C74:
                self.assertEqual((call_index, a0), (0, OWNER))
                write_words(uc, a1, current["bounds"])
            elif address == 0x80010000:
                self.assertEqual(call_index, 1)
                self.assertEqual(read_words(uc, a0, 3), current["bounds"][4:7])
                self.assertEqual(read_words(uc, a1, 3), current["bounds"][:3])
                write_words(uc, a2, current["difference"])
            elif address == 0x80053EA0:
                self.assertEqual(call_index, 2)
                self.assertEqual(read_words(uc, a0, 3), current["difference"])
                write_words(uc, a1, current["half"])
            elif address == 0x80010038:
                self.assertEqual(call_index, 3)
                self.assertEqual(read_words(uc, a0, 3), current["bounds"][:3])
                self.assertEqual(read_words(uc, a1, 3), current["half"])
                write_words(uc, a2, current["center"])
                if current["mutation"]:
                    write_words(uc, OWNER + 0x14, (COMPONENTS[1],))
            elif address == 0x8005476C:
                self.assertEqual(call_index, 4)
                self.assertEqual(a0, COMPONENTS[current["mutation"]])
                current["selected_component"] = current["mutation"]
                write_words(uc, a1, current["normalized"])
            elif address == 0x80064528:
                self.assertEqual((call_index, a2), (5, RESULT))
                self.assertEqual(read_words(uc, a0, 3), current["center"])
                self.assertEqual(read_words(uc, a1, 8), current["normalized"])
                write_words(uc, a2, current["result"])
            else:
                return
            current["calls"] += 1
            uc.reg_write(UC_MIPS_REG_V0, current["statuses"][call_index])
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            current.update(
                calls=0, selected_component=0, statuses=entry[:6], mutation=entry[6],
                bounds=entry[8:16], difference=entry[16:19], half=entry[19:22],
                center=entry[22:25], normalized=entry[25:33], result=entry[33:36],
            )
            write_words(cpu, OWNER + 0x14, (COMPONENTS[0],))
            cpu.reg_write(UC_MIPS_REG_A0, OWNER)
            cpu.reg_write(UC_MIPS_REG_A1, 0 if entry[7] else BASIS)
            cpu.reg_write(UC_MIPS_REG_A2, RESULT)
            self.call(cpu, 0x80058E7C)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, current["calls"],
                current["selected_component"], *read_words(cpu, RESULT, 3),
            )
            self.assertEqual(native, original, f"case {index}: {entry}")
