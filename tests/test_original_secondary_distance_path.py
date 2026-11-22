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


INPUT = struct.Struct("<26I")
OUTPUT = struct.Struct("<11I")
OBJECTS = (0x801E0000, 0x801E0100)
COMPONENTS = (0x801E0200, 0x801E0300)
BOUNDS = (0x801E0400, 0x801E0500)
BASIS = 0x801E0600
RESULT = 0x801E0700
TABLES = (0x801E0800, 0x801E0900)
ENTRIES = (0x801E0A00, 0x801E0B00)


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalSecondaryDistancePathTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        controller_route.OriginalControllerRouteTests.setUpClass()
        cls.load_address = controller_route.OriginalControllerRouteTests.load_address
        cls.payload = controller_route.OriginalControllerRouteTests.payload

    def fresh_cpu(self):
        return controller_route.OriginalControllerRouteTests.fresh_cpu(self)

    def call(self, cpu, address):
        return controller_route.OriginalControllerRouteTests.call(self, cpu, address)

    def test_distance_path_against_original(self):
        self.compare_distance_path(real_bounds=False)

    def test_distance_path_with_real_bounds_against_original(self):
        self.compare_distance_path(real_bounds=True)

    def test_distance_path_with_real_bounds_and_basis_against_original(self):
        self.compare_distance_path(real_bounds=True, real_basis=True)

    def test_distance_path_with_real_sequence_against_original(self):
        self.compare_distance_path(real_bounds=True, real_basis=True, real_sequence=True)

    def test_distance_path_with_real_transform_wrapper_against_original(self):
        self.compare_distance_path(real_bounds=True, real_basis=True, real_sequence=True, real_wrapper=True)

    def test_distance_path_with_real_point_transform_against_original(self):
        self.compare_distance_path(
            real_bounds=True, real_basis=True, real_sequence=True, real_wrapper=True, real_transform=True
        )

    def test_distance_path_with_real_rotation_dispatch_against_original(self):
        self.compare_distance_path(
            real_bounds=True, real_basis=True, real_sequence=True, real_wrapper=True,
            real_transform=True, real_rotation_dispatch=True,
        )

    def test_distance_path_with_modeled_gte_against_original(self):
        self.compare_distance_path(
            real_bounds=True, real_basis=True, real_sequence=True, real_wrapper=True,
            real_transform=True, real_rotation_dispatch=True, modeled_gte=True,
        )

    def test_distance_path_with_real_length_against_original(self):
        self.compare_distance_path(
            real_bounds=True, real_basis=True, real_sequence=True, real_wrapper=True,
            real_transform=True, real_rotation_dispatch=True, modeled_gte=True, real_length=True,
        )

    def test_distance_path_with_modeled_sqrt_against_original(self):
        self.compare_distance_path(
            real_bounds=True, real_basis=True, real_sequence=True, real_wrapper=True,
            real_transform=True, real_rotation_dispatch=True, modeled_gte=True,
            real_length=True, modeled_sqrt=True,
        )

    def test_distance_path_with_original_subtraction_against_original(self):
        self.compare_distance_path(
            real_bounds=True, real_basis=True, real_sequence=True, real_wrapper=True,
            real_transform=True, real_rotation_dispatch=True, modeled_gte=True,
            real_length=True, modeled_sqrt=True, original_subtraction=True,
        )

    def test_distance_path_with_identity_basis_against_original(self):
        self.compare_distance_path(
            real_bounds=True, real_basis=True, real_sequence=True, real_wrapper=True,
            real_transform=True, real_rotation_dispatch=True, modeled_gte=True,
            real_length=True, modeled_sqrt=True, original_subtraction=True, identity_basis=True,
        )

    def compare_distance_path(
        self, real_bounds, real_basis=False, real_sequence=False, real_wrapper=False,
        real_transform=False, real_rotation_dispatch=False, modeled_gte=False, real_length=False,
        modeled_sqrt=False, original_subtraction=False, identity_basis=False,
    ):
        generator = random.Random(0x590B458E7C)
        if original_subtraction:
            cases = [
                (alias, mutation, null_basis, generator.getrandbits(32),
                 *(generator.randint(-1000, 1000) & 0xFFFFFFFF for _ in range(22)))
                for alias, mutation, null_basis in itertools.product((0, 1), repeat=3)
                for _ in range(16)
            ]
        else:
            cases = [
                (alias, mutation, null_basis, *(generator.getrandbits(32) for _ in range(23)))
                for alias, mutation, null_basis in itertools.product((0, 1), repeat=3)
                for _ in range(16)
            ]
        sources = [
            "src/game/func_800590B4.c", "src/game/func_80058E7C.c",
            "src/game/func_80053EA0.c", "tests/test_secondary_distance_path.c",
        ]
        flags = []
        if real_bounds:
            sources.extend(["src/game/func_80058C74.c", "src/game/func_80052094.c"])
            flags.append("-DBUBSY3D_DISTANCE_REAL_BOUNDS")
        if real_basis:
            sources.extend([
                "src/game/func_8005476C.c", "src/game/func_800546F4.c",
                "src/game/func_80054488.c",
            ])
            flags.append("-DBUBSY3D_DISTANCE_REAL_BASIS")
        if real_sequence:
            sources.append("src/player_model/player_actor_flags.c")
            flags.append("-DBUBSY3D_DISTANCE_REAL_SEQUENCE")
        if real_wrapper:
            sources.append("src/game/func_80064528.c")
            flags.append("-DBUBSY3D_DISTANCE_REAL_WRAPPER")
        if real_transform:
            sources.extend(["src/game/func_800540BC.c", "src/game/func_80010038.c"])
            flags.append("-DBUBSY3D_DISTANCE_REAL_TRANSFORM")
        if real_rotation_dispatch:
            sources.append("src/game/func_80053FA4.c")
            flags.append("-DBUBSY3D_DISTANCE_REAL_ROTATION_DISPATCH")
        if modeled_gte:
            sources.append("src/game/component_transform.c")
            flags.append("-DBUBSY3D_DISTANCE_MODELED_GTE")
        if real_length:
            sources.extend(["src/game/func_80062D4C.c", "src/game/func_80053904.c"])
            flags.append("-DBUBSY3D_DISTANCE_REAL_LENGTH")
        if modeled_sqrt:
            sources.extend(["src/game/sqrt_backend_wrappers.c", "src/game/gte_sqrt_backends.c"])
            flags.append("-DBUBSY3D_DISTANCE_REAL_SQRT")
        if original_subtraction:
            flags.append("-DBUBSY3D_DISTANCE_ORIGINAL_SUBTRACTION")
        if identity_basis:
            flags.append("-DBUBSY3D_DISTANCE_IDENTITY_BASIS")
        native_outputs = run_native_probe(self, sources, cases, INPUT, OUTPUT, compiler_flags=flags)
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
            if address == 0x80058C74:
                if real_bounds:
                    return
                index = 0 if current["sampling"] == 0 or current["alias"] else 1
                self.assertEqual(a0, OBJECTS[index])
                write_words(uc, a1, read_words(uc, BOUNDS[index], 8))
                status = -1
            elif address == 0x800529F8 and real_bounds:
                index = 0 if current["sampling"] == 0 or current["alias"] else 1
                self.assertEqual(a0, OBJECTS[index])
                if real_sequence:
                    current["calls"] += 1
                    return
                write_words(uc, a1, (0,))
                status = 0
            elif address == 0x80010000:
                upper, lower = read_words(uc, a0, 3), read_words(uc, a1, 3)
                if real_length and current["sampling"] == 2:
                    current["first"], current["second"] = upper, lower
                if original_subtraction:
                    for left, right in zip(upper, lower):
                        left = left if left < 0x80000000 else left - 0x100000000
                        right = right if right < 0x80000000 else right - 0x100000000
                        self.assertGreaterEqual(left - right, -0x80000000)
                        self.assertLessEqual(left - right, 0x7FFFFFFF)
                    current["calls"] += 1
                    return
                write_words(uc, a2, tuple((a - b) & 0xFFFFFFFF for a, b in zip(upper, lower)))
                status = -2
            elif address == 0x80010038:
                if real_transform:
                    return
                lower, half = read_words(uc, a0, 3), read_words(uc, a1, 3)
                write_words(uc, a2, tuple((a + b) & 0xFFFFFFFF for a, b in zip(lower, half)))
                status = -3
            elif address == 0x8005476C:
                if real_basis:
                    return
                index = 0 if current["sampling"] == 0 or current["alias"] else 1
                self.assertEqual(a0, COMPONENTS[index])
                write_words(uc, a1, read_words(uc, a0, 8))
                status = -4
            elif address == 0x80053FA4 and identity_basis:
                if current["sampling"] == 0 and current["mutation"]:
                    index = 0 if current["alias"] else 1
                    offset = (32 if real_sequence else 0) + (20 if real_bounds else 4)
                    value = read_words(uc, BOUNDS[index] + offset, 1)[0]
                    write_words(uc, BOUNDS[index] + offset, (value ^ 0x20,))
                current["sampling"] += 1
                current["calls"] += 3
                return
            elif address == (
                0x80077298 if real_rotation_dispatch else
                0x80053FA4 if real_transform else 0x800540BC if real_wrapper else 0x80064528
            ):
                if real_basis:
                    current["calls"] += 2 if real_transform else 1
                center_address, basis_address = (a1, a0) if real_rotation_dispatch else (a0, a1)
                center = read_words(uc, center_address, 3)
                translation = read_words(uc, basis_address + 20, 3)
                if not modeled_gte:
                    if real_transform:
                        write_words(uc, a2, center if real_rotation_dispatch else center + (0,))
                    else:
                        write_words(uc, a2, tuple((a + b) & 0xFFFFFFFF for a, b in zip(center, translation)))
                if current["sampling"] == 0 and current["mutation"]:
                    index = 0 if current["alias"] else 1
                    offset = (32 if real_sequence else 0) + (20 if real_bounds else 4)
                    value = read_words(uc, BOUNDS[index] + offset, 1)[0]
                    write_words(uc, BOUNDS[index] + offset, (value ^ (0x20 if original_subtraction else 0x80000000),))
                current["sampling"] += 1
                if modeled_gte:
                    current["calls"] += 1
                    return
                status = a2 if real_rotation_dispatch else -5
            elif address == 0x80062D4C:
                if real_length:
                    return
                self.assertEqual(current["sampling"], 2)
                current["first"] = read_words(uc, a0, 3)
                current["second"] = read_words(uc, a1, 3)
                self.assertEqual(a2, RESULT)
                write_words(uc, a2, (current["result"],))
                status = -6
            elif address == 0x800535CC and real_length:
                if modeled_sqrt:
                    return
                expected = sum(
                    ((a - b) & 0xFFFFFFFF) ** 2 for a, b in zip(current["first"], current["second"])
                ) & 0xFFFFFFFF
                self.assertEqual(a0, expected)
                self.assertEqual(a1, RESULT)
                write_words(uc, a1, (current["result"],))
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFA)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                return
            else:
                return
            current["calls"] += 1
            uc.reg_write(UC_MIPS_REG_V0, status & 0xFFFFFFFF)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        if modeled_gte:
            gte = controller_route.RotationGteModel()
            cpu.hook_add(UC_HOOK_CODE, gte.hook)
        if real_rotation_dispatch:
            cpu.mem_write(physical(0x800A42CC), struct.pack("<9h H 3i", 4096, 0, 0, 0, 4096, 0, 0, 0, 4096, 0, 0, 0, 0))
        if modeled_sqrt:
            domain = tuple((index * 97 + 13) & 0x7FFF for index in range(512))
            cpu.mem_write(physical(controller_route.SQRT_LOOKUP_DOMAIN), struct.pack("<512h", *domain))
        for index, (entry, native) in enumerate(zip(cases, native_outputs)):
            alias, mutation, null_basis, result = entry[:4]
            current.update(alias=alias, mutation=mutation, result=result, calls=0, sampling=0)
            for i in range(2):
                cpu.mem_write(physical(OBJECTS[i]), bytes(32))
                write_words(cpu, OBJECTS[i] + 0x14, (COMPONENTS[i],))
                write_words(cpu, COMPONENTS[i], (0,) * 5 + entry[20 + i * 3:23 + i * 3])
                if modeled_gte:
                    rotation = (
                        (4096, 0, 0, 0, 4096, 0, 0, 0, 4096, 0) if identity_basis else
                        (0, 4096, 0, -4096, 0, 0, 0, 0, 4096, 0)
                    )
                    cpu.mem_write(
                        physical(COMPONENTS[i]),
                        struct.pack("<9hH", *rotation),
                    )
                record = list(entry[4 + i * 8:12 + i * 8])
                if real_bounds:
                    record[1], record[5] = (-record[5]) & 0xFFFFFFFF, (-record[1]) & 0xFFFFFFFF
                    write_words(cpu, OBJECTS[i] + 0x18, (TABLES[i],))
                    cpu.mem_write(physical(TABLES[i]), bytes(48))
                    write_words(cpu, TABLES[i], (BOUNDS[i],))
                    if real_sequence:
                        cpu.mem_write(physical(OBJECTS[i] + 8), struct.pack("<h", 1))
                        write_words(cpu, TABLES[i] + 4, (ENTRIES[i],))
                        write_words(cpu, TABLES[i] + 0x2C, (0 if null_basis else 4,))
                        write_words(cpu, ENTRIES[i], ((-2000) & 0xFFFFFFFF, 2001))
                cpu.mem_write(physical(BOUNDS[i]), bytes(64))
                write_words(cpu, BOUNDS[i] + (32 if real_sequence else 0), record)
            cpu.reg_write(UC_MIPS_REG_A0, OBJECTS[0])
            cpu.reg_write(UC_MIPS_REG_A1, OBJECTS[0] if alias else OBJECTS[1])
            cpu.reg_write(UC_MIPS_REG_A2, 0 if null_basis else BASIS)
            cpu.reg_write(UC_MIPS_REG_A3, RESULT)
            self.call(cpu, 0x800590B4)
            bounds_y = [
                read_words(cpu, BOUNDS[i] + (32 if real_sequence else 0) + (20 if real_bounds else 4), 1)[0]
                for i in range(2)
            ]
            if real_bounds:
                bounds_y = [(-value) & 0xFFFFFFFF for value in bounds_y]
            original = (
                cpu.reg_read(UC_MIPS_REG_V0) & 0xFFFFFFFF, current["calls"],
                *current["first"], *current["second"], read_words(cpu, RESULT, 1)[0],
                *bounds_y,
            )
            self.assertEqual(native, original, f"case {index}: {entry}")
        if modeled_gte:
            if identity_basis:
                self.assertEqual(gte.basis_transform_passes, 0)
            else:
                self.assertGreater(gte.basis_transform_passes, 0)
