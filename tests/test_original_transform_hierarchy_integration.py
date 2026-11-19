import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, RotationGteModel, physical
from test_original_transform_reset_real_composition import random_matrix

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_V0


INPUT = struct.Struct("<114I")
OUTPUT = struct.Struct("<68I")
NODE_BASE = 0x801E0000
NODE_STRIDE = 0x80
DESCRIPTOR = 0x801CBF20
STATUS = 0x801CBED0
SNAPSHOT = 0x801CBED4
SOURCE = 0x801E3000
SOURCE_POINTER = 0x800BE5E4
CHAIN = 0x801D79B0
GENERATION = 0x801D8718
WORKING = 0x801D58F0
RENDER = 0x801D3414
NODE_COUNT = 4


def node_address(index):
    return NODE_BASE + NODE_STRIDE * index


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalTransformHierarchyIntegrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_reset_with_hierarchy_and_modeled_gte_against_original(self):
        generator = random.Random(0x731C073324)
        cases = []
        for binding, count, generation, route, source_alias, _ in itertools.product(
            range(2), range(1, 5), (0, 1, 7), range(2), range(2), range(16)
        ):
            versions = tuple(generator.choice((0, generation, 0x12345678)) for _ in range(NODE_COUNT))
            matrices = tuple(word for _ in range(NODE_COUNT * 2 + 3) for word in random_matrix(generator))
            source_and_snapshot = tuple(generator.getrandbits(32) for _ in range(16))
            cases.append((binding, count, generation, route, source_alias, generator.getrandbits(32),
                          *versions, *matrices, *source_and_snapshot))
        native_outputs = run_native_probe(
            self, [
                "src/game/func_80051678.c",
                "src/game/func_800731C0.c", "src/game/func_80073324.c",
                "src/game/func_80077C00.c", "src/game/func_8007242C.c",
                "src/game/func_800724B0.c", "src/game/gte_rotation_composition.c",
                "src/game/component_transform.c", "tests/test_transform_hierarchy_integration.c",
            ], cases, INPUT, OUTPUT,
        )
        cpu = self.fresh_cpu()
        gte = RotationGteModel()
        current = {}

        def intercept(uc, address, size, data):
            if address == 0x80077298:
                current["basis_calls"] += 1
            elif address == 0x800731C0:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF, DESCRIPTOR)
                current["reset_calls"] += 1

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.hook_add(UC_HOOK_CODE, gte.hook)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            binding, count, generation, route, source_alias, status = entry[:6]
            versions = entry[6:10]
            local, cached = entry[10:42], entry[42:74]
            descriptor, working, render = entry[74:82], entry[82:90], entry[90:98]
            source, snapshot = entry[98:106], entry[106:114]
            current.update(basis_calls=0, reset_calls=0)
            gte.control.clear()
            gte.data.clear()
            gte.rotations = 0
            gte.basis_transform_passes = 0
            cpu.mem_write(physical(GENERATION), struct.pack("<I", generation))
            cpu.mem_write(physical(CHAIN), bytes(NODE_COUNT * 4))
            cpu.mem_write(physical(DESCRIPTOR), struct.pack("<8II", *descriptor, node_address(0) if binding else 0))
            cpu.mem_write(physical(WORKING), struct.pack("<8I", *working))
            cpu.mem_write(physical(RENDER), struct.pack("<8I", *render))
            cpu.mem_write(physical(STATUS), struct.pack("<I", status))
            cpu.mem_write(physical(SOURCE), struct.pack("<8I", *source))
            cpu.mem_write(physical(SNAPSHOT), struct.pack("<8I", *snapshot))
            cpu.mem_write(physical(SOURCE_POINTER), struct.pack("<I", SNAPSHOT if source_alias else SOURCE))
            for node in range(NODE_COUNT):
                parent = node_address(node + 1) if node + 1 < count else 0
                contents = struct.pack("<18I", versions[node], *local[node * 8:node * 8 + 8],
                                       *cached[node * 8:node * 8 + 8], 0)
                cpu.mem_write(physical(node_address(node)), contents + struct.pack("<I", parent))
            cpu.reg_write(UC_MIPS_REG_A0, DESCRIPTOR)
            self.call(cpu, 0x80051678 if route else 0x800731C0)
            final_status, = struct.unpack("<I", cpu.mem_read(physical(STATUS), 4))
            final_generation, = struct.unpack("<I", cpu.mem_read(physical(GENERATION), 4))
            final_versions = tuple(struct.unpack("<I", cpu.mem_read(physical(node_address(node)), 4))[0]
                                   for node in range(NODE_COUNT))
            final_cached = tuple(
                word for node in range(NODE_COUNT)
                for word in struct.unpack("<8I", cpu.mem_read(physical(node_address(node) + 0x24), 32))
            )
            final_working = struct.unpack("<8I", cpu.mem_read(physical(WORKING), 32))
            final_render = struct.unpack("<8I", cpu.mem_read(physical(RENDER), 32))
            final_snapshot = struct.unpack("<8I", cpu.mem_read(physical(SNAPSHOT), 32))
            slots = struct.unpack("<4I", cpu.mem_read(physical(CHAIN), NODE_COUNT * 4))
            chain = tuple((address - NODE_BASE) // NODE_STRIDE if address else 0xFFFFFFFF for address in slots)
            original = (cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, final_status,
                        final_generation, current["basis_calls"], *final_versions, *final_cached,
                        *final_working, *final_render, *chain, *final_snapshot)
            with self.subTest(index=index, binding=binding, count=count, generation=generation,
                              route=route, source_alias=source_alias):
                self.assertEqual(native, original)
                self.assertEqual(current["reset_calls"], 1)
                self.assertEqual(final_status, 0 if route else status)
                self.assertEqual(final_snapshot, (snapshot if source_alias else source) if route else snapshot)
                self.assertEqual(gte.basis_transform_passes, current["basis_calls"] * 2)
                self.assertEqual(gte.rotations, 3 * (current["basis_calls"] - 1) if binding else 0)
