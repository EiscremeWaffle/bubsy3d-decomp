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
        UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
        UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<7I")
OUTPUT = struct.Struct("<254I")
LOADED = 0x80189C71
SLOTS = 0x80189C90
EMPTY = 0xFFFFFFFF


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalTextureFontPositionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_font_position_against_original(self):
        self.compare_font_position(real_setters=False)

    def test_font_position_with_real_setters_against_original(self):
        self.compare_font_position(real_setters=True)

    def test_original_setter_boundaries_and_null(self):
        cpu = self.fresh_cpu()
        payload = 0x801E0000
        for address, offset in ((0x80055DF4, 28), (0x80055E38, 30)):
            for value in (0, 0x7FFF, 0x8000, 0xFFFF, 0x12345678, 0xFFFFFFFF):
                cpu.reg_write(UC_MIPS_REG_A0, 0)
                cpu.reg_write(UC_MIPS_REG_A1, value)
                self.call(cpu, address)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 22)
                before = bytes(range(40))
                cpu.mem_write(physical(payload), before)
                cpu.reg_write(UC_MIPS_REG_A0, payload)
                cpu.reg_write(UC_MIPS_REG_A1, value)
                self.call(cpu, address)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)
                expected = bytearray(before)
                expected[offset:offset + 2] = struct.pack("<H", value & 0xFFFF)
                self.assertEqual(cpu.mem_read(physical(payload), 40), expected)

    def compare_font_position(self, real_setters):
        generator = random.Random(0x481D4)
        cases = [
            (loaded, assertion_loaded, tag, generator.getrandbits(32), generator.getrandbits(32), matches, mutations)
            for loaded, assertion_loaded, tag, matches, mutations in itertools.product(
                (0, 1, 2), (0, 1, 2), (0, 127, 128, 255, 0x100, 0xFFFFFF80),
                (0, 1, 3, 5, 15), (0, 1, 2, 4, 8, 15),
            )
        ]
        sources = ["src/game/func_800481D4.c", "tests/test_texture_font_position.c"]
        if real_setters:
            sources += ["src/game/func_80055DF4.c", "src/game/func_80055E38.c"]
        native = run_native_probe(self, sources, cases, INPUT, OUTPUT,
                                  compiler_flags=("-DREAL_POSITION",) if real_setters else ())
        cpu = self.fresh_cpu()
        current = {}

        def write_byte(uc, address, value):
            uc.mem_write(physical(address), bytes((value & 0xFF,)))

        def intercept(uc, address, size, data):
            if address == 0x800556D0:
                self.assertEqual((uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1),
                                  uc.reg_read(UC_MIPS_REG_A2), uc.reg_read(UC_MIPS_REG_A3)),
                                 (0, 0x80013730, 0x80013724, 0x17A))
                current["assertions"] += 1
                write_byte(uc, LOADED, current["assertion_loaded"])
            elif address in (0x80055DF4, 0x80055E38):
                payload = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
                offset = payload - SLOTS - 4
                self.assertEqual(offset % 44, 0)
                index = offset // 44
                self.assertIn(index, range(50))
                position = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
                value = current["x"] if address == 0x80055DF4 else current["y"]
                signed = value & 0xFFFF
                expected = signed if signed < 0x8000 else signed | 0xFFFF0000
                self.assertEqual(position, expected)
                if real_setters:
                    return
                current["calls"].append(index * 2 + (address == 0x80055E38))
                if index == 0 and address == 0x80055DF4:
                    if current["mutations"] & 1:
                        write_byte(uc, SLOTS + 44, current["tag"])
                    if current["mutations"] & 2:
                        write_byte(uc, LOADED, 0)
                    if current["mutations"] & 4:
                        write_byte(uc, SLOTS, current["tag"] ^ 0x80)
                if index == 0 and address == 0x80055E38 and current["mutations"] & 8:
                    write_byte(uc, SLOTS + 44, current["tag"] ^ 0x80)
            else:
                return
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for case_index, (entry, expected) in enumerate(zip(cases, native)):
            loaded, assertion_loaded, tag, x, y, matches, mutations = entry
            current.update(assertion_loaded=assertion_loaded, tag=tag, x=x, y=y,
                           mutations=mutations, assertions=0, calls=[])
            cpu.mem_write(physical(SLOTS), bytes(50 * 44))
            for index in range(50):
                write_byte(cpu, SLOTS + index * 44, tag ^ 0x80)
            for bit, index in enumerate((0, 1, 25, 49)):
                if matches & (1 << bit):
                    write_byte(cpu, SLOTS + index * 44, tag)
            write_byte(cpu, LOADED, loaded)
            cpu.reg_write(UC_MIPS_REG_A0, tag)
            cpu.reg_write(UC_MIPS_REG_A1, x)
            cpu.reg_write(UC_MIPS_REG_A2, y)
            self.call(cpu, 0x800481D4)
            tags = [cpu.mem_read(physical(SLOTS + index * 44), 1)[0] for index in range(50)]
            positions = [EMPTY] * 100
            if real_setters:
                positions = [
                    value
                    for index in range(50)
                    for value in struct.unpack("<2H", cpu.mem_read(physical(SLOTS + index * 44 + 32), 4))
                ]
            observed = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                current["assertions"], cpu.mem_read(physical(LOADED), 1)[0], len(current["calls"]),
                *(current["calls"] + [EMPTY] * (100 - len(current["calls"]))), *tags, *positions,
            )
            self.assertEqual(observed, expected, f"case {case_index}: {entry}")
