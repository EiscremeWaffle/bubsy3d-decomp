import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RotationGteModel, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_PC, UC_MIPS_REG_RA, UC_MIPS_REG_V0


INPUT = struct.Struct("<33I")
OUTPUT = struct.Struct("<19I")
DESCRIPTOR = 0x801E0000
BINDING = 0x801E1000
WORKING = 0x801D58F0
RENDER = 0x801D3414


def random_matrix(generator):
    matrices = (
        (4096, 0, 0, 0, 4096, 0, 0, 0, 4096),
        (0, -4096, 0, 4096, 0, 0, 0, 0, 4096),
        (4096, 0, 0, 0, 0, -4096, 0, 4096, 0),
        tuple(generator.randrange(-4096, 4097) for _ in range(9)),
    )
    matrix = generator.choice(matrices)
    padding = generator.randrange(0x10000)
    translation = tuple(generator.randrange(-10000, 10001) for _ in range(3))
    return struct.unpack("<8I", struct.pack("<9hH3i", *matrix, padding, *translation))


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalTransformResetRealCompositionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_reset_with_real_composition_against_original(self):
        generator = random.Random(0x731C07242C)
        cases = [
            (binding, *random_matrix(generator), *random_matrix(generator),
             *random_matrix(generator), *random_matrix(generator))
            for binding, _ in itertools.product(range(2), range(256))
        ]
        native_outputs = run_native_probe(
            self, [
                "src/game/func_800731C0.c", "src/game/func_80077C00.c",
                "src/game/func_8007242C.c", "src/game/gte_rotation_composition.c",
                "tests/test_transform_reset_real_composition.c",
            ], cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        gte = RotationGteModel()
        current = {}

        def intercept(uc, address, size, data):
            if address == 0x80073324:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), BINDING)
                output = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
                uc.mem_write(physical(output), struct.pack("<8I", *current["hierarchy"]))
                current["hierarchy_calls"] += 1
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
            elif address == 0x80077298:
                current["transform_calls"] += 1

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.hook_add(UC_HOOK_CODE, gte.hook)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            binding = entry[0]
            descriptor, hierarchy, working, render = entry[1:9], entry[9:17], entry[17:25], entry[25:33]
            current.update(hierarchy=hierarchy, hierarchy_calls=0, transform_calls=0)
            gte.control.clear()
            gte.data.clear()
            gte.rotations = 0
            gte.basis_transform_passes = 0
            cpu.mem_write(physical(DESCRIPTOR), struct.pack("<8II", *descriptor, BINDING if binding else 0))
            cpu.mem_write(physical(WORKING), struct.pack("<8I", *working))
            cpu.mem_write(physical(RENDER), struct.pack("<8I", *render))
            cpu.reg_write(UC_MIPS_REG_A0, DESCRIPTOR)
            self.call(cpu, 0x800731C0)
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF,
                current["hierarchy_calls"], current["transform_calls"],
                *struct.unpack("<8I", cpu.mem_read(physical(WORKING), 32)),
                *struct.unpack("<8I", cpu.mem_read(physical(RENDER), 32)),
            )
            with self.subTest(index=index, binding=binding):
                self.assertEqual(native, original)
                self.assertEqual(gte.rotations, 3 if binding else 0)
                self.assertEqual(gte.basis_transform_passes, 4 if binding else 0)
