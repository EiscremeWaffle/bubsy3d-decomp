import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import (
        UC_MIPS_REG_A0,
        UC_MIPS_REG_A1,
        UC_MIPS_REG_PC,
        UC_MIPS_REG_RA,
        UC_MIPS_REG_V0,
    )


INPUT = struct.Struct("<5I")
OUTPUT = struct.Struct("<8I")


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSequenceEffectWrapperTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_wrappers_against_original(self):
        values = (0, 1, 0x7FFF, 0x8000, 0xFFFF, 0x1234FFFF, 0x80000000)
        statuses = (0, 1, 0x1B, 0x80000000, 0xFFFFFFFF)
        cases = [
            (first, second, status, 0xABCDEF12, which)
            for which in range(2)
            for first in values
            for second in values
            for status in statuses
        ]
        generator = random.Random(0x5E708)
        cases.extend(
            (
                generator.getrandbits(32), generator.getrandbits(32),
                generator.choice(statuses), generator.getrandbits(32), generator.randrange(2),
            )
            for _ in range(1500)
        )
        native = run_native_probe(
            self,
            ["src/game/func_8005E708.c", "src/game/func_8005E760.c",
             "tests/test_sequence_effect_wrappers.c"],
            cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        state = {}

        def intercept(uc, address, size, data):
            if address == 0x8005D5A8:
                state["selector"].append((uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)))
                uc.reg_write(UC_MIPS_REG_A0, 0xDEADBEEF)
                uc.reg_write(UC_MIPS_REG_A1, 0xA5A5A5A5)
                uc.reg_write(UC_MIPS_REG_V0, state["selector_result"])
            elif address in (0x8008DBF8, 0x8008DF28):
                state["backend"].append((
                    uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1),
                    1 if address == 0x8008DBF8 else 2,
                ))
                uc.reg_write(UC_MIPS_REG_V0, state["backend_result"])
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, actual) in enumerate(zip(cases, native)):
            first, second, selector_result, backend_result, which = entry
            state.update(selector=[], backend=[], selector_result=selector_result, backend_result=backend_result)
            cpu.reg_write(UC_MIPS_REG_A0, first)
            cpu.reg_write(UC_MIPS_REG_A1, second)
            self.call(cpu, (0x8005E708, 0x8005E760)[which])
            selector = state["selector"]
            backend = state["backend"]
            self.assertEqual(len(selector), 1)
            self.assertLessEqual(len(backend), 1)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0), len(selector), *selector[0], len(backend),
                *(backend[0] if backend else (0, 0, 0)),
            )
            with self.subTest(index=index, input=entry):
                self.assertEqual(actual, original)
