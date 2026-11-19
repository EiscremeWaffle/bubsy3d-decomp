import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import GP, HAS_UNICORN, ORIGINAL, STACK, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_PC, UC_MIPS_REG_SP


INPUT = struct.Struct("<7I")
OUTPUT = struct.Struct("<9I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerInputPreparationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def test_input_preparation_against_original(self):
        button_bits = (8, 11, 24, 27, 28, 30, 22)
        cases = []
        for mask in range(1 << len(button_bits)):
            buttons = sum(1 << bit for index, bit in enumerate(button_bits) if mask & (1 << index))
            for flags in range(64):
                cases.append((buttons, *((flags >> bit) & 1 for bit in range(6))))
        generator = random.Random(0x23E94)
        cases.extend(
            (generator.getrandbits(32), *(generator.choice((0, 1, 2, 255)) for _ in range(6)))
            for _ in range(3000)
        )
        native = run_native_probe(
            self,
            ["src/game/controller_input_prepare.c", "src/game/func_80056678.c",
             "src/game/func_8005668C.c", "tests/test_controller_input_prepare.c"],
            cases, INPUT, OUTPUT,
        )

        record = 0x801E5000
        cpu = self.fresh_cpu()

        def stop_after_prefix(uc, address, size, data):
            if address == 0x80024068:
                uc.emu_stop()

        cpu.hook_add(UC_HOOK_CODE, stop_after_prefix)
        for index, (entry, actual) in enumerate(zip(cases, native)):
            buttons, status_1, status_2, latched_1, latched_2, second_port, mirror_extra = entry
            data = bytearray([0xA5] * 28)
            struct.pack_into("<I", data, 0x0C, buttons)
            data[0x10], data[0x11] = status_1 & 0xFF, status_2 & 0xFF
            cpu.mem_write(physical(record), bytes(data))
            for address, value in (
                (GP + 0x1A7, latched_1), (GP + 0x1A8, latched_2),
                (0x801D5F7C, second_port), (GP + 0x1A3, mirror_extra),
            ):
                cpu.mem_write(physical(address), bytes((value & 0xFF,)))
            cpu.reg_write(UC_MIPS_REG_SP, STACK)
            cpu.reg_write(UC_MIPS_REG_A0, record)
            cpu.emu_start(0x80023E94, 0x80024068, count=5000)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), 0x80024068)
            original = (
                *struct.unpack("<7I", cpu.mem_read(physical(record), 28)),
                cpu.mem_read(physical(GP + 0x1A7), 1)[0],
                cpu.mem_read(physical(GP + 0x1A8), 1)[0],
            )
            with self.subTest(index=index, input=entry):
                self.assertEqual(actual, original)
