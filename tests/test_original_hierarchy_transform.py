import itertools
import random
import struct
import unittest

from heading_native_probe import run_native_probe
import test_original_controller_route as controller_route
from test_original_controller_route import HAS_UNICORN, ORIGINAL, physical

if HAS_UNICORN:
    from unicorn import UC_HOOK_CODE
    from unicorn.mips_const import UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_PC, UC_MIPS_REG_RA


NODE_BASE = 0x801E0000
NODE_STRIDE = 0x80
OUTPUT_ADDRESS = 0x801E8000
CHAIN = 0x801D79B0
GENERATION = 0x801D8718
NODE_COUNT = 9
MAX_CHAIN = 8


def node_address(index):
    return NODE_BASE + NODE_STRIDE * index


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalHierarchyTransformTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_hierarchy_control_flow_against_original(self):
        generator = random.Random(0x73324)
        settings = []
        for count in range(1, 5):
            for generation, mutations in itertools.product((0, 1, 0xFFFFFFFF), range(4)):
                for versions in itertools.product(range(3), repeat=count):
                    settings.append((count, generation, mutations, versions))
        for _ in range(256):
            count = generator.randrange(5, MAX_CHAIN + 1)
            settings.append((count, generator.choice((0, 1, 0xFFFFFFFF)), generator.randrange(4),
                             tuple(generator.randrange(3) for _ in range(count))))

        cases = []
        for count, generation, mutations, choices in settings:
            versions = [generator.getrandbits(32) for _ in range(NODE_COUNT)]
            for index, choice in enumerate(choices):
                versions[index] = (0, generation, 0x12345678)[choice]
            cases.append((count, generation, mutations, *versions,
                          *(generator.getrandbits(32) for _ in range(2 * NODE_COUNT * 8 + 8))))
        self.compare_cases(cases, MAX_CHAIN)

    def test_hierarchy_uncached_sentinel_boundary_against_original(self):
        generator = random.Random(0x7332464)
        max_chain = 102
        node_count = max_chain + 1
        cases = []
        for count in (101, 102):
            for uncached, mutations in itertools.product((None, 99, 100, count - 1), range(4)):
                versions = [0x12345678] * node_count
                if uncached is not None:
                    versions[uncached] = 0
                cases.append((count, 7, mutations, *versions,
                              *(generator.getrandbits(32) for _ in range(2 * node_count * 8 + 8))))
        self.compare_cases(cases, max_chain)

    def compare_cases(self, cases, max_chain):
        node_count = max_chain + 1
        input_layout = struct.Struct(f"<{11 + node_count * 17}I")
        output_layout = struct.Struct(f"<{10 + 2 * max_chain + node_count * 9}I")
        native_outputs = run_native_probe(
            self, ["src/game/func_80073324.c", "tests/test_hierarchy_transform.c"],
            cases, input_layout, output_layout,
            compiler_flags=(f"-DHIERARCHY_TEST_MAX_CHAIN={max_chain}",),
        )
        cpu = self.fresh_cpu()
        current = {}

        def intercept(uc, address, size, data):
            if address != 0x800724B0:
                return
            a0 = uc.reg_read(UC_MIPS_REG_A0) & 0xFFFFFFFF
            a1 = uc.reg_read(UC_MIPS_REG_A1) & 0xFFFFFFFF
            self.assertEqual(a0, OUTPUT_ADDRESS)
            node = (a1 - NODE_BASE - 4) // NODE_STRIDE
            self.assertGreaterEqual(node, 0)
            self.assertLess(node, current["count"])
            self.assertEqual(a1, node_address(node) + 4)
            current["calls"].append(node)
            call_count = len(current["calls"])
            slots = struct.unpack(f"<{max_chain}I", uc.mem_read(physical(CHAIN), max_chain * 4))
            slot = slots.index(node_address(node))
            old = struct.unpack("<8I", uc.mem_read(physical(a0), 32))
            local = struct.unpack("<8I", uc.mem_read(physical(a1), 32))
            combined = tuple((left + right + call_count * 0x01010101) & 0xFFFFFFFF
                             for left, right in zip(old, local))
            uc.mem_write(physical(a0), struct.pack("<8I", *combined))
            if current["mutations"] & 1:
                generation, = struct.unpack("<I", uc.mem_read(physical(GENERATION), 4))
                uc.mem_write(physical(GENERATION), struct.pack("<I", generation ^ 0xA5A50000 ^ call_count))
            if current["mutations"] & 2:
                uc.mem_write(physical(CHAIN + slot * 4), struct.pack("<I", node_address(node_count - 1)))
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            count, generation, mutations = entry[:3]
            local_start = 3 + node_count
            cached_start = local_start + node_count * 8
            initial_start = cached_start + node_count * 8
            versions = entry[3:local_start]
            local = entry[local_start:cached_start]
            cached = entry[cached_start:initial_start]
            initial = entry[initial_start:initial_start + 8]
            current.update(count=count, mutations=mutations, calls=[])
            cpu.mem_write(physical(GENERATION), struct.pack("<I", generation))
            cpu.mem_write(physical(CHAIN), bytes(max_chain * 4))
            cpu.mem_write(physical(OUTPUT_ADDRESS), struct.pack("<8I", *initial))
            for node in range(node_count):
                parent = node_address(node + 1) if node + 1 < count else 0
                contents = struct.pack("<18I", versions[node], *local[node * 8:node * 8 + 8],
                                       *cached[node * 8:node * 8 + 8], 0)
                cpu.mem_write(physical(node_address(node)), contents + struct.pack("<I", parent))
            cpu.reg_write(UC_MIPS_REG_A0, node_address(0))
            cpu.reg_write(UC_MIPS_REG_A1, OUTPUT_ADDRESS)
            self.call(cpu, 0x80073324)
            final_generation, = struct.unpack("<I", cpu.mem_read(physical(GENERATION), 4))
            final_versions = tuple(struct.unpack("<I", cpu.mem_read(physical(node_address(node)), 4))[0]
                                   for node in range(node_count))
            final_cached = tuple(
                word for node in range(node_count)
                for word in struct.unpack("<8I", cpu.mem_read(physical(node_address(node) + 0x24), 32))
            )
            slots = struct.unpack(f"<{max_chain}I", cpu.mem_read(physical(CHAIN), max_chain * 4))
            chain = tuple((address - NODE_BASE) // NODE_STRIDE if address else 0xFFFFFFFF for address in slots)
            final_output = struct.unpack("<8I", cpu.mem_read(physical(OUTPUT_ADDRESS), 32))
            calls = current["calls"] + [0xFFFFFFFF] * (max_chain - len(current["calls"]))
            original = (len(current["calls"]), *calls, final_generation, *final_versions,
                        *final_cached, *chain, *final_output)
            with self.subTest(index=index, count=count, generation=generation, mutations=mutations):
                self.assertEqual(native, original)
                if max_chain == 102:
                    if count == 102 and versions[100] == 0 and versions[101] != 0:
                        self.assertEqual(len(current["calls"]), 0)
                        self.assertEqual(final_output, cached[:8])
                    elif versions[count - 1] == 0:
                        self.assertEqual(len(current["calls"]), count - 1)
                    elif count == 102 and versions[99] == 0 and versions[100] != 0:
                        self.assertEqual(len(current["calls"]), 100)
