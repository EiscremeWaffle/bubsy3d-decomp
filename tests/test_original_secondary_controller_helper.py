import itertools
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, RETURN, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_SP, UC_MIPS_REG_V0


INPUT = struct.Struct("<3I")
OUTPUT = struct.Struct("<4I")
FLOW_OUTPUT = struct.Struct("<8I")
EVENT_POINTER = GP + 0x8E4
FLAG = GP + 0x507
EVENTS = (0, 0x801E5000, 0x801E5100, 0x801E5200)
EVENT_LIST = 0x800BE5B8
EVENT_BUSY = 0x800BE5BC


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryControllerHelperTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_event_removal_wrapper_against_original(self):
        cases = list(itertools.product(range(256), range(3), range(4)))
        native_outputs = run_native_probe(
            self, ["src/game/func_800493CC.c", "tests/test_secondary_controller_helper.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_word(uc, address):
            return struct.unpack("<I", uc.mem_read(physical(address), 4))[0]

        def write_word(uc, address, value):
            uc.mem_write(physical(address), struct.pack("<I", value))

        def intercept(uc, address, size, data):
            if address != 0x8004FFEC:
                return
            self.assertEqual(uc.mem_read(physical(FLAG), 1)[0], 1)
            current["argument"] = EVENTS.index(uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF)
            current["calls"] += 1
            if current["mutations"] & 1:
                uc.mem_write(physical(FLAG), b"\xA5")
            if current["mutations"] & 2:
                write_word(uc, EVENT_POINTER, EVENTS[2])
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            flag, event, mutations = entry
            current.update(mutations=mutations, argument=0, calls=0)
            cpu.mem_write(physical(FLAG), bytes((flag,)))
            write_word(cpu, EVENT_POINTER, EVENTS[event])
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.emu_start(0x800493CC, RETURN, count=1000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
            original = (
                cpu.mem_read(physical(FLAG), 1)[0],
                EVENTS.index(read_word(cpu, EVENT_POINTER)), current["argument"], current["calls"],
            )
            self.assertEqual(original, native, f"case {index}: {entry}")

    def test_event_removal_wrapper_with_original_list_backend(self):
        cases = list(itertools.product((0, 1, 255), range(4), range(4)))
        native_outputs = run_native_probe(
            self, [
                "src/game/func_800493CC.c", "src/game/func_8004FFEC.c",
                "tests/test_secondary_controller_event_flow.c",
            ],
            cases, INPUT, FLOW_OUTPUT,
        )
        cpu = self.fresh_cpu()
        current = {}

        def read_word(uc, address):
            return struct.unpack("<I", uc.mem_read(physical(address), 4))[0]

        def write_word(uc, address, value):
            uc.mem_write(physical(address), struct.pack("<I", value))

        def intercept(uc, address, size, data):
            if address != 0x80082EFC:
                return
            self.assertEqual(uc.mem_read(physical(FLAG), 1)[0], 1)
            self.assertEqual(uc.mem_read(physical(EVENT_BUSY), 1)[0], 1)
            current["freed"] = EVENTS.index(uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF)
            current["frees"] += 1
            if current["mutations"] & 1:
                uc.mem_write(physical(FLAG), b"\xA5")
            if current["mutations"] & 2:
                write_word(uc, EVENT_POINTER, EVENTS[3])
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            flag, event, mutations = entry
            current.update(mutations=mutations, freed=0xFFFFFFFF, frees=0)
            for record in EVENTS[1:]:
                cpu.mem_write(physical(record), bytes(16))
            write_word(cpu, EVENTS[1] + 0x0C, EVENTS[2])
            write_word(cpu, EVENT_LIST, EVENTS[1])
            write_word(cpu, EVENT_POINTER, EVENTS[event])
            cpu.mem_write(physical(EVENT_BUSY), b"\x09")
            cpu.mem_write(physical(FLAG), bytes((flag,)))
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_RA, RETURN)
            cpu.emu_start(0x800493CC, RETURN, count=1000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)
            original = (
                cpu.mem_read(physical(FLAG), 1)[0], EVENTS.index(read_word(cpu, EVENT_POINTER)),
                EVENTS.index(read_word(cpu, EVENT_LIST)), EVENTS.index(read_word(cpu, EVENTS[1] + 0x0C)),
                EVENTS.index(read_word(cpu, EVENTS[2] + 0x0C)), cpu.mem_read(physical(EVENT_BUSY), 1)[0],
                current["freed"], current["frees"],
            )
            self.assertEqual(original, native, f"case {index}: {entry}")


if __name__ == "__main__":
    unittest.main()
