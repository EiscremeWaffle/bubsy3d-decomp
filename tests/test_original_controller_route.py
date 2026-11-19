import importlib.util
import json
import random
import struct
import unittest

from tools.verify_original import ROOT, inspect_executable, verify_executable


HAS_UNICORN = importlib.util.find_spec("unicorn") is not None
ORIGINAL = ROOT / "original" / "usa" / "L0" / "L0.EXE"
if HAS_UNICORN:
    from unicorn import Uc, UC_ARCH_MIPS, UC_HOOK_CODE, UC_HOOK_MEM_WRITE, UC_MODE_LITTLE_ENDIAN, UC_MODE_MIPS32
    from unicorn.mips_const import (
        UC_MIPS_REG_0,
        UC_MIPS_REG_A0,
        UC_MIPS_REG_A1,
        UC_MIPS_REG_A2,
        UC_MIPS_REG_A3,
        UC_MIPS_REG_GP,
        UC_MIPS_REG_PC,
        UC_MIPS_REG_RA,
        UC_MIPS_REG_S0,
        UC_MIPS_REG_S1,
        UC_MIPS_REG_S3,
        UC_MIPS_REG_S7,
        UC_MIPS_REG_SP,
        UC_MIPS_REG_V0,
    )

GP = 0x800BDD5C
STACK = 0x801FF000
RETURN = 0x800F1000
CONFIG = 0x801D891C
BITMAP_STATE = 0x800BE468
MAPPING_ROW = 0x801D5F74
ROW_SELECTOR = 0x801D5F84
PENDING_EVENTS = 0x80182F78
EVENT_RECORD = PENDING_EVENTS + 4
QUEUE_UPDATE_FLAG = GP + 0x2C8
QUEUE_UPDATE_GATE = 0x80032D7C
NOOP_BACKENDS = (
    0x8006BB84, 0x800439EC, 0x80044268,
    0x8001B0A0, 0x80029164, 0x80032C8C, 0x80039AB4,
)
ROTATION_GENERATOR = 0x80077C50
ROTATION_TRIG_TABLE = 0x800A6558
SQRT_LOOKUP_DOMAIN = 0x800A5CA8
ROTATION_MULTIPLIER = 0x8007699C
ROTATION_CONVERTER = 0x800542A4
ROTATION_CROSS = 0x80010128
CYCLIC_ENTRIES = 0x8009E64C
TRACE = {
    0x8001B1E4: "queue processor",
    QUEUE_UPDATE_GATE: "queue-update gate",
    0x8001B8B8: "queue snapshot",
    0x80023E94: "controller callback",
    0x80036018: "heading handler",
    0x8004B35C: "grounded handler",
    0x80034EF8: "grounded turn",
    0x80024F2C: "movement consumer",
    0x800548A0: "rotation wrapper",
    ROTATION_GENERATOR: "rotation generator",
    ROTATION_MULTIPLIER: "rotation composition",
    ROTATION_CONVERTER: "rotation conversion",
    ROTATION_CROSS: "cross product",
}


def physical(address):
    return address & 0x1FFFFFFF


def write_word(cpu, address, value):
    cpu.mem_write(physical(address), struct.pack("<I", value))


def read_word(cpu, address):
    return struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]


def signed_halfword(value):
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


def rotation_generator_reference(angles, table):
    def trig(angle):
        angle = signed_halfword(angle)
        sine, cosine = struct.unpack_from("<hh", table, (abs(angle) & 0xFFF) * 4)
        return (-sine if angle < 0 else sine), cosine

    def product(left, right, negate=False):
        value = left * right
        if negate:
            value = -value
        value = ((value + 0x80000000) & 0xFFFFFFFF) - 0x80000000
        return value >> 12

    sx, cx = trig(angles[0])
    sy, cy = trig(angles[1])
    sz, cz = trig(angles[2])
    x_part = product(cz, -sy)
    z_part = product(sz, -sy)
    return tuple(value & 0xFFFF for value in (
        product(cz, cy),
        product(sz, cy, negate=True),
        sy,
        product(sz, cx) - product(x_part, sx),
        product(cz, cx) + product(z_part, sx),
        product(cy, sx, negate=True),
        product(sz, sx) + product(x_part, cx),
        product(cz, sx) - product(z_part, cx),
        product(cy, cx),
    ))


def leading_sign_bits(value):
    value &= 0xFFFFFFFF
    if value & 0x80000000:
        value = (~value) & 0xFFFFFFFF
    return 32 - value.bit_length()


def sqrt_backend_reference(value, domain, q12):
    value &= 0xFFFFFFFF
    leading = leading_sign_bits(value)
    if leading == 32:
        return 0
    even = leading & ~1
    signed_value = value if value < 0x80000000 else value - 0x100000000
    adjustment = even - 24
    if adjustment >= 0:
        normalized = (signed_value << adjustment) & 0xFFFFFFFF
        if normalized & 0x80000000:
            normalized -= 0x100000000
    else:
        normalized = signed_value >> -adjustment
    index = normalized + 256
    if not 0 <= index < 512:
        raise AssertionError(f"normalized square-root lookup escaped its domain: {index}")
    sample = struct.unpack_from("<h", domain, index * 2)[0] & 0xFFFFFFFF
    if q12:
        shift = 9 - even // 2
        return (sample << shift) & 0xFFFFFFFF if shift >= 0 else sample >> -shift
    return ((sample << (15 - even // 2)) & 0xFFFFFFFF) >> 12


def gte_rotate_vector(matrix, vector):
    if len(matrix) != 9 or len(vector) != 3:
        raise ValueError("GTE rotation requires a 3x3 matrix and a three-component vector")
    if any(value < -32768 or value > 32767 for value in (*matrix, *vector)):
        raise ValueError("GTE rotation operands must be signed halfwords")
    output = []
    for row in range(3):
        value = sum(matrix[row * 3 + column] * vector[column] for column in range(3)) >> 12
        output.append(max(-32768, min(32767, value)))
    return tuple(output)


def gte_basis_transform(matrix, vector):
    if len(matrix) != 9 or len(vector) != 3:
        raise ValueError("GTE basis transform requires a 3x3 matrix and a three-component vector")
    if any(value < -32768 or value > 32767 for value in matrix):
        raise ValueError("GTE basis matrix elements must be signed halfwords")
    if any(value < -0x80000000 or value > 0x7FFFFFFF for value in vector):
        raise ValueError("GTE basis vector elements must be signed words")

    high_parts = []
    low_parts = []
    for value in vector:
        if value < 0:
            magnitude = (-value) & 0xFFFFFFFF
            high_parts.append(signed_halfword(-(magnitude >> 15)))
            low_parts.append(signed_halfword(-(magnitude & 0x7FFF)))
        else:
            high_parts.append(signed_halfword(value >> 15))
            low_parts.append(signed_halfword(value & 0x7FFF))

    output = []
    for row in range(3):
        high_mac = sum(
            matrix[row * 3 + column] * high_parts[column]
            for column in range(3)
        ) & 0xFFFFFFFF
        low_mac = sum(
            matrix[row * 3 + column] * low_parts[column]
            for column in range(3)
        ) >> 12
        result = ((high_mac << 3) + low_mac) & 0xFFFFFFFF
        output.append(result - 0x100000000 if result & 0x80000000 else result)
    return tuple(output)


class RotationGteModel:
    """Model the observed MVMVA operations and leading-sign transfers."""

    def __init__(self):
        self.control = {}
        self.data = {}
        self.rotations = 0
        self.basis_transform_passes = 0
        self.leading_counts = 0

    def hook(self, cpu, address, size, data):
        if not (
            0x80076890 <= address < 0x80076AA8
            or 0x80077298 <= address < 0x800773F8
            or address in (0x800764D0, 0x800764DC, 0x80076554, 0x80076560)
        ):
            return
        word = struct.unpack("<I", cpu.mem_read(physical(address), 4))[0]
        opcode = word >> 26
        if opcode not in (0x12, 0x3A):
            return
        rs = (word >> 21) & 31
        rt = (word >> 16) & 31
        rd = (word >> 11) & 31
        if word == 0x4A486012:
            packed = b"".join(struct.pack("<I", self.control[index]) for index in range(5))
            matrix = struct.unpack("<10h", packed)[:9]
            vector = (
                signed_halfword(self.data[0]),
                signed_halfword(self.data[0] >> 16),
                signed_halfword(self.data[1]),
            )
            for index, value in enumerate(gte_rotate_vector(matrix, vector)):
                self.data[9 + index] = value & 0xFFFFFFFF
            self.rotations += 1
        elif word in (0x4A41E012, 0x4A49E012):
            packed = b"".join(struct.pack("<I", self.control[index]) for index in range(5))
            matrix = struct.unpack("<10h", packed)[:9]
            vector = tuple(signed_halfword(self.data[index]) for index in (9, 10, 11))
            shift = 12 if word == 0x4A49E012 else 0
            for row in range(3):
                value = sum(
                    matrix[row * 3 + column] * vector[column]
                    for column in range(3)
                ) >> shift
                self.data[25 + row] = value & 0xFFFFFFFF
            self.basis_transform_passes += 1
        elif opcode == 0x12 and rs == 6 and rd in range(5):
            self.control[rd] = cpu.reg_read(UC_MIPS_REG_0 + rt) & 0xFFFFFFFF
        elif opcode == 0x12 and rs == 4 and rd in (0, 1, 9, 10, 11, 30):
            value = cpu.reg_read(UC_MIPS_REG_0 + rt) & 0xFFFFFFFF
            self.data[rd] = value
            if rd == 30:
                self.data[31] = leading_sign_bits(value)
                self.leading_counts += 1
        elif opcode == 0x12 and rs == 0 and rd in (9, 10, 11, 25, 26, 27, 31):
            cpu.reg_write(UC_MIPS_REG_0 + rt, self.data[rd])
        elif opcode == 0x3A and rt == 11:
            target = cpu.reg_read(UC_MIPS_REG_0 + rs) + signed_halfword(word)
            cpu.mem_write(physical(target), struct.pack("<I", self.data[rt]))
        else:
            raise AssertionError(f"unsupported GTE instruction {word:#010x} at {address:#010x}")
        cpu.reg_write(UC_MIPS_REG_PC, address + 4)


class GteModelTests(unittest.TestCase):
    def test_leading_sign_count(self):
        for value in range(0x10000):
            for sample in (value, (~value) & 0xFFFFFFFF):
                sign = (sample >> 31) & 1
                expected = 0
                for bit in range(31, -1, -1):
                    if ((sample >> bit) & 1) != sign:
                        break
                    expected += 1
                self.assertEqual(leading_sign_bits(sample), expected)
        for bit in range(31):
            self.assertEqual(leading_sign_bits(1 << bit), 31 - bit)
            self.assertEqual(leading_sign_bits(~(1 << bit)), 31 - bit)
        self.assertEqual(leading_sign_bits(0x80000000), 1)

    def test_rotation_shift_and_saturation(self):
        identity = (4096, 0, 0, 0, 4096, 0, 0, 0, 4096)
        self.assertEqual(gte_rotate_vector(identity, (-32768, 0, 32767)), (-32768, 0, 32767))
        self.assertEqual(gte_rotate_vector((1, 0, 0, 0, 4096, 0, 0, 0, 4096), (-1, 2, 3)),
                         (-1, 2, 3))
        self.assertEqual(gte_rotate_vector((32767,) * 9, (32767,) * 3), (32767,) * 3)
        self.assertEqual(gte_rotate_vector((32767,) * 9, (-32768,) * 3), (-32768,) * 3)
        self.assertEqual(gte_rotate_vector((4096, 4096, 0, 4096, -4096, 0, 0, 0, 4096),
                                           (100, -200, 300)), (-100, 300, 300))
        with self.assertRaises(ValueError):
            gte_rotate_vector(identity, (32768, 0, 0))

    def test_unsupported_gte_operation_fails(self):
        class FakeCpu:
            def mem_read(self, address, size):
                return struct.pack("<I", 0x4A000001)

        with self.assertRaisesRegex(AssertionError, "unsupported GTE instruction"):
            RotationGteModel().hook(FakeCpu(), 0x8007699C, 4, None)


@unittest.skipUnless(HAS_UNICORN, "optional Unicorn package is not installed")
@unittest.skipUnless(ORIGINAL.is_file(), "verified original L0 executable is not present")
class OriginalControllerRouteTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        manifest = json.loads((ROOT / "config" / "executable-map.json").read_text())
        entry = next(item for item in manifest["executables"] if item["filename"] == "L0/L0.EXE")
        verify_executable(ORIGINAL, entry)
        original = ORIGINAL.read_bytes()
        cls.load_address = int(inspect_executable(original)["load_address"], 16)
        cls.payload = original[0x800:]

    def call(self, cpu, address):
        cpu.reg_write(UC_MIPS_REG_RA, RETURN)
        cpu.emu_start(address, RETURN, count=500000)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_PC), RETURN)

    def fresh_cpu(self):
        cpu = Uc(UC_ARCH_MIPS, UC_MODE_MIPS32 | UC_MODE_LITTLE_ENDIAN)
        cpu.mem_map(0, 0x200000)
        cpu.mem_write(physical(self.load_address), self.payload)
        cpu.reg_write(UC_MIPS_REG_GP, GP)
        cpu.reg_write(UC_MIPS_REG_SP, STACK)
        return cpu

    def poll(self, buttons):
        cpu = self.fresh_cpu()
        raw = (~buttons) & 0xFFFF
        cpu.mem_write(physical(0x8018CA48), bytes((0, 0x41, raw >> 8, raw & 0xFF, 0, 0, 0, 0)))
        cpu.mem_write(physical(0x8018CA50), bytes((0xFF, 0, 0, 0, 0, 0, 0, 0)))
        cpu.mem_write(physical(0x800BE2E5), b"\0")
        self.call(cpu, 0x8002278C)
        table = read_word(cpu, 0x801D36F4)
        self.assertEqual(table, 0x8009EC8C)
        self.assertEqual(read_word(cpu, table + 3 * 12 + 4), 0x80023E94)
        self.assertEqual(read_word(cpu, table + 3 * 12 + 8), 0x80023A0C)
        self.call(cpu, 0x8001A6E0)
        self.assertEqual(read_word(cpu, PENDING_EVENTS), 1)
        event = struct.unpack("<HHiiIBB", cpu.mem_read(physical(EVENT_RECORD), 18))
        self.assertEqual(event, (3, 0, 0xFFFE, 0xFFFE, buttons, 1, 0))
        return cpu

    def activate_heading_table(self, cpu):
        write_word(cpu, 0x800BE698, CONFIG)
        cpu.reg_write(UC_MIPS_REG_A0, CONFIG)
        self.call(cpu, 0x800195DC)
        self.call(cpu, 0x80027F08)
        self.assertEqual(read_word(cpu, 0x801D36F8), 0x800979B0)
        self.assertEqual(read_word(cpu, 0x800979C0), 0x80036018)

    def test_physical_pad_to_turn_events_without_stubs(self):
        for row in range(4):
            for bit in (13, 15):
                for active in (True, False):
                    with self.subTest(row=row, bit=bit, active=active):
                        cpu = self.poll((1 << bit) if active else 0)
                        self.activate_heading_table(cpu)
                        write_word(cpu, MAPPING_ROW, row)
                        cpu.mem_write(physical(ROW_SELECTOR), b"\0\0")
                        write_word(cpu, BITMAP_STATE, 0 if active else 1 << bit)
                        cpu.reg_write(UC_MIPS_REG_A0, EVENT_RECORD)
                        self.call(cpu, 0x80023E94)
                        self.assertEqual(read_word(cpu, PENDING_EVENTS), 2)
                        event = struct.unpack("<HHii", cpu.mem_read(physical(EVENT_RECORD + 28), 12))
                        event_id = (8 if bit == 13 else 10) + (0 if active else 1)
                        self.assertEqual(event, (event_id, 4, 0xFFFE, 0))
                        self.assertEqual(read_word(cpu, BITMAP_STATE), (1 << bit) if active else 0)

    def test_controller_bitmap_dispatch_loop(self):
        cpu = self.fresh_cpu()
        rng = random.Random(0x23E94)
        press_address, release_address = 0x8009EEF0, 0x8009EFF0
        stack = STACK - 0x120
        events = []
        mutation_enabled = False

        def intercept(uc, address, size, data):
            if address != 0x8001B558:
                return
            arguments = tuple(
                uc.reg_read(register)
                for register in (UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3)
            )
            arguments += struct.unpack("<4I", uc.mem_read(physical(uc.reg_read(UC_MIPS_REG_SP) + 0x10), 16))
            events.append((arguments, read_word(uc, BITMAP_STATE)))
            if mutation_enabled and len(events) == 1:
                write_word(uc, BITMAP_STATE, read_word(uc, BITMAP_STATE) | (1 << 5))
                uc.mem_write(physical(ROW_SELECTOR), struct.pack("<h", 1))
                write_word(uc, MAPPING_ROW + 4, 2)
                write_word(uc, press_address + (2 * 16 + 4) * 4, 0x1F1)
                write_word(uc, release_address + (2 * 16 + 5) * 4, 0x222)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for sample in range(513):
            if sample == 0:
                buttons, bitmap, selector, rows = 0x11, 0x5A5A0000, 0, [0, 0, 0, 0]
                pressed = [[0] * 16 for _ in range(4)]
                released = [[0] * 16 for _ in range(4)]
                pressed[0][0] = 0x100
                mutation_enabled = True
            else:
                buttons = rng.getrandbits(32)
                bitmap = rng.getrandbits(32)
                selector = rng.randrange(2)
                rows = [rng.randrange(4) for _ in range(4)]
                pressed = [[rng.choice((0, 0, 0x100, rng.getrandbits(32))) for _ in range(16)]
                           for _ in range(4)]
                released = [[rng.choice((0, 0, 0x100, rng.getrandbits(32))) for _ in range(16)]
                            for _ in range(4)]
                mutation_enabled = bool(sample & 1)

            expected_bitmap, expected_selector = bitmap, selector
            expected_rows = rows.copy()
            expected_pressed = [row.copy() for row in pressed]
            expected_released = [row.copy() for row in released]
            expected_events = []
            for bit in range(16):
                mask = 1 << bit
                if buttons & mask:
                    expected_bitmap |= mask
                    table = expected_pressed
                elif expected_bitmap & mask:
                    expected_bitmap &= ~mask
                    table = expected_released
                else:
                    continue
                event = table[expected_rows[expected_selector]][bit]
                if event == 0:
                    continue
                expected_events.append(((event & 0xFF, 4, 0xFFFE, 0, 0, 0, 0, 0), expected_bitmap))
                if mutation_enabled and len(expected_events) == 1:
                    expected_bitmap |= 1 << 5
                    expected_selector = 1
                    expected_rows[1] = 2
                    expected_pressed[2][4] = 0x1F1
                    expected_released[2][5] = 0x222

            cpu.mem_write(physical(stack), b"\0" * 0x120)
            write_word(cpu, stack + 0x118, RETURN)
            cpu.reg_write(UC_MIPS_REG_SP, stack)
            cpu.reg_write(UC_MIPS_REG_S3, buttons)
            cpu.reg_write(UC_MIPS_REG_S7, 0)
            write_word(cpu, BITMAP_STATE, bitmap)
            cpu.mem_write(physical(ROW_SELECTOR), struct.pack("<h", selector))
            cpu.mem_write(physical(MAPPING_ROW), struct.pack("<4i", *rows))
            cpu.mem_write(physical(press_address), struct.pack("<64I", *(value for row in pressed for value in row)))
            cpu.mem_write(physical(release_address), struct.pack("<64I", *(value for row in released for value in row)))
            events.clear()
            with self.subTest(sample=sample, mutation=mutation_enabled):
                self.call(cpu, 0x80024A70)
                self.assertEqual(events, expected_events)
                self.assertEqual(read_word(cpu, BITMAP_STATE), expected_bitmap)
                self.assertEqual(struct.unpack("<h", cpu.mem_read(physical(ROW_SELECTOR), 2))[0],
                                 expected_selector)
                self.assertEqual(struct.unpack("<4i", cpu.mem_read(physical(MAPPING_ROW), 16)),
                                 tuple(expected_rows))
                self.assertEqual(struct.unpack("<64I", cpu.mem_read(physical(press_address), 256)),
                                 tuple(value for row in expected_pressed for value in row))
                self.assertEqual(struct.unpack("<64I", cpu.mem_read(physical(release_address), 256)),
                                 tuple(value for row in expected_released for value in row))

    def test_grounded_component_and_bit_helpers(self):
        cpu = self.fresh_cpu()
        component, output = 0x801E5000, 0x801E5100
        sentinel = bytes(range(48))
        for value in range(256):
            cpu.mem_write(physical(component), sentinel)
            cpu.mem_write(physical(component + 4), bytes((value,)))
            write_word(cpu, output, 0xDEADBEEF)
            cpu.reg_write(UC_MIPS_REG_A0, component)
            cpu.reg_write(UC_MIPS_REG_A1, output)
            self.call(cpu, 0x80052AD4)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)
            self.assertEqual(read_word(cpu, output), value & 3)
            self.assertEqual(cpu.mem_read(physical(component + 4), 1)[0], value)

        cpu.mem_write(physical(component), sentinel)
        cpu.reg_write(UC_MIPS_REG_A0, component)
        cpu.reg_write(UC_MIPS_REG_A1, component + 4)
        self.call(cpu, 0x80052AD4)
        self.assertEqual(read_word(cpu, component + 4), sentinel[4] & 3)

        rng = random.Random(0x532CC)
        values = (0, 1, 0x7F, 0x80, 0xFF, 0x100, 0x7FFF, 0x8000,
                  0xFFFF, 0x10000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        cases = [(a, b, c, f) for a in values for b in values for c in values
                 for f in (0, 0x80000000, 0xFFFFFFFF)]
        cases.extend(tuple(rng.getrandbits(32) for _ in range(4)) for _ in range(5000))
        for first, second, third, flags in cases:
            cpu.mem_write(physical(component), sentinel)
            cpu.reg_write(UC_MIPS_REG_A0, component)
            cpu.reg_write(UC_MIPS_REG_A1, first)
            cpu.reg_write(UC_MIPS_REG_A2, second)
            cpu.reg_write(UC_MIPS_REG_A3, third)
            write_word(cpu, STACK + 0x10, flags)
            expected = (flags | (second << 16) | (signed_halfword(first) << 8) |
                        signed_halfword(third)) & 0xFFFFFFFF
            self.call(cpu, 0x800532CC)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), expected)
            self.assertEqual(read_word(cpu, component + 0x24), expected)
            self.assertEqual(cpu.mem_read(physical(component), 0x24), sentinel[:0x24])
            self.assertEqual(cpu.mem_read(physical(component + 0x28), 8), sentinel[0x28:])

        for selector in (*range(256), 0x100, 0x1FF, 0x10000, 0xFFFFFFFF):
            for state in (0, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA):
                write_word(cpu, component + 4, state)
                write_word(cpu, component + 0x10, state ^ 0xFFFFFFFF)
                for address, word in ((0x80022674, state), (0x80022604, state ^ 0xFFFFFFFF)):
                    cpu.reg_write(UC_MIPS_REG_A0, component)
                    cpu.reg_write(UC_MIPS_REG_A1, selector)
                    self.call(cpu, address)
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), (word >> (selector & 31)) & 1)
                self.assertEqual(read_word(cpu, component + 4), state)
                self.assertEqual(read_word(cpu, component + 0x10), state ^ 0xFFFFFFFF)

    def test_grounded_auxiliary_and_global_helpers(self):
        cpu = self.fresh_cpu()
        record_count = 0x801D8994
        auxiliary_base = 0x801D0E8C
        values = (0, 1, 7, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFD, 0xFFFFFFFF)
        for count in values:
            write_word(cpu, record_count, count)
            self.call(cpu, 0x80028D48)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), count)
            for index in values:
                cpu.reg_write(UC_MIPS_REG_A0, index)
                self.call(cpu, 0x80040FC0)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0),
                                 (auxiliary_base + ((index - count) & 0xFFFFFFFF) * 0x34) & 0xFFFFFFFF)

        case_object = 0x800BE104
        pointers = (0, 0x801E1000, 0x801E2000)
        for current in pointers:
            for expected in pointers:
                for replacement in pointers:
                    write_word(cpu, case_object, current)
                    cpu.reg_write(UC_MIPS_REG_A0, expected)
                    cpu.reg_write(UC_MIPS_REG_A1, replacement)
                    self.call(cpu, 0x80037B14)
                    self.assertEqual(read_word(cpu, case_object),
                                     replacement if current == expected else current)

        for pointer in pointers:
            cpu.reg_write(UC_MIPS_REG_A0, pointer)
            self.call(cpu, 0x80053270)
            self.assertEqual(read_word(cpu, 0x800BE33C), pointer)
            self.call(cpu, 0x8005327C)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), pointer)

        rng = random.Random(0x691AC)
        for value in (*values, *(rng.getrandbits(32) for _ in range(1024))):
            write_word(cpu, GP + 0x6C0, 0xA5A51234)
            cpu.reg_write(UC_MIPS_REG_A0, value)
            self.call(cpu, 0x800691AC)
            self.assertEqual(read_word(cpu, GP + 0x6C0), ((value & 0xFFFF) << 16) | 0x1234)

    def test_level_record_position_lookup(self):
        cpu = self.fresh_cpu()
        records, positions, output = 0x801E6000, 0x801E6100, 0x801E6200
        samples = (
            (0, 0, 0, 0),
            (-1, 1, -0x80000000, 0x7FFFFFFF),
            (123, -456, 789, -111),
            (0x12345678, -99, 55, -0x1234567),
        )
        write_word(cpu, 0x801D89A4, records)
        for index, position in enumerate(samples):
            write_word(cpu, records + index * 0x24 + 0x0C, positions + index * 16)
            cpu.mem_write(physical(positions + index * 16), struct.pack("<4i", *position))

        for count in (-1, 0, 1, 4):
            write_word(cpu, 0x801D8994, count & 0xFFFFFFFF)
            for index in (-0x80000000, -1, 0, 1, 3, 4, 0x7FFFFFFF):
                cpu.mem_write(physical(output), b"\xA5" * 20)
                cpu.reg_write(UC_MIPS_REG_A0, index & 0xFFFFFFFF)
                cpu.reg_write(UC_MIPS_REG_A1, output)
                self.call(cpu, 0x80028B5C)
                valid = 0 <= index < count
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), int(valid))
                expected = struct.pack("<4i", *samples[index]) if valid else b"\xA5" * 16
                self.assertEqual(cpu.mem_read(physical(output), 20), expected + b"\xA5" * 4)

        write_word(cpu, 0x801D8994, len(samples))
        for index, position in enumerate(samples):
            cpu.reg_write(UC_MIPS_REG_A0, index)
            cpu.reg_write(UC_MIPS_REG_A1, positions + index * 16)
            self.call(cpu, 0x80028B5C)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)
            self.assertEqual(cpu.mem_read(physical(positions + index * 16), 16),
                             struct.pack("<4i", *position))

    def test_level_record_rotation_lookup(self):
        cpu = self.fresh_cpu()
        records, other_records = 0x801E6000, 0x801E6300
        positions, rotations, output = 0x801E6400, 0x801E6500, 0x801E6600
        position_values = (
            (0, 1, -1, 123),
            (-0x80000000, -7, 0x7FFFFFFF, 456),
            (123, -456, 789, -111),
            (0x12345678, -99, 55, -0x1234567),
        )
        rotation_values = tuple(
            tuple((record * 0x1111 + element * 0x1234) & 0xFFFF for element in range(9))
            for record in range(4)
        )
        default = bytes(cpu.mem_read(physical(0x8009F120), 20))
        write_word(cpu, 0x801D89A4, records)
        for index, position in enumerate(position_values):
            entry = records + index * 0x24
            write_word(cpu, entry + 8, 0 if index == 1 else rotations + index * 0x20)
            write_word(cpu, entry + 0x0C, positions + index * 16)
            cpu.mem_write(physical(rotations + index * 0x20),
                          struct.pack("<9H", *rotation_values[index]))
            cpu.mem_write(physical(positions + index * 16), struct.pack("<4i", *position))

        for count in (-1, 0, 1, 4):
            write_word(cpu, 0x801D8994, count & 0xFFFFFFFF)
            for index in (-0x80000000, -1, 0, 1, 2, 3, 4, 0x7FFFFFFF):
                cpu.mem_write(physical(output), b"\xA5" * 36)
                cpu.reg_write(UC_MIPS_REG_A0, index & 0xFFFFFFFF)
                cpu.reg_write(UC_MIPS_REG_A1, output)
                self.call(cpu, 0x800289E8)
                valid = 0 <= index < count
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), int(valid))
                if valid:
                    matrix = (default if index == 1 else
                              struct.pack("<9H", *rotation_values[index]) + b"\xA5" * 2)
                    expected = matrix + struct.pack("<3i", *position_values[index][:3])
                else:
                    expected = b"\xA5" * 32
                self.assertEqual(cpu.mem_read(physical(output), 36), expected + b"\xA5" * 4)

        cpu = self.fresh_cpu()
        alternate_rotation = tuple(0xF000 + element for element in range(9))
        alternate_position = (101, 202, 303, 404)
        write_word(cpu, 0x801D8994, 4)
        write_word(cpu, records + 8, rotations)
        write_word(cpu, records + 0x0C, positions)
        cpu.mem_write(physical(rotations), struct.pack("<9H", *rotation_values[0]))
        cpu.mem_write(physical(positions), struct.pack("<4i", *position_values[0]))
        write_word(cpu, other_records + 8, rotations + 0x80)
        write_word(cpu, other_records + 0x0C, positions + 0x80)
        cpu.mem_write(physical(rotations + 0x80), struct.pack("<9H", *alternate_rotation))
        cpu.mem_write(physical(positions + 0x80), struct.pack("<4i", *alternate_position))
        reads = []

        def switch_table(uc, address, size, data):
            if address == 0x80028A9C:
                write_word(uc, 0x801D89A4, other_records if len(reads) & 1 else records)
                reads.append(address)

        write_word(cpu, 0x801D89A4, records)
        hook = cpu.hook_add(UC_HOOK_CODE, switch_table)
        cpu.mem_write(physical(output), b"\xA5" * 36)
        cpu.reg_write(UC_MIPS_REG_A0, 0)
        cpu.reg_write(UC_MIPS_REG_A1, output)
        self.call(cpu, 0x800289E8)
        cpu.hook_del(hook)
        self.assertEqual(len(reads), 9)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)
        expected_rotation = tuple(
            alternate_rotation[index] if index & 1 else rotation_values[0][index]
            for index in range(9)
        )
        expected = (struct.pack("<9H", *expected_rotation) + b"\xA5" * 2
                    + struct.pack("<3i", *position_values[0][:3]) + b"\xA5" * 4)
        self.assertEqual(cpu.mem_read(physical(output), 36), expected)

    def test_move_request_object_reset(self):
        cpu = self.fresh_cpu()
        object_address = 0x801E5000
        first, second, replacement = 0x801E5100, 0x801E5200, 0x801E5300
        calls = []
        mutation = 0

        def intercept(uc, address, size, data):
            if address != 0x80028600:
                return
            calls.append((uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)))
            self.assertEqual(uc.mem_read(physical(object_address + 0x2C), 1), b"\0")
            for offset in (4, 8, 0x10, 0x14):
                self.assertEqual(read_word(uc, object_address + offset), 0)
            if mutation and len(calls) == 1:
                write_word(uc, object_address + 0x24, replacement if mutation == 1 else 0)
                write_word(uc, object_address + 0x18, 0x87654321)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for first_value in (0, first):
            for second_value in (0, second):
                for mutation in (0, 1, 2):
                    cpu.mem_write(physical(object_address), b"\xA5" * 0x34)
                    write_word(cpu, object_address + 0x18, 0x12345678)
                    write_word(cpu, object_address + 0x20, first_value)
                    write_word(cpu, object_address + 0x24, second_value)
                    cpu.reg_write(UC_MIPS_REG_A0, object_address)
                    calls.clear()
                    self.call(cpu, 0x80028698)
                    expected_second = (replacement if mutation == 1 else 0) if mutation else second_value
                    expected_calls = [(object_address, first_value)]
                    if expected_second:
                        expected_calls.append((object_address, expected_second))
                    self.assertEqual(calls, expected_calls)
                    self.assertEqual(read_word(cpu, object_address + 0x1C), first_value)
                    self.assertEqual(read_word(cpu, object_address + 0x24), expected_second)
                    self.assertEqual(read_word(cpu, object_address + 0x18),
                                     0x87654321 if mutation else 0x12345678)
                    self.assertEqual(read_word(cpu, object_address + 0x28), 0xA5A5A5A5)
                    self.assertEqual(read_word(cpu, object_address + 0x30), 0xA5A5A5A5)

    def test_move_request_component_reset_backend(self):
        cpu = self.fresh_cpu()
        object_address, component = 0x801E5000, 0x801E5100
        original_transform, replacement_transform = 0x801E5200, 0x801E5300
        position = (-0x80000000, -7, 0x7FFFFFFF, 123)
        rotation = struct.pack("<9HH3i", 0, 1, 0x7FFF, 0x8000, 0xFFFF, 5, 6, 7, 8,
                               0x1234, -0x80000000, 0, 0x7FFFFFFF)
        callbacks = []
        targets = (0x80028B5C, 0x80054C7C, 0x800289E8, 0x800549E8, 0x800288D8)
        initial_index = 0
        mutation = False

        def intercept(uc, address, size, data):
            if address not in targets:
                return
            callbacks.append(address)
            a0 = uc.reg_read(UC_MIPS_REG_A0)
            a1 = uc.reg_read(UC_MIPS_REG_A1)
            if address == 0x80028B5C:
                self.assertEqual(a0, initial_index)
                uc.mem_write(physical(a1), struct.pack("<4i", *position))
                if mutation:
                    write_word(uc, component + 0x14, replacement_transform)
                    write_word(uc, object_address + 0x0C, 0xFFFFFFFE)
            elif address == 0x80054C7C:
                self.assertEqual(a0, replacement_transform if mutation else original_transform)
                self.assertEqual(a1, 0)
                a2 = uc.reg_read(UC_MIPS_REG_A2)
                self.assertEqual(struct.unpack("<3i", uc.mem_read(physical(a2), 12)), position[:3])
                if mutation:
                    write_word(uc, object_address + 0x0C, 0xFFFFFFFD)
            elif address == 0x800289E8:
                self.assertEqual(a0, 0xFFFFFFFD if mutation else initial_index)
                uc.mem_write(physical(a1), rotation)
                if mutation:
                    write_word(uc, component + 0x14, original_transform)
                    write_word(uc, object_address + 0x0C, 0xFFFFFFFC)
            elif address == 0x800549E8:
                self.assertEqual(a0, replacement_transform if mutation else original_transform)
                self.assertEqual(uc.mem_read(physical(a1), len(rotation)), rotation)
                if mutation:
                    write_word(uc, object_address + 0x0C, 0xFFFFFFFB)
            else:
                self.assertEqual(a0, 0xFFFFFFFB if mutation else initial_index)
                if mutation:
                    flags = uc.mem_read(physical(component + 4), 1)[0]
                    uc.mem_write(physical(component + 4), bytes((flags ^ 0x5A,)))
                    write_word(uc, component + 0x10, 0xDEADBEEF)
            uc.reg_write(UC_MIPS_REG_V0, 0)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for initial_index in (0, 7, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF):
            for mutation in (False, True):
                for flags in range(256):
                    cpu.mem_write(physical(object_address), b"\xA5" * 0x34)
                    cpu.mem_write(physical(component), b"\xA5" * 0x18)
                    write_word(cpu, object_address + 0x0C, initial_index)
                    cpu.mem_write(physical(component + 4), bytes((flags,)))
                    write_word(cpu, component + 0x10, 0x12345678)
                    write_word(cpu, component + 0x14, original_transform)
                    cpu.reg_write(UC_MIPS_REG_A0, object_address)
                    cpu.reg_write(UC_MIPS_REG_A1, component)
                    callbacks.clear()
                    self.call(cpu, 0x80028600)
                    self.assertEqual(callbacks, list(targets))
                    self.assertEqual(read_word(cpu, component + 0x10), 0)
                    self.assertEqual(cpu.mem_read(physical(component + 4), 1)[0],
                                     (flags ^ (0x5A if mutation else 0)) & 0xDF)
                    self.assertEqual(read_word(cpu, component + 0x14), original_transform)
                    self.assertEqual(read_word(cpu, object_address + 0x0C),
                                     0xFFFFFFFB if mutation else initial_index)
                    self.assertEqual(cpu.mem_read(physical(component), 4), b"\xA5" * 4)
                    self.assertEqual(cpu.mem_read(physical(component + 5), 11), b"\xA5" * 11)

    def test_move_request_record_reset(self):
        cpu = self.fresh_cpu()
        context, records = 0x801E5000, 0x801E6000
        object_address, component, transform = 0x801E6200, 0x801E6300, 0x801E6400
        fallback_object, fallback_component = 0x801E6500, 0x801E6600
        fallback_transform = 0x801E6700
        count_address, records_address = 0x801D8994, 0x801D89A4
        translation = (0x80000000, 0x12345678, 0xFFFFFFFF)
        callbacks = []
        index = 0
        count_on_context = None
        shrink_count = False
        backend_status = 0

        def intercept(uc, address, size, data):
            if address == 0x8002893C and shrink_count:
                write_word(uc, count_address, 0)
            if address not in (0x80026648, 0x80040FC0, 0x80054B20, 0x8001B558):
                return
            callbacks.append(address)
            a0 = uc.reg_read(UC_MIPS_REG_A0)
            a1 = uc.reg_read(UC_MIPS_REG_A1)
            a2 = uc.reg_read(UC_MIPS_REG_A2)
            if address == 0x80026648:
                self.assertEqual(a0, index & 0xFFFFFFFF)
                if count_on_context is not None:
                    write_word(uc, count_address, count_on_context)
                result = context
            elif address == 0x80040FC0:
                self.assertEqual(a0, index & 0xFFFFFFFF)
                result = fallback_object
            elif address == 0x80054B20:
                self.assertEqual(a0, fallback_transform if shrink_count else transform)
                self.assertEqual(a1, 0)
                uc.mem_write(physical(a2), struct.pack("<3I", *translation))
                result = backend_status
            else:
                self.assertEqual((a0, a1, a2, uc.reg_read(UC_MIPS_REG_A3)),
                                 (0x1A, 4, 0xFFFE, 0xFFFE))
                stack = uc.reg_read(UC_MIPS_REG_SP)
                arguments = [read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C)]
                self.assertEqual(read_word(uc, arguments[0]), 11)
                self.assertEqual(arguments[2], arguments[1] + 4)
                self.assertEqual(arguments[3], arguments[1] + 8)
                self.assertEqual(tuple(read_word(uc, pointer) for pointer in arguments[1:]), translation)
                result = 0
            uc.reg_write(UC_MIPS_REG_V0, result)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cases = (
            (-1, 3, None, 1, False, 0),
            (-0x80000000, 3, None, 1, False, 0),
            (3, 3, None, 1, False, 0),
            (0x7FFFFFFF, 3, None, 1, False, 0),
            (0, 3, None, 0, False, 0),
            (1, 3, None, 1, False, 0),
            (2, 3, None, 1, True, 0),
            (1, 0, 3, 1, False, 0x18),
        )
        for index, initial_count, count_on_context, enabled, shrink_count, backend_status in cases:
            with self.subTest(index=index, count=initial_count, enabled=enabled,
                              shrink=shrink_count, status=backend_status):
                cpu.mem_write(physical(records), b"\xA5" * (3 * 0x24))
                write_word(cpu, count_address, initial_count)
                write_word(cpu, records_address, records)
                cpu.mem_write(physical(context + 0x0C), bytes((enabled,)))
                write_word(cpu, object_address + 0x1C, component)
                write_word(cpu, component + 0x14, transform)
                write_word(cpu, fallback_object + 0x1C, fallback_component)
                write_word(cpu, fallback_component + 0x14, fallback_transform)
                for record_index in range(3):
                    write_word(cpu, records + record_index * 0x24 + 0x1C, object_address)
                before = bytes(cpu.mem_read(physical(records), 3 * 0x24))
                callbacks.clear()
                cpu.reg_write(UC_MIPS_REG_A0, index & 0xFFFFFFFF)
                self.call(cpu, 0x800288D8)
                effective_count = initial_count if count_on_context is None else count_on_context
                valid = 0 <= index < effective_count
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), int(valid))
                expected = bytearray(before)
                if valid:
                    expected[index * 0x24 + 0x20] = 0
                self.assertEqual(cpu.mem_read(physical(records), len(expected)), expected)
                expected_callbacks = [0x80026648]
                if valid and shrink_count:
                    expected_callbacks.append(0x80040FC0)
                if valid and enabled:
                    expected_callbacks.extend((0x80054B20, 0x8001B558))
                self.assertEqual(callbacks, expected_callbacks)

    def test_heading_callback_event_and_context_gates(self):
        cpu = self.fresh_cpu()
        event_byte, environment, context_gate = 0x800BE020, 0x801D36F0, 0x800BDFC7
        for value in range(256):
            cpu.mem_write(physical(event_byte), bytes((value,)))
            self.call(cpu, 0x8002E824)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), value)
        for setter, expected in ((0x8002E7D8, 0), (0x8002E7FC, 1)):
            self.call(cpu, setter)
            self.call(cpu, 0x8002E824)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), expected)

        calls = []

        def intercept(uc, address, size, data):
            if address != 0x800556D0:
                return
            calls.append(tuple(uc.reg_read(register) for register in (
                UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3,
            )))
            uc.mem_write(physical(context_gate), b"\xD3")
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for kind in (-32768, -1, 0, 18, 19, 20, 32767):
            for value in (0, 1, 255):
                with self.subTest(kind=kind, value=value):
                    cpu.mem_write(physical(environment), struct.pack("<h", kind))
                    cpu.mem_write(physical(context_gate), bytes((value,)))
                    calls.clear()
                    self.call(cpu, 0x8002B620)
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), value if kind == 19 else 0xD3)
                    expected = [] if kind == 19 else [(0, 0x80012640, 0x8001239C, 0x8D7)]
                    self.assertEqual(calls, expected)

    def test_heading_entry_gates_and_live_callback_mutations(self):
        cases = (
            (18, 1, 0, 1, 0, 8, 2, 1, False, 0, 1, True, 0, 1),
            (19, 1, 0, 0, 0, 8, 2, 2, False, 0, 0, True, 1, 0),
            (19, 1, 0, 1, 0, 8, 2, 1, True, 46, 1, True, 3, 0),
            (20, 1, 1, 1, 1, 46, 4, 4, False, 8, 0, False, 2, 0),
            (19, 1, 0, 0, 1, 8, 0, 0, False, 0, 0, True, 0, 0),
            (19, 2, 0, 0, 0, 8, 0, 0, False, 0, 0, True, 0, 0),
            (19, 1, 1, 0, 0, 8, 0, 0, False, 0, 0, False, 0, 0),
            (19, 1, 0, 0, 0x80000000, 46, 0, 0, True, 0, 0, True, 1, 0),
        )
        event, environment, context_gate = 0x801E5000, 0x801D36F0, 0x800BDFC7
        stop, active, condition = 0x8018647B, 0x80186463, 0x8009F58C
        counter, previous, handles = 0x800BDE98, 0x800BE0EC, 0x800BE68C

        for case in cases:
            (kind, gate, stopped, active_value, word, event_id, current, old, activate,
             replacement_event, changed, proceed, expected_count, expected_active) = case
            with self.subTest(case=case):
                cpu = self.fresh_cpu()
                cpu.mem_write(physical(event), struct.pack("<H", event_id))
                cpu.mem_write(physical(environment), struct.pack("<h", kind))
                cpu.mem_write(physical(context_gate), bytes((gate,)))
                cpu.mem_write(physical(stop), bytes((stopped,)))
                cpu.mem_write(physical(active), bytes((active_value,)))
                for address, value in ((condition, word), (counter, current), (previous, old),
                                       (handles, 0x11111111), (handles + 4, 0x22222222)):
                    write_word(cpu, address, value)
                calls, reached = [], []

                def intercept(uc, address, size, data):
                    if address == 0x8004733C:
                        calls.append((address, uc.reg_read(UC_MIPS_REG_A0)))
                        self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 1)
                        if activate:
                            uc.mem_write(physical(stop), b"\1")
                        uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                    elif address == 0x80052BDC:
                        handle = uc.reg_read(UC_MIPS_REG_A0)
                        calls.append((address, handle))
                        self.assertEqual(uc.reg_read(UC_MIPS_REG_A1), 1)
                        if handle == 0x11111111:
                            write_word(uc, handles + 4, 0x33333333)
                            uc.mem_write(physical(event), struct.pack("<H", replacement_event))
                        else:
                            self.assertEqual(handle, 0x33333333)
                        uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))
                    elif address in (0x8003611C, 0x80036950):
                        reached.append(address == 0x8003611C)
                        uc.emu_stop()

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_A0, event)
                cpu.reg_write(UC_MIPS_REG_RA, RETURN)
                cpu.emu_start(0x80036018, RETURN, count=10000)
                self.assertEqual(reached, [proceed])
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_S1), changed)
                self.assertEqual(read_word(cpu, previous), current)
                self.assertEqual(cpu.mem_read(physical(active), 1), bytes((expected_active,)))
                self.assertEqual(len(calls), expected_count)

    def test_default_grounded_routing_gates(self):
        cpu = self.poll(0)
        actor, state = 0x801E1000, 0x801E5000
        reached = []
        targets = {0x80036920: "grounded", 0x8003693C: "swim", 0x80036950: "skip"}

        def trace(uc, address, size, data):
            if address in targets:
                reached.append(targets[address])
                uc.emu_stop()

        cpu.hook_add(UC_HOOK_CODE, trace)
        cpu.reg_write(UC_MIPS_REG_S0, actor)
        cpu.reg_write(UC_MIPS_REG_S3, state)
        for runtime in (0, 4, 5, 255):
            for state_id in (8, 0x24, 0x25, 0x26):
                for auxiliary in (0, 0xFFFE, -2):
                    for mode in (0, 1):
                        with self.subTest(runtime=runtime, state_id=state_id, auxiliary=auxiliary, mode=mode):
                            cpu.mem_write(physical(state), struct.pack("<H", state_id))
                            write_word(cpu, state + 4, auxiliary & 0xFFFFFFFF)
                            cpu.mem_write(physical(0x80186461), bytes((runtime,)))
                            cpu.mem_write(physical(0x800BE548), bytes((mode,)))
                            allowed = runtime != 5 or state_id == 0x25 or (
                                state_id == 0x26 and auxiliary != 0xFFFE
                            )
                            expected = ("grounded" if mode == 0 else "swim") if allowed else "skip"
                            reached.clear()
                            cpu.emu_start(0x80036874, RETURN, count=1000)
                            self.assertEqual(reached, [expected])

        cpu.mem_write(physical(0x80186461), b"\0")
        for address, value, size in (
            (actor + 0x10, 0x100000, 4), (actor + 4, 0x100, 4),
            (0x80186479, 1, 1), (0x80186454, 1, 2),
        ):
            with self.subTest(gate=hex(address)):
                cpu.mem_write(physical(address), value.to_bytes(size, "little"))
                reached.clear()
                cpu.emu_start(0x80036874, RETURN, count=1000)
                self.assertEqual(reached, ["skip"])
                cpu.mem_write(physical(address), bytes(size))

        write_word(cpu, actor + 0x10, 0x10000)
        reached.clear()
        cpu.emu_start(0x80036874, RETURN, count=1000)
        self.assertEqual(reached, ["swim"])

    def test_controller_reference_gate_uses_signed_comparison(self):
        cases = (
            (0, 99, 0), (0, 100, 0), (0, 101, 1),
            (0x7FFFFFCD, 0x80000040, 1),
            (0xFFFFFFFF, 0x80000000, 0),
            (0xFFFFFF9C, 0, 0), (0xFFFFFF9C, 1, 1),
        )
        for reference, sample, expected in cases:
            with self.subTest(reference=hex(reference), sample=hex(sample)):
                cpu = self.poll(1 << 13)
                self.activate_heading_table(cpu)
                cpu.mem_write(physical(0x801D8AD0), b"\1")
                write_word(cpu, 0x801D8AD4, reference)
                write_word(cpu, 0x800BE648, sample)
                comparisons = []

                def trace(uc, address, size, data):
                    if address == 0x800240AC:
                        comparisons.append(uc.reg_read(UC_MIPS_REG_V0))
                        uc.emu_stop()

                cpu.hook_add(UC_HOOK_CODE, trace)
                cpu.reg_write(UC_MIPS_REG_A0, EVENT_RECORD)
                cpu.reg_write(UC_MIPS_REG_RA, RETURN)
                cpu.emu_start(0x80023E94, RETURN, count=500000)
                self.assertEqual(comparisons, [expected])

    def test_cyclic_entry_lookup_and_overlap(self):
        cpu = self.fresh_cpu()
        cursor, output = 0x801E5000, 0x801E6000
        rows = [[0x12340000 + row * 16 + word for word in range(4)] for row in range(7)]
        writes = []

        def trace_write(uc, access, address, size, value, data):
            address = physical(address)
            if address == physical(cursor) or physical(output) <= address < physical(output + 16):
                writes.append((address, size))

        cpu.hook_add(UC_HOOK_MEM_WRITE, trace_write)
        cpu.mem_write(physical(CYCLIC_ENTRIES), struct.pack("<28I", *(word for row in rows for word in row)))
        for index in (-0x80000000, -1, 0, 1, 2, 3, 4, 5, 6, 7, 0x7FFFFFFF):
            with self.subTest(index=index):
                initial = bytes([0xA5] * 16)
                write_word(cpu, cursor, index & 0xFFFFFFFF)
                cpu.mem_write(physical(output), initial)
                writes.clear()
                cpu.reg_write(UC_MIPS_REG_A0, cursor)
                cpu.reg_write(UC_MIPS_REG_A1, output)
                self.call(cpu, 0x8001A280)
                valid = 0 <= index < 7
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), int(valid))
                self.assertEqual(read_word(cpu, cursor), (index + 1) % 7 if valid else index & 0xFFFFFFFF)
                expected = struct.pack("<4I", *rows[index]) if valid else initial
                self.assertEqual(bytes(cpu.mem_read(physical(output), 16)), expected)
                expected_writes = [(physical(output + offset), 4) for offset in range(0, 16, 4)]
                expected_writes.append((physical(cursor), 4))
                self.assertEqual(writes, expected_writes if valid else [])

        for slot in range(4):
            for value in (0, 5, 6, 7, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF):
                with self.subTest(overlap_slot=slot, replacement=hex(value)):
                    source = rows[2].copy()
                    source[slot] = value
                    cpu.mem_write(physical(CYCLIC_ENTRIES + 2 * 16), struct.pack("<4I", *source))
                    cpu.mem_write(physical(output), bytes(16))
                    cursor = output + slot * 4
                    write_word(cpu, cursor, 2)
                    writes.clear()
                    cpu.reg_write(UC_MIPS_REG_A0, cursor)
                    cpu.reg_write(UC_MIPS_REG_A1, output)
                    self.call(cpu, 0x8001A280)
                    next_value = (value + 1) & 0xFFFFFFFF
                    source[slot] = 0 if 7 <= next_value < 0x80000000 else next_value
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)
                    self.assertEqual(bytes(cpu.mem_read(physical(output), 16)), struct.pack("<4I", *source))
                    expected_writes = [(physical(output + offset), 4) for offset in range(0, 16, 4)]
                    expected_writes.append((physical(cursor), 4))
                    self.assertEqual(writes, expected_writes)

    def test_cyclic_entry_lookup_rereads_cursor(self):
        cursor, output = 0x801E5000, 0x801E6000
        rows = [[0x12340000 + row * 16 + word for word in range(4)] for row in range(7)]
        cases = (
            (0x8001A2B8, 0xFFFFFFFF, 0, 0xFFFFFFFF, None),
            (0x8001A2E8, 6, 1, 0, 6),
            (0x8001A36C, 0x7FFFFFFF, 1, 0x80000000, 0),
        )
        for at, replacement, result, final_cursor, row in cases:
            with self.subTest(reload=hex(at)):
                cpu = self.fresh_cpu()
                cpu.mem_write(physical(CYCLIC_ENTRIES), struct.pack(
                    "<28I", *(word for entry in rows for word in entry)
                ))
                cpu.mem_write(physical(output), bytes([0xA5] * 16))
                write_word(cpu, cursor, 0)
                visits = []

                def mutate(uc, address, size, data):
                    if address == at:
                        visits.append(address)
                        write_word(uc, cursor, replacement)

                cpu.hook_add(UC_HOOK_CODE, mutate)
                cpu.reg_write(UC_MIPS_REG_A0, cursor)
                cpu.reg_write(UC_MIPS_REG_A1, output)
                self.call(cpu, 0x8001A280)
                self.assertEqual(visits, [at])
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), result)
                self.assertEqual(read_word(cpu, cursor), final_cursor)
                expected = bytes([0xA5] * 16) if row is None else struct.pack("<4I", *rows[row])
                self.assertEqual(bytes(cpu.mem_read(physical(output), 16)), expected)

    def test_texture_color_payload_overlap_and_real_dispatch(self):
        cpu = self.fresh_cpu()
        payload, color = 0x801E5000, 0x801E6000
        cpu.reg_write(UC_MIPS_REG_A0, 0)
        cpu.reg_write(UC_MIPS_REG_A1, 0)
        self.call(cpu, 0x80055EA8)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0x16)
        for offset in range(38):
            with self.subTest(offset=offset):
                initial = bytes((index * 37 + offset) & 0xFF for index in range(40))
                expected = bytearray(initial)
                expected[0x14] = expected[offset]
                expected[0x15] = expected[offset + 1]
                expected[0x16] = expected[offset + 2]
                cpu.mem_write(physical(payload), initial)
                cpu.reg_write(UC_MIPS_REG_A0, payload)
                cpu.reg_write(UC_MIPS_REG_A1, payload + offset)
                self.call(cpu, 0x80055EA8)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)
                self.assertEqual(bytes(cpu.mem_read(physical(payload), 40)), expected)

        loaded, slots = 0x80189C71, 0x80189C90
        initial = bytearray([0xA5] * (50 * 44))
        for index in range(50):
            initial[index * 44] = 0xFF
        initial[0] = initial[49 * 44] = 3
        initial[4 + 0x13:4 + 0x16] = b"\x22\x33\x44"
        cpu.mem_write(physical(slots), bytes(initial))
        cpu.mem_write(physical(loaded), b"\1")
        cpu.reg_write(UC_MIPS_REG_A0, 3)
        cpu.reg_write(UC_MIPS_REG_A1, slots + 4 + 0x13)
        self.call(cpu, 0x800482D0)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)
        self.assertEqual(bytes(cpu.mem_read(physical(slots + 4 + 0x14), 3)), b"\x22" * 3)
        self.assertEqual(bytes(cpu.mem_read(physical(slots + 49 * 44 + 4 + 0x14), 3)), b"\x22" * 3)
        self.assertEqual(bytes(cpu.mem_read(physical(slots + 44 + 4 + 0x14), 3)), b"\xA5" * 3)

    def test_texture_color_dispatch_callback_mutations(self):
        loaded, slots, color = 0x80189C71, 0x80189C90, 0x801E6000
        cases = ((0, 1, True), (2, 0, False), (1, None, True))
        for initial_flag, replacement, should_scan in cases:
            with self.subTest(initial_flag=initial_flag, replacement=replacement):
                cpu = self.fresh_cpu()
                initial = bytearray([0xA5] * (50 * 44))
                for index in range(50):
                    initial[index * 44] = 0xFF
                initial[0] = 3
                cpu.mem_write(physical(slots), bytes(initial))
                cpu.mem_write(physical(loaded), bytes([initial_flag]))
                cpu.mem_write(physical(color), b"\1\2\3")
                calls = []

                def intercept(uc, address, size, data):
                    if address == 0x800556D0:
                        line = uc.reg_read(UC_MIPS_REG_A3)
                        calls.append(("assert", line))
                        if line == 0x19C and replacement is not None:
                            uc.mem_write(physical(loaded), bytes([replacement]))
                    elif address == 0x80055EA8:
                        pointer = uc.reg_read(UC_MIPS_REG_A0)
                        calls.append(("visit", pointer))
                        self.assertEqual(uc.reg_read(UC_MIPS_REG_A1), color)
                        if pointer == slots + 4:
                            uc.mem_write(physical(loaded), b"\0")
                            uc.mem_write(physical(slots + 49 * 44), b"\3")
                        uc.reg_write(UC_MIPS_REG_V0, 1)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.reg_write(UC_MIPS_REG_A0, 0x103)
                cpu.reg_write(UC_MIPS_REG_A1, color)
                self.call(cpu, 0x800482D0)
                expected = [] if initial_flag == 1 else [("assert", 0x19C)]
                if should_scan:
                    expected.extend([
                        ("visit", slots + 4), ("assert", 0x1A8),
                        ("visit", slots + 49 * 44 + 4), ("assert", 0x1A8),
                    ])
                self.assertEqual(calls, expected)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), int(should_scan))

    def test_controller_result_selection_and_reload(self):
        context, previous = 0x801E5000, GP + 0x280
        duration = 0x89ABCDEF
        for selected in (-32768, -1, 0, 1, 32767):
            for changed in (False, True):
                with self.subTest(selected=selected, changed=changed):
                    cpu = self.fresh_cpu()
                    cpu.mem_write(physical(context + 0x0C), b"\1")
                    cpu.mem_write(physical(context + 0x14), struct.pack("<h", selected))
                    prior = selected ^ 1 if changed else selected
                    replacement = selected ^ 2
                    cpu.mem_write(physical(previous), struct.pack("<h", prior))
                    calls = []

                    def intercept(uc, address, size, data):
                        if address == 0x80026648:
                            calls.append(address)
                            self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 0x1234)
                            self.assertEqual(uc.reg_read(UC_MIPS_REG_A1), 0x5678)
                            uc.reg_write(UC_MIPS_REG_V0, context)
                        elif address == 0x8002DF24:
                            calls.append(address)
                            self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 0x8001273C if selected == 0 else 0x80012730)
                            uc.mem_write(physical(context + 0x14), struct.pack("<h", replacement))
                        elif address == 0x8002C604:
                            calls.append((address, uc.reg_read(UC_MIPS_REG_A0)))
                        elif address == 0x8002BD0C:
                            calls.append(address)
                            self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 17)
                            uc.reg_write(UC_MIPS_REG_V0, duration)
                        elif address == 0x8002CA88:
                            calls.append(address)
                            self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 17)
                            self.assertEqual(uc.reg_read(UC_MIPS_REG_A1), duration)
                        elif address in (0x8002E1E0, 0x8002E348):
                            calls.append(address)
                            self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 0)
                        else:
                            return
                        uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                    cpu.hook_add(UC_HOOK_CODE, intercept)
                    cpu.reg_write(UC_MIPS_REG_A0, 0x1234)
                    cpu.reg_write(UC_MIPS_REG_A1, 0x5678)
                    self.call(cpu, 0x8002C370)
                    expected = [0x80026648]
                    if changed:
                        expected.append(0x8002DF24)
                    expected.extend([
                        (0x8002C604, 6), (0x8002C604, 7), 0x8002BD0C,
                        0x8002CA88, 0x8002E1E0, 0x8002E348,
                    ])
                    self.assertEqual(calls, expected)
                    self.assertEqual(
                        struct.unpack("<h", cpu.mem_read(physical(previous), 2))[0],
                        replacement if changed else selected,
                    )

    def test_controller_result_other_modes(self):
        context, enabled = 0x801E5000, 0x8009F58C
        for mode in (0, 2, 255):
            for runtime in (0, 1, 2, 255):
                for active in (False, True):
                    with self.subTest(mode=mode, runtime=runtime, active=active):
                        cpu = self.fresh_cpu()
                        cpu.mem_write(physical(context + 0x0C), bytes([mode]))
                        write_word(cpu, enabled, 0 if active else 1)
                        calls = []

                        def intercept(uc, address, size, data):
                            if address == 0x80026648:
                                calls.append(address)
                                uc.reg_write(UC_MIPS_REG_V0, context)
                            elif address in (0x8002BC7C, 0x8002C010, 0x8002C1F0, 0x8002BC9C):
                                calls.append(address)
                                if address == 0x8002C1F0:
                                    write_word(uc, enabled, 0x80000000 if active else 0)
                            elif address == 0x800283E0:
                                calls.append(address)
                                uc.reg_write(UC_MIPS_REG_V0, runtime)
                            elif address == 0x8002CA88:
                                value = uc.reg_read(UC_MIPS_REG_A0)
                                calls.append((address, value))
                                self.assertEqual(uc.reg_read(UC_MIPS_REG_A1), 0xFFFFFFFF)
                            else:
                                return
                            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                        cpu.hook_add(UC_HOOK_CODE, intercept)
                        self.call(cpu, 0x8002C370)
                        expected = [0x80026648, 0x8002BC7C, 0x8002C010, 0x8002C1F0]
                        if active:
                            expected.append((0x8002CA88, 5))
                        expected.append(0x800283E0)
                        if runtime == 1:
                            expected.extend((0x8002BC9C, (0x8002CA88, 15)))
                        self.assertEqual(calls, expected)

    def test_tick_history_counters_and_callbacks(self):
        reference, snapshot, count, total, index = (GP + offset for offset in (0x5A0, 0x5A4, 0x5A8, 0x5AC, 0x5B0))
        sample_address, history = GP + 0x858, 0x8018CA58
        cases = (
            (0, 0, 0, 100, 159, 0),
            (98, 3, 5, 100, 160, 1),
            (99, 7, 9, 100, 161, 0x1234ABCD),
            (100, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFE1, 29, 0x100),
            (0xFFFFFFFF, 5, 12, 0, 0xFFFFFFFF, 0xFF),
            (12, 0, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, 0x10001),
        )
        for initial_index, initial_count, initial_total, initial_reference, sample, result in cases:
            with self.subTest(index=initial_index, sample=hex(sample)):
                cpu = self.fresh_cpu()
                cpu.mem_write(physical(history), bytes([0xA5] * 128 * 4))
                for address, value in (
                    (reference, initial_reference), (snapshot, 0x98765432),
                    (count, initial_count), (total, initial_total),
                    (index, initial_index), (sample_address, sample),
                ):
                    write_word(cpu, address, value)
                expected_index = (initial_index + 1) & 0xFFFFFFFF
                if expected_index == 100:
                    expected_index = 0
                incremented = (initial_count + 1) & 0xFFFFFFFF
                update = ((sample - initial_reference) & 0xFFFFFFFF) >= 60
                expected_reference = sample if update else initial_reference
                expected_snapshot = incremented if update else 0x98765432
                expected_count = 0 if update else incremented
                calls = []

                def intercept(uc, address, size, data):
                    if address == 0x8006C8C0:
                        calls.append(address)
                        self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 1)
                        self.assertEqual(read_word(uc, reference), expected_reference)
                        self.assertEqual(read_word(uc, snapshot), expected_snapshot)
                        self.assertEqual(read_word(uc, count), expected_count)
                        self.assertEqual(read_word(uc, total), (initial_total + 1) & 0xFFFFFFFF)
                        self.assertEqual(read_word(uc, index), expected_index)
                        self.assertEqual(read_word(uc, history + expected_index * 4), sample)
                        write_word(uc, count, 0xABCDEF01)
                    elif address == 0x800720FC:
                        calls.append(address)
                        self.assertEqual(read_word(uc, count), 0xABCDEF01)
                        write_word(uc, snapshot, 0xF1234567)
                    elif address == 0x80051360:
                        calls.append(address)
                        self.assertEqual(read_word(uc, snapshot), 0xF1234567)
                        uc.reg_write(UC_MIPS_REG_V0, result)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                self.call(cpu, 0x8004F688)
                self.assertEqual(calls, [0x8006C8C0, 0x800720FC, 0x80051360])
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), result & 0xFF)
                self.assertEqual(read_word(cpu, count), 0xABCDEF01)
                self.assertEqual(read_word(cpu, snapshot), 0xF1234567)
                for slot in range(128):
                    self.assertEqual(read_word(cpu, history + slot * 4), sample if slot == expected_index else 0xA5A5A5A5)

    def test_tick_history_rereads_sample_after_history_write(self):
        cpu = self.fresh_cpu()
        reference, snapshot, count, total, index = (GP + offset for offset in (0x5A0, 0x5A4, 0x5A8, 0x5AC, 0x5B0))
        sample, history = GP + 0x858, 0x8018CA58
        for address, value in ((reference, 100), (snapshot, 8), (count, 2), (total, 0), (index, 0), (sample, 150)):
            write_word(cpu, address, value)
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x8004F6DC:
                self.assertEqual(read_word(uc, history + 4), 150)
                write_word(uc, sample, 161)
            elif address in (0x8006C8C0, 0x800720FC, 0x80051360):
                calls.append(address)
                if address == 0x80051360:
                    uc.reg_write(UC_MIPS_REG_V0, 1)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        self.call(cpu, 0x8004F688)
        self.assertEqual(calls, [0x8006C8C0, 0x800720FC, 0x80051360])
        self.assertEqual(read_word(cpu, history + 4), 150)
        self.assertEqual(read_word(cpu, reference), 161)
        self.assertEqual(read_word(cpu, snapshot), 3)
        self.assertEqual(read_word(cpu, count), 0)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)

    def test_frame_buffer_selection_and_live_callbacks(self):
        index_address, sequence_address = 0x801D871C, 0x801D8718
        values_address, selected_address, entries_address = 0x801D053C, 0x801D36A0, 0x801D2EC0
        cases = ((0, 0, 0), (1, 1, 9), (1, -3, 0xFFFFFFFF), (0, 2, 0x7FFFFFFF))
        values = (0x0123, 0x4567, 0x89AB, 0xCDEF)

        for selected_index, after_sync, sequence in cases:
            with self.subTest(index=selected_index, after_sync=after_sync, sequence=hex(sequence)):
                cpu = self.fresh_cpu()
                cpu.mem_write(physical(values_address), struct.pack("<4H", *values))
                cpu.mem_write(physical(index_address), struct.pack("<h", selected_index))
                cpu.mem_write(physical(selected_address), b"\0" * 4)
                cpu.mem_write(physical(entries_address), b"\xA5" * (32 * 40))
                write_word(cpu, sequence_address, 0x87654321)
                calls, sequence_writes, entry_writes = [], [], []

                def intercept(uc, address, size, data):
                    if address == 0x8006D434:
                        calls.append(address)
                        self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), selected_address)
                        self.assertEqual(
                            struct.unpack("<2H", uc.mem_read(physical(selected_address), 4)),
                            (values[selected_index], values[2 + selected_index]),
                        )
                        uc.mem_write(physical(selected_address), struct.pack("<H", 0x1357))
                        uc.mem_write(physical(index_address), struct.pack("<h", 4))
                    elif address == 0x8006CCC4:
                        calls.append(address)
                        self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 1)
                        self.assertEqual(struct.unpack("<h", uc.mem_read(physical(index_address), 2))[0], 4)
                        uc.mem_write(physical(index_address), struct.pack("<h", after_sync))
                    elif address == 0x8007201C:
                        calls.append(address)
                        self.assertEqual(
                            struct.unpack("<h", uc.mem_read(physical(index_address), 2))[0],
                            1 if after_sync == 0 else 0,
                        )
                        write_word(uc, sequence_address, 0xAABBCCDD)
                        uc.mem_write(physical(entries_address + 5 * 40), struct.pack("<H", 0x3456))
                    elif address == 0x80071F00:
                        calls.append(address)
                        self.assertEqual(read_word(uc, sequence_address), 0xAABBCCDD)
                        self.assertEqual(
                            struct.unpack("<H", uc.mem_read(physical(entries_address + 5 * 40), 2))[0],
                            0x3456,
                        )
                        write_word(uc, sequence_address, sequence)
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                def record_write(uc, access, address, size, value, data):
                    if address == physical(sequence_address):
                        sequence_writes.append((size, value))
                    if physical(entries_address) <= address < physical(entries_address + 32 * 40):
                        entry_writes.append((address - physical(entries_address), size, value))

                cpu.hook_add(UC_HOOK_CODE, intercept)
                cpu.hook_add(UC_HOOK_MEM_WRITE, record_write)
                self.call(cpu, 0x800720FC)
                self.assertEqual(calls, [0x8006D434, 0x8006CCC4, 0x8007201C, 0x80071F00])
                self.assertEqual(
                    struct.unpack("<2H", cpu.mem_read(physical(selected_address), 4)),
                    (0x1357, values[2 + selected_index]),
                )
                self.assertEqual(read_word(cpu, sequence_address), (sequence + 1) & 0xFFFFFFFF)
                self.assertEqual(sequence_writes, [(4, (sequence + 1) & 0xFFFFFFFF)] * 2)
                self.assertEqual(entry_writes, [(entry * 40, 2, 0) for entry in range(31, -1, -1)])
                expected_entries = (b"\0\0" + b"\xA5" * 38) * 32
                self.assertEqual(cpu.mem_read(physical(entries_address), 32 * 40), expected_entries)

    def test_queue_snapshot_preserves_priority_and_remaining_records(self):
        cpu = self.poll(0)
        source, destination = 0x801E6000, 0x801E7000
        records = (
            (0, 11, 2), (2, 12, 0), (5, 13, 4), (7, 14, 2), (9, 15, 7)
        )
        write_word(cpu, source, len(records))
        for slot, event_id, mode in records:
            data = struct.pack("<HHii4I", event_id, mode, 100 + slot, 200 + slot, slot, 1, 2, 3)
            cpu.mem_write(physical(source + 4 + 28 * slot), data)
        cpu.reg_write(UC_MIPS_REG_A0, source)
        cpu.reg_write(UC_MIPS_REG_A1, destination)
        self.call(cpu, 0x8001B8B8)
        self.assertEqual(read_word(cpu, source), 1)
        self.assertEqual(read_word(cpu, destination), 4)
        for index, slot in enumerate((2, 0, 7, 5)):
            original = next(row for row in records if row[0] == slot)
            expected = struct.pack("<HHii4I", original[1], original[2], 100 + slot, 200 + slot, slot, 1, 2, 3)
            actual = bytes(cpu.mem_read(physical(destination + 4 + 28 * index), 28))
            self.assertEqual(actual, expected)
            self.assertEqual(bytes(cpu.mem_read(physical(source + 4 + 28 * slot), 2)), b"\0\0")
        self.assertEqual(struct.unpack("<H", cpu.mem_read(physical(source + 4 + 28 * 9), 2))[0], 15)

    def test_queue_reset_clears_only_ids_and_counts(self):
        cpu = self.fresh_cpu()
        source, destination = 0x80182F78, 0x80183A6C
        queue_size = 4 + 100 * 28
        rng = random.Random(0x1B9C8)
        writes = []

        def trace_write(uc, access, address, size, value, data):
            address = physical(address)
            if physical(source) <= address < physical(destination) + queue_size:
                writes.append((address, size, value))

        cpu.hook_add(UC_HOOK_MEM_WRITE, trace_write)
        expected_writes = []
        for slot in range(100):
            expected_writes.extend((
                (physical(source + 4 + 28 * slot), 2, 0),
                (physical(destination + 4 + 28 * slot), 2, 0),
            ))
        expected_writes.extend(((physical(source), 4, 0), (physical(destination), 4, 0)))

        for iteration in range(8):
            with self.subTest(iteration=iteration):
                initial = bytes(rng.getrandbits(8) for _ in range(2 * queue_size))
                expected = bytearray(initial)
                for base in (0, queue_size):
                    expected[base:base + 4] = b"\0" * 4
                    for slot in range(100):
                        position = base + 4 + 28 * slot
                        expected[position:position + 2] = b"\0\0"
                cpu.mem_write(physical(source), initial)
                writes.clear()
                self.call(cpu, 0x8001B9C8)
                self.assertEqual(bytes(cpu.mem_read(physical(source), len(expected))), expected)
                self.assertEqual(writes, expected_writes)

    def test_transform_reset_snapshot(self):
        cpu = self.fresh_cpu()
        status, snapshot, descriptor = 0x801CBED0, 0x801CBED4, 0x801CBF20
        region, region_size = snapshot - 32, 128
        separate_source = 0x801E5000
        rng = random.Random(0x51678)
        expected = bytearray()
        callbacks = []
        writes = []

        def intercept(uc, address, size, data):
            if address != 0x800731C0:
                return
            callbacks.append(address)
            self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), descriptor)
            self.assertEqual(read_word(uc, status), 0)
            self.assertEqual(bytes(uc.mem_read(physical(region), region_size)), expected)
            write_word(uc, status, 0x12345678)
            write_word(uc, snapshot, 0xABCDEF01)
            uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        def trace_write(uc, access, address, size, value, data):
            address = physical(address)
            if physical(region) <= address < physical(region) + region_size:
                writes.append((address, size))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.hook_add(UC_HOOK_MEM_WRITE, trace_write)
        for offset in (None, -4, -3, -2, -1, 0, 1, 2, 3, 4):
            with self.subTest(source_offset=offset):
                initial = bytes(rng.getrandbits(8) for _ in range(region_size))
                outside = bytes(rng.getrandbits(8) for _ in range(32))
                source = separate_source if offset is None else snapshot + offset * 4
                cpu.mem_write(physical(region), initial)
                cpu.mem_write(physical(separate_source), outside)
                write_word(cpu, GP + 0x888, source)
                expected = bytearray(initial)
                expected[status - region:status - region + 4] = b"\0" * 4
                for first, count in ((0, 3), (3, 3), (6, 2)):
                    source_offset = source - region + first * 4
                    values = (
                        outside[first * 4:(first + count) * 4]
                        if offset is None else expected[source_offset:source_offset + count * 4]
                    )
                    destination_offset = snapshot - region + first * 4
                    expected[destination_offset:destination_offset + count * 4] = values
                callbacks.clear()
                writes.clear()
                self.call(cpu, 0x80051678)
                self.assertEqual(callbacks, [0x800731C0])
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)
                self.assertEqual(writes, [(physical(status), 4)] +
                                 [(physical(snapshot + index * 4), 4) for index in range(8)])
                expected[status - region:status - region + 4] = struct.pack("<I", 0x12345678)
                expected[snapshot - region:snapshot - region + 4] = struct.pack("<I", 0xABCDEF01)
                self.assertEqual(bytes(cpu.mem_read(physical(region), region_size)), expected)

    def test_deferred_event_copy_appends_seven_words(self):
        cpu = self.fresh_cpu()
        queue = 0x80182F78
        source = 0x801E5000
        record = (0x1234, 0x5678, 0x89ABCDEF, 0xFEDCBA98,
                  0x10203040, 0x50607080, 0x90A0B0C0)
        cpu.mem_write(physical(source), struct.pack("<7I", *record))
        write_word(cpu, queue, 2)
        cpu.mem_write(physical(queue + 4 + 28 * 0), b"\x11" * 28)
        cpu.mem_write(physical(queue + 4 + 28 * 1), b"\x22" * 28)
        cpu.mem_write(physical(queue + 4 + 28 * 2), b"\x33" * 28)
        cpu.mem_write(physical(queue + 4 + 28 * 3), b"\x44" * 28)
        cpu.reg_write(UC_MIPS_REG_A0, source)
        self.call(cpu, 0x8001B044)
        self.assertEqual(read_word(cpu, queue), 3)
        self.assertEqual(
            bytes(cpu.mem_read(physical(queue + 4 + 28 * 2), 28)),
            struct.pack("<7I", *record),
        )
        self.assertEqual(bytes(cpu.mem_read(physical(queue + 4), 28)), b"\x11" * 28)
        self.assertEqual(bytes(cpu.mem_read(physical(queue + 4 + 28), 28)), b"\x22" * 28)
        self.assertEqual(bytes(cpu.mem_read(physical(queue + 4 + 28 * 3), 28)), b"\x44" * 28)

    def test_queue_update_gate_calls_backends_only_when_enabled(self):
        for flag in (0, 1, 255):
            with self.subTest(flag=flag):
                cpu = self.poll(0)
                cpu.mem_write(physical(QUEUE_UPDATE_FLAG), bytes((flag,)))
                seen = []
                context = 0x801E6000
                write_word(cpu, GP + 0x888, context)

                def trace_and_stub_backends(uc, address, size, data):
                    if address == 0x800516F8:
                        seen.append(("resolve",))
                        return
                    if address == 0x80054A6C:
                        seen.append(("apply", uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)))
                    elif address == 0x8003ABE8:
                        seen.append(("finish",))
                    else:
                        return
                    uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

                cpu.hook_add(UC_HOOK_CODE, trace_and_stub_backends)
                self.call(cpu, QUEUE_UPDATE_GATE)
                expected = [] if flag == 0 else [
                    ("resolve",), ("apply", context, 0x80186154), ("finish",),
                ]
                self.assertEqual(seen, expected)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)

    def test_queue_adjustment_with_real_translation_helpers(self):
        cpu = self.poll(0)
        records, actor, holder, transform = (
            0x801E0000, 0x801E1000, 0x801E2000, 0x801E3000
        )
        write_word(cpu, 0x801D8994, 1)
        write_word(cpu, 0x801D89A4, records)
        write_word(cpu, actor + 0x1C, holder)
        write_word(cpu, holder + 0x14, transform)
        calls = []
        targets = {0x80028764, 0x80054B20, 0x80054C7C}

        def trace(uc, address, size, data):
            if address in targets:
                calls.append(address)

        cpu.hook_add(UC_HOOK_CODE, trace)
        for flag in (0, 1, 255):
            for available in (False, True):
                for y in (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF):
                    for offset in (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF):
                        with self.subTest(flag=flag, available=available, y=y, offset=offset):
                            before = struct.pack(
                                "<9H H 3I", *range(9), 0xA5A5, 0x80000000, y, 0xFFFFFFFF
                            )
                            cpu.mem_write(physical(transform), before)
                            cpu.mem_write(physical(GP + 0x36C), bytes((flag,)))
                            write_word(cpu, GP + 0x378, offset)
                            write_word(cpu, records + 0x1C, actor if available else 0)
                            calls.clear()
                            self.call(cpu, 0x8003ABE8)
                            expected = bytearray(before)
                            if flag and available:
                                struct.pack_into("<I", expected, 24, (y + offset) & 0xFFFFFFFF)
                            self.assertEqual(
                                bytes(cpu.mem_read(physical(transform), len(before))),
                                bytes(expected),
                            )
                            self.assertEqual(read_word(cpu, GP + 0x378),
                                             0 if flag and available else offset)
                            expected_calls = [] if not flag else [0x80028764]
                            if flag and available:
                                expected_calls += [0x80054B20, 0x80054C7C]
                            self.assertEqual(calls, expected_calls)
                            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)

    def test_controller_callback_small_setters(self):
        cpu = self.poll(0)
        for value in range(256):
            cpu.mem_write(physical(GP + 0x6E7), bytes((0xA5, value, 0x5A)))
            self.call(cpu, 0x8001BC08)
            self.assertEqual(bytes(cpu.mem_read(physical(GP + 0x6E7), 3)), b"\xA5\x01\x5A")
            cpu.mem_write(physical(GP + 0x465), bytes((0xA5, value, 0x5A)))
            self.call(cpu, 0x80045278)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), value)
            self.assertEqual(bytes(cpu.mem_read(physical(GP + 0x465), 3)),
                             bytes((0xA5, value, 0x5A)))

        for value in (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF):
            cpu.reg_write(UC_MIPS_REG_A0, value)
            write_word(cpu, GP + 0x6A4, ~value & 0xFFFFFFFF)
            self.call(cpu, 0x80061A64)
            self.assertEqual(read_word(cpu, GP + 0x6A4), value)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)

    def test_controller_callback_resource_cleanup_order(self):
        cpu = self.poll(0)
        calls = []
        replacement = 0

        def intercept(uc, address, size, data):
            if address == 0x80062C54:
                calls.append((address, uc.reg_read(UC_MIPS_REG_A0)))
                write_word(uc, 0x801D8A08, replacement)
            elif address == 0x80082EFC:
                calls.append((address, uc.reg_read(UC_MIPS_REG_A0)))
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for before, after in ((0, 0), (0, 0x801E7000), (0x801E6000, 0x801E7000),
                              (0xFFFFFFFF, 0)):
            with self.subTest(before=before, after=after):
                write_word(cpu, 0x801D8A08, before)
                replacement = after
                calls.clear()
                self.call(cpu, 0x80027F40)
                self.assertEqual(calls, [(0x80062C54, 0x800BE4F8), (0x80082EFC, after)])
                self.assertEqual(read_word(cpu, 0x801D8A08), after)

    def test_controller_callback_mode_transition(self):
        cpu = self.poll(0)
        calls = []
        mode_address = 0x801D89F0
        flag_address = GP + 0x276
        mutate_mode = False

        def intercept(uc, address, size, data):
            if address == 0x8002CA88:
                calls.append((address, uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)))
                if mutate_mode and uc.reg_read(UC_MIPS_REG_A0) == 5:
                    uc.mem_write(physical(mode_address), b"\1")
            elif address == 0x8002BC9C:
                calls.append((address,))
                uc.mem_write(physical(flag_address), b"\xA5")
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for value in (0, 1, 0x100, 0x101, 0x80000000, 0xFFFFFFFF):
            for enabled in (0, 1, 0xFFFFFFFF):
                for mode in (0, 1, 2, 255):
                    with self.subTest(value=value, enabled=enabled, mode=mode):
                        write_word(cpu, 0x8009F58C, enabled)
                        cpu.mem_write(physical(mode_address), bytes((mode,)))
                        cpu.mem_write(physical(flag_address), b"\x5A")
                        calls.clear()
                        cpu.reg_write(UC_MIPS_REG_A0, value)
                        self.call(cpu, 0x8002C9D4)
                        expected = []
                        if (value & 0xFF) == 0:
                            if enabled:
                                expected.append((0x8002CA88, 5, 0xFFFFFFFF))
                            if mode == 1:
                                expected += [(0x8002BC9C,), (0x8002CA88, 15, 0xFFFFFFFF)]
                        self.assertEqual(calls, expected)
                        self.assertEqual(bytes(cpu.mem_read(physical(flag_address), 1)),
                                         bytes((value & 0xFF,)))

        write_word(cpu, 0x8009F58C, 1)
        cpu.mem_write(physical(mode_address), b"\0")
        cpu.mem_write(physical(flag_address), b"\x5A")
        calls.clear()
        mutate_mode = True
        cpu.reg_write(UC_MIPS_REG_A0, 0x100)
        self.call(cpu, 0x8002C9D4)
        self.assertEqual(calls, [(0x8002CA88, 5, 0xFFFFFFFF),
                                 (0x8002BC9C,), (0x8002CA88, 15, 0xFFFFFFFF)])
        self.assertEqual(bytes(cpu.mem_read(physical(flag_address), 1)), b"\0")

    def test_controller_callback_runtime_reset(self):
        cpu = self.fresh_cpu()
        context = 0x801E5000
        actor = 0
        result = 0
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x80028764:
                calls.append(("actor", uc.reg_read(UC_MIPS_REG_A0),
                              bytes(uc.mem_read(physical(GP + 0x6E8), 1))[0]))
                uc.reg_write(UC_MIPS_REG_V0, actor)
            elif address == 0x8003AA1C:
                calls.append(("context", bytes(uc.mem_read(physical(GP + 0x6E8), 1))[0]))
                write_word(uc, uc.reg_read(UC_MIPS_REG_A0), context)
                uc.reg_write(UC_MIPS_REG_V0, 1)
            elif address == 0x80045278:
                calls.append(("gate",))
                uc.reg_write(UC_MIPS_REG_V0, result)
            elif address in (0x80045094, 0x80069758):
                calls.append((address, read_word(uc, GP + 0x13C)))
            elif address == 0x8001B558:
                stack = uc.reg_read(UC_MIPS_REG_SP)
                registers = tuple(uc.reg_read(register) for register in (
                    UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3
                ))
                arguments = tuple(read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C))
                calls.append(("event", *registers, *arguments))
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for flag_a in (0, 1, 255):
            for flag_b in (0, 1, 255):
                for result in (0, 1, 0x10000, 0xFFFFFFFF):
                    for count in (0, 0xFFFFFFFF):
                        for actor in (0, 0x801E6000):
                            with self.subTest(flag_a=flag_a, flag_b=flag_b,
                                              result=result, count=count, actor=actor):
                                cpu.mem_write(physical(context + 0x6D), bytes((flag_a,)))
                                cpu.mem_write(physical(context + 0x2B), bytes((flag_b,)))
                                cpu.mem_write(physical(GP + 0x6E8), b"\x5A")
                                write_word(cpu, GP + 0x13C, count)
                                calls.clear()
                                self.call(cpu, 0x8001BC28)
                                expected = [("actor", 0, 0x5A), ("context", 0)]
                                triggered = flag_a == 0 and flag_b == 0 and (result & 0xFFFF) != 0
                                if flag_a == 0 and flag_b == 0:
                                    expected.append(("gate",))
                                if triggered:
                                    next_count = (count + 1) & 0xFFFFFFFF
                                    expected += [
                                        (0x80045094, next_count), (0x80069758, next_count),
                                        ("event", 4, 1, 0xFFFE, 0xFFFE,
                                         (actor + 0x0C) & 0xFFFFFFFF, 0, 0, 0),
                                    ]
                                self.assertEqual(calls, expected)
                                self.assertEqual(read_word(cpu, GP + 0x13C),
                                                 (count + int(triggered)) & 0xFFFFFFFF)
                                self.assertEqual(bytes(cpu.mem_read(physical(GP + 0x6E8), 1)), b"\0")

    def test_controller_callback_timeout_dispatch(self):
        cpu = self.fresh_cpu()
        event_id_address = 0x801D8AC4
        gate_address = 0x801D8AD0
        calls = []
        status = 0
        cleanup_replacement = None
        reset_replacement = None

        def intercept(uc, address, size, data):
            if address == 0x80062890:
                calls.append(("check",))
                uc.reg_write(UC_MIPS_REG_V0, status)
            elif address == 0x800556D0:
                args = tuple(uc.reg_read(register) for register in (
                    UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3
                ))
                calls.append(("assert", *args))
            elif address == 0x80048488:
                calls.append(("cleanup",))
                if cleanup_replacement is not None:
                    uc.mem_write(physical(event_id_address), bytes((cleanup_replacement,)))
            elif address == 0x8001BC28:
                calls.append(("reset", bytes(uc.mem_read(physical(gate_address), 1))[0]))
                if reset_replacement is not None:
                    uc.mem_write(physical(event_id_address), bytes((reset_replacement,)))
                    uc.mem_write(physical(gate_address), b"\xA5")
            elif address == 0x8001B558:
                stack = uc.reg_read(UC_MIPS_REG_SP)
                registers = tuple(uc.reg_read(register) for register in (
                    UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3
                ))
                arguments = tuple(read_word(uc, stack + offset) for offset in (0x10, 0x14, 0x18, 0x1C))
                calls.append(("event", *registers, *arguments))
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        write_word(cpu, 0x801D8AC8, 0x80000000)
        write_word(cpu, 0x801D8ACC, 0xFFFFFFFF)
        for event_id in range(256):
            for status in (0, 1, 0xFFFFFFFF):
                with self.subTest(event_id=event_id, status=status):
                    cpu.mem_write(physical(event_id_address), bytes((event_id,)))
                    cpu.mem_write(physical(gate_address), b"\x5A")
                    calls.clear()
                    self.call(cpu, 0x8001C04C)
                    expected = []
                    if event_id not in (0xFE, 0x16):
                        expected.append(("check",))
                        if status:
                            expected.append(("assert", 0, 0x80011E14, 0x80011E2C, 0x49A))
                        expected.append(("cleanup",))
                    if event_id != 0xFE:
                        expected.append(("reset", 0))
                    if event_id not in (0xFE, 0xFF):
                        expected.append(("event", event_id, 0, 0x80000000, 0xFFFFFFFF, 0, 0, 0, 0))
                    self.assertEqual(calls, expected)
                    self.assertEqual(bytes(cpu.mem_read(physical(gate_address), 1)), b"\0")
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)

        for original, cleanup_replacement, reset_replacement, expected_id in (
            (1, 0xFE, None, None), (1, 0xFF, 3, 3),
            (0x16, None, 0xFF, None), (0x16, None, 0xFD, 0xFD),
        ):
            with self.subTest(original=original, cleanup=cleanup_replacement,
                              reset=reset_replacement):
                status = 0
                cpu.mem_write(physical(event_id_address), bytes((original,)))
                cpu.mem_write(physical(gate_address), b"\x5A")
                calls.clear()
                self.call(cpu, 0x8001C04C)
                dispatched = [event for event in calls if event[0] == "event"]
                expected = [] if expected_id is None else [
                    ("event", expected_id, 0, 0x80000000, 0xFFFFFFFF, 0, 0, 0, 0)
                ]
                self.assertEqual(dispatched, expected)
                self.assertEqual(bytes(cpu.mem_read(physical(gate_address), 1)),
                                 b"\xA5" if reset_replacement is not None else b"\0")

    def test_controller_timeout_resource_cleanup(self):
        cpu = self.fresh_cpu()
        active = GP + 0x6AC
        protection = GP + 0x688
        pointers = tuple(GP + offset for offset in (0x694, 0x698, 0x69C, 0x6A0))
        allocations = tuple(0x801E6000 + index * 0x20 for index in range(6))
        calls = []
        unprotect = False
        mutate = False

        def intercept(uc, address, size, data):
            nonlocal mutate
            if address in (0x80062750, 0x8008714C):
                calls.append((address,))
                if address == 0x80062750 and unprotect:
                    write_word(uc, protection, 0)
            elif address in (0x800968BC, 0x80096AEC):
                calls.append((address, uc.reg_read(UC_MIPS_REG_A0)))
            elif address == 0x80082EFC:
                calls.append((address, uc.reg_read(UC_MIPS_REG_A0)))
                if mutate:
                    mutate = False
                    write_word(uc, protection, 1)
                    write_word(uc, pointers[0], allocations[4])
                    write_word(uc, pointers[1], allocations[5])
                    write_word(uc, pointers[2], 0)
            elif address == 0x800601D0:
                calls.append((address, uc.reg_read(UC_MIPS_REG_A0),
                              bytes(uc.mem_read(physical(active), 1))[0]))
                uc.mem_write(physical(active), b"\x77")
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for enabled in (0, 1, 255):
            for protected in (0, 1, 0xFFFFFFFF):
                for mask in range(16):
                    with self.subTest(enabled=enabled, protected=protected, mask=mask):
                        cpu.mem_write(physical(active), bytes((enabled,)))
                        write_word(cpu, protection, protected)
                        for index, pointer in enumerate(pointers):
                            write_word(cpu, pointer, allocations[index] if mask & (1 << index) else 0)
                        calls.clear()
                        self.call(cpu, 0x80062890)
                        expected = []
                        if enabled:
                            expected = [(0x80062750,), (0x8008714C,),
                                        (0x800968BC, 0), (0x80096AEC, 0)]
                            if protected == 0:
                                expected.extend((0x80082EFC, allocations[index])
                                                for index in range(4) if mask & (1 << index))
                            expected.append((0x800601D0, 0x800BE3D0, enabled))
                        self.assertEqual(calls, expected)
                        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)
                        self.assertEqual(bytes(cpu.mem_read(physical(active), 1)), b"\0")
                        for index, pointer in enumerate(pointers):
                            original = allocations[index] if mask & (1 << index) else 0
                            expected_pointer = 0 if enabled and protected == 0 else original
                            self.assertEqual(read_word(cpu, pointer), expected_pointer)

        cpu.mem_write(physical(active), b"\1")
        write_word(cpu, protection, 1)
        for index, pointer in enumerate(pointers):
            write_word(cpu, pointer, allocations[index])
        calls.clear()
        unprotect = True
        mutate = True
        self.call(cpu, 0x80062890)
        self.assertEqual(calls, [
            (0x80062750,), (0x8008714C,), (0x800968BC, 0), (0x80096AEC, 0),
            (0x80082EFC, allocations[0]), (0x80082EFC, allocations[5]),
            (0x80082EFC, allocations[3]), (0x800601D0, 0x800BE3D0, 1),
        ])
        self.assertEqual(read_word(cpu, protection), 1)
        self.assertEqual([read_word(cpu, pointer) for pointer in pointers], [0, 0, 0, 0])
        self.assertEqual(bytes(cpu.mem_read(physical(active), 1)), b"\0")

    def test_controller_reset_resource_backend(self):
        cpu = self.fresh_cpu()
        active = GP + 0x484
        secondary = GP + 0x485
        entry_count = GP + 0x91C
        entries = 0x801D795C
        clear_count = GP + 0x468
        clear_words = 0x801D08E4
        words = (0x1234FFFF, 0xF00D8000, 0xC0FF007F)
        halves = (0xFFFFFFFF, 0xFFFF8000, 0x7F)
        calls = []
        mutation = 0
        entry_calls = 0

        def intercept(uc, address, size, data):
            nonlocal entry_calls
            if address == 0x8004380C:
                calls.append(("setup", bytes(uc.mem_read(physical(active), 1))[0]))
                if mutation == 1:
                    write_word(uc, entry_count, 2)
                    write_word(uc, clear_count, 2)
            elif address == 0x80044458:
                calls.append(("word", uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)))
                if mutation == 1 and entry_calls == 0:
                    write_word(uc, entries, 0x1234FF80)
                    write_word(uc, entry_count, 3)
                entry_calls += 1
            elif address == 0x8004232C:
                calls.append(("half", uc.reg_read(UC_MIPS_REG_A0)))
                if mutation == 2 and entry_calls == 1:
                    write_word(uc, entry_count, 1)
            elif address in (0x800439EC, 0x80044268, 0x80069758):
                calls.append((address, bytes(uc.mem_read(physical(active), 1))[0],
                              bytes(uc.mem_read(physical(secondary), 1))[0]))
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        def initialize(count, clear, mutation_mode):
            nonlocal mutation, entry_calls
            mutation = mutation_mode
            entry_calls = 0
            calls.clear()
            cpu.mem_write(physical(active), b"\x5A\xA5")
            write_word(cpu, entry_count, count)
            write_word(cpu, clear_count, clear & 0xFFFFFFFF)
            for index, value in enumerate(words):
                write_word(cpu, entries + index * 4, value)
            for index in range(6):
                write_word(cpu, clear_words + index * 4, 0xA5A50000 + index)

        def verify(clear):
            self.assertEqual(bytes(cpu.mem_read(physical(active), 2)), b"\0\0")
            for index in range(6):
                expected = 0 if 0 <= index < clear else 0xA5A50000 + index
                self.assertEqual(read_word(cpu, clear_words + index * 4), expected)

        tail = [(0x800439EC, 1, 0), (0x800439EC, 1, 0),
                (0x80044268, 1, 0), (0x80069758, 1, 0)]
        cpu.hook_add(UC_HOOK_CODE, intercept)
        for count in (0, 1, 3):
            for clear in (-0x80000000, -1, 0, 1, 4):
                with self.subTest(count=count, clear=clear):
                    initialize(count, clear, 0)
                    self.call(cpu, 0x80045094)
                    expected = [("setup", 1)]
                    for index in range(count):
                        expected.extend((("word", words[index], 0), ("half", halves[index])))
                    self.assertEqual(calls, expected + tail)
                    verify(clear)

        initialize(1, 0, 1)
        self.call(cpu, 0x80045094)
        self.assertEqual(calls, [
            ("setup", 1), ("word", words[0], 0), ("half", 0xFFFFFF80),
            ("word", words[1], 0), ("half", halves[1]),
            ("word", words[2], 0), ("half", halves[2]),
        ] + tail)
        self.assertEqual(read_word(cpu, entry_count), 3)
        verify(2)

        initialize(3, 0, 2)
        self.call(cpu, 0x80045094)
        self.assertEqual(calls, [("setup", 1), ("word", words[0], 0), ("half", halves[0])] + tail)
        self.assertEqual(read_word(cpu, entry_count), 1)
        verify(0)

    def test_proximity_list_backend(self):
        cpu = self.fresh_cpu()
        reference_object, reference_position = 0x801A0000, 0x801A0040
        first_node, first_object, first_position = 0x801A0100, 0x801A0200, 0x801A0300
        second_node, second_object, second_position = 0x801A0400, 0x801A0500, 0x801A0600
        other_current = 0x801A0700
        count_address, objects_address = 0x800BE41C, 0x801D5050
        state = {}
        calls = []

        def write_half(address, value):
            cpu.mem_write(physical(address), struct.pack("<H", value & 0xFFFF))

        def read_half(address):
            return struct.unpack("<H", cpu.mem_read(physical(address), 2))[0]

        def intercept(uc, address, size, data):
            if address == 0x8005327C:
                calls.append("reference")
                write_half(count_address, 0xA55A)
                uc.reg_write(UC_MIPS_REG_V0, state["reference"])
            elif address == 0x800529D0:
                calls.append("list")
                self.assertEqual(read_half(count_address), 0)
                state["slot"] = uc.reg_read(UC_MIPS_REG_A0)
                write_word(uc, state["slot"], state["head"])
                if state["initial_count"] is not None:
                    write_half(count_address, state["initial_count"])
                if state["mutate_reference"]:
                    write_word(uc, reference_position + 0x14, 100000)
                    write_word(uc, reference_position + 0x1C, (-100000) & 0xFFFFFFFF)
            elif address == 0x800556D0:
                args = tuple(uc.reg_read(register) for register in (
                    UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3))
                calls.append(("assert", read_half(count_address), args))
                self.assertEqual(args, (1, 0x80013E78, 0x80013E30, 0x342))
                if state["mutation"]:
                    self.assertEqual(read_half(count_address), 40)
                    self.assertEqual(read_word(uc, objects_address + 39 * 4), first_object)
                    if state["mutation"] == "slot":
                        write_word(uc, state["slot"], other_current)
                    else:
                        write_word(uc, first_node + 8, second_node)
                    write_half(count_address, state["replacement_count"])
                    write_word(uc, reference_position + 0x14, 100000)
                    write_word(uc, reference_position + 0x1C, (-100000) & 0xFFFFFFFF)
                    state["mutation"] = None
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        def initialize(rx=0, rz=0, x=0, z=0, radius=1, reference=True, head=True,
                       initial_count=None, mutate_reference=False):
            calls.clear()
            state.clear()
            state.update({
                "reference": reference_object if reference else 0,
                "head": first_node if head else 0,
                "initial_count": initial_count,
                "mutate_reference": mutate_reference,
                "mutation": None,
                "replacement_count": 0,
            })
            write_word(cpu, reference_object + 0x14, reference_position)
            write_word(cpu, reference_position + 0x14, rx & 0xFFFFFFFF)
            write_word(cpu, reference_position + 0x1C, rz & 0xFFFFFFFF)
            write_word(cpu, first_node, first_object)
            write_word(cpu, first_node + 8, 0)
            write_word(cpu, first_object + 0x14, first_position)
            write_half(first_object + 0x1E, radius)
            write_word(cpu, first_position + 0x14, x & 0xFFFFFFFF)
            write_word(cpu, first_position + 0x1C, z & 0xFFFFFFFF)
            write_word(cpu, second_node, second_object)
            write_word(cpu, second_node + 8, 0)
            write_word(cpu, second_object + 0x14, second_position)
            write_half(second_object + 0x1E, 1)
            write_word(cpu, second_position + 0x14, rx & 0xFFFFFFFF)
            write_word(cpu, second_position + 0x1C, rz & 0xFFFFFFFF)
            write_word(cpu, other_current + 8, second_node)
            write_half(count_address, 0xFFFF)
            for index in (-1, 0, 5, 38, 39, 40, 32767):
                write_word(cpu, objects_address + index * 4, 0xDEADBEEF)

        def nearby(rx, rz, x, z, radius):
            def distance(a, b):
                value = (a - b) & 0xFFFFFFFF
                return (-value) & 0xFFFFFFFF if value & 0x80000000 else value

            total = (distance(x, rx) + distance(z, rz)) & 0xFFFFFFFF
            signed_total = total - 0x100000000 if total & 0x80000000 else total
            return radius != 0 and signed_total < radius

        cpu.hook_add(UC_HOOK_CODE, intercept)
        initialize(head=False)
        self.call(cpu, 0x80069758)
        self.assertEqual(calls, ["reference", "list"])
        self.assertEqual(read_half(count_address), 0)
        self.assertEqual(read_word(cpu, objects_address), 0xDEADBEEF)

        initialize(reference=False, initial_count=23)
        write_word(cpu, first_node, 0)
        write_word(cpu, first_node + 8, second_node)
        write_word(cpu, second_node, 0)
        self.call(cpu, 0x80069758)
        self.assertEqual(calls, ["reference", "list"])
        self.assertEqual(read_half(count_address), 23)

        coordinates = (-0x80000000, -0x7FFFFFFF, -32768, -1, 0, 1, 32767, 0x7FFFFFFF)
        radii = (-32768, -1, 0, 1, 2, 32767)
        rng = random.Random(0x69758)
        samples = [(0, 0, 0, 0, radius) for radius in radii]
        samples.extend((rng.choice(coordinates), rng.choice(coordinates),
                        rng.choice(coordinates), rng.choice(coordinates), rng.choice(radii))
                       for _ in range(512))
        for rx, rz, x, z, radius in samples:
            with self.subTest(reference=(rx, rz), position=(x, z), radius=radius):
                initialize(rx, rz, x, z, radius)
                self.call(cpu, 0x80069758)
                expected = nearby(rx, rz, x, z, radius)
                self.assertEqual(calls, ["reference", "list"])
                self.assertEqual(read_half(count_address), int(expected))
                self.assertEqual(read_word(cpu, objects_address),
                                 first_object if expected else 0xDEADBEEF)

        for initial in (38, 39, 40, 32767):
            with self.subTest(initial_count=initial):
                initialize(17, -9, 17, -9, initial_count=initial, mutate_reference=True)
                self.call(cpu, 0x80069758)
                self.assertEqual(read_half(count_address), (initial + 1) & 0xFFFF)
                self.assertEqual(read_word(cpu, objects_address + initial * 4), first_object)
                assertions = [call for call in calls if isinstance(call, tuple)]
                self.assertEqual(len(assertions), int(initial in (39, 40)))
                if assertions:
                    self.assertEqual(assertions[0][1], initial + 1)

        for mutation, replacement_count in (("slot", 0xFFFF), ("next", 5)):
            with self.subTest(mutation=mutation):
                initialize(17, -9, 17, -9, initial_count=39)
                state["mutation"] = mutation
                state["replacement_count"] = replacement_count
                self.call(cpu, 0x80069758)
                self.assertEqual(calls[:2], ["reference", "list"])
                self.assertEqual(len(calls), 3)
                self.assertEqual(calls[2][1], 40)
                self.assertEqual(read_word(cpu, objects_address + 39 * 4), first_object)
                index = -1 if mutation == "slot" else 5
                self.assertEqual(read_word(cpu, objects_address + index * 4), second_object)
                self.assertEqual(read_half(count_address), (replacement_count + 1) & 0xFFFF)

    def test_proximity_with_real_object_getters(self):
        cpu = self.fresh_cpu()
        reference_global, list_global = 0x800BE33C, 0x800BE32C
        count_address, objects_address = 0x800BE41C, 0x801D5050
        reference, reference_position = 0x801A0000, 0x801A0040
        node, object_address, position = 0x801A0100, 0x801A0200, 0x801A0300
        output = 0x801A0400

        write_word(cpu, reference_global, 0)
        write_word(cpu, list_global, 0)
        write_word(cpu, output, 0xDEADBEEF)
        self.call(cpu, 0x8005327C)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)
        cpu.reg_write(UC_MIPS_REG_A0, output)
        self.call(cpu, 0x800529D0)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)
        self.assertEqual(read_word(cpu, output), 0)

        write_word(cpu, reference_global, reference)
        write_word(cpu, list_global, node)
        write_word(cpu, reference + 0x14, reference_position)
        write_word(cpu, reference_position + 0x14, 7)
        write_word(cpu, reference_position + 0x1C, (-5) & 0xFFFFFFFF)
        write_word(cpu, node, object_address)
        write_word(cpu, node + 8, 0)
        write_word(cpu, object_address + 0x14, position)
        cpu.mem_write(physical(object_address + 0x1E), struct.pack("<h", 2))
        write_word(cpu, position + 0x14, 8)
        write_word(cpu, position + 0x1C, (-5) & 0xFFFFFFFF)
        self.call(cpu, 0x8005327C)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), reference)
        cpu.reg_write(UC_MIPS_REG_A0, list_global)
        self.call(cpu, 0x800529D0)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)
        self.assertEqual(read_word(cpu, list_global), node)
        self.call(cpu, 0x80069758)
        self.assertEqual(struct.unpack("<H", cpu.mem_read(physical(count_address), 2))[0], 1)
        self.assertEqual(read_word(cpu, objects_address), object_address)

        write_word(cpu, reference_global, 0)
        write_word(cpu, node, 0)
        self.call(cpu, 0x80069758)
        self.assertEqual(struct.unpack("<H", cpu.mem_read(physical(count_address), 2))[0], 0)

    def test_reset_tracked_entry_backend(self):
        cpu = self.fresh_cpu()
        head_global = 0x800BE568
        reference_a, reference_b = 0x801A1000, 0x801A1040
        nodes = [0x801A2000 + index * 0x20 for index in range(4)]
        objects = [0x801A3000 + index * 0x40 for index in range(4)]
        calls = []
        state = {"reference": reference_a, "getter_mutates": False, "removal_mutates": False}

        def intercept(uc, address, size, data):
            if address == 0x8005327C:
                calls.append("reference")
                if state["getter_mutates"]:
                    write_word(uc, head_global, nodes[3])
                uc.reg_write(UC_MIPS_REG_V0, state["reference"])
            elif address == 0x80042170:
                node = uc.reg_read(UC_MIPS_REG_A0)
                calls.append(("remove", node))
                if state["removal_mutates"] and node == nodes[0]:
                    write_word(uc, nodes[0] + 0x0C, nodes[3])
                    write_word(uc, objects[2] + 0x10, 0)
                    state["reference"] = reference_b
                    state["removal_mutates"] = False
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        write_word(cpu, head_global, 0)
        cpu.reg_write(UC_MIPS_REG_A0, 0)
        self.call(cpu, 0x8004232C)
        self.assertEqual(calls, ["reference"])

        for value in (0, 1, 0x7FFF, 0x8000, 0xFFFF, 0x10000, 0x1FFFF, 0xFFFFFFFF):
            with self.subTest(tag=value):
                calls.clear()
                write_word(cpu, head_global, nodes[0])
                write_word(cpu, nodes[0], objects[0])
                cpu.mem_write(physical(nodes[0] + 4), struct.pack("<H", value & 0xFFFF))
                write_word(cpu, nodes[0] + 0x0C, 0)
                write_word(cpu, objects[0] + 0x10, 0)
                write_word(cpu, objects[0] + 0x1C, reference_b)
                cpu.reg_write(UC_MIPS_REG_A0, value)
                self.call(cpu, 0x8004232C)
                self.assertEqual(calls, ["reference", ("remove", nodes[0])])

        calls.clear()
        state.update(reference=reference_a, getter_mutates=True, removal_mutates=True)
        write_word(cpu, head_global, nodes[0])
        for index in range(4):
            write_word(cpu, nodes[index], objects[index])
            cpu.mem_write(physical(nodes[index] + 4), struct.pack("<h", 7))
            write_word(cpu, nodes[index] + 0x0C, nodes[index + 1] if index < 2 else 0)
            write_word(cpu, objects[index] + 0x10, 0)
            write_word(cpu, objects[index] + 0x1C, reference_b)
        write_word(cpu, objects[1] + 0x1C, reference_a)
        write_word(cpu, objects[2] + 0x10, 0x200000)
        cpu.reg_write(UC_MIPS_REG_A0, 7)
        self.call(cpu, 0x8004232C)
        self.assertEqual(calls, ["reference", ("remove", nodes[0]), ("remove", nodes[2])])
        self.assertEqual(read_word(cpu, head_global), nodes[3])
        self.assertEqual(read_word(cpu, nodes[0] + 0x0C), nodes[3])

    def test_reset_entry_dispatch_backend(self):
        cpu = self.fresh_cpu()
        table_global, table, other_table = 0x800BE1B8, 0x801A5000, 0x801A5100
        assets = [0x801A6000 + index * 0x40 for index in range(4)]
        calls = []
        state = {"result": 0}

        def intercept(uc, address, size, data):
            if address == 0x80052BDC:
                calls.append(("prepare", uc.reg_read(UC_MIPS_REG_A0),
                              uc.reg_read(UC_MIPS_REG_A1)))
                write_word(uc, table_global, other_table)
            elif address == 0x80044B24:
                calls.append(("select", uc.reg_read(UC_MIPS_REG_A0),
                              uc.reg_read(UC_MIPS_REG_A1)))
                uc.reg_write(UC_MIPS_REG_V0, state["result"])
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, asset in enumerate(assets):
            write_word(cpu, table + index * 4, asset)
            write_word(cpu, other_table + index * 4, 0x801A7000 + index * 0x40)
        for mode in range(512):
            with self.subTest(mode=mode):
                index = mode % 4
                calls.clear()
                state["result"] = (mode << 16) | (255 - (mode & 0xFF))
                write_word(cpu, table_global, table)
                cpu.reg_write(UC_MIPS_REG_A0, index)
                cpu.reg_write(UC_MIPS_REG_A1, mode)
                self.call(cpu, 0x80044458)
                self.assertEqual(calls, [
                    ("prepare", assets[index], 1),
                    ("select", index, mode & 0xFF),
                ])
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), state["result"] & 0xFF)
                self.assertEqual(read_word(cpu, table_global), other_table)

    def test_reset_entry_group_membership(self):
        cpu = self.fresh_cpu()
        groups_global, active_count_global, active_indices = 0x800BE1B0, 0x800BE678, 0x801D795C
        groups, index_arrays = 0x801A1000, 0x801A2000
        rng = random.Random(0x44A88)

        write_word(cpu, groups_global, 0)
        write_word(cpu, active_count_global, 0)
        cpu.reg_write(UC_MIPS_REG_A0, 0)
        cpu.reg_write(UC_MIPS_REG_A1, 0)
        self.call(cpu, 0x80044A88)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)

        write_word(cpu, groups_global, groups)
        for case in range(1024):
            counts = [rng.choice((-0x80000000, -1, 0, 1, 2, 3)) for _ in range(4)]
            entries = [[rng.randrange(6) for _ in range(3)] for _ in range(4)]
            active = [rng.randrange(4) for _ in range(rng.randrange(5))]
            excluded = rng.randrange(5)
            object_index = rng.choice((0, 1, 2, 3, 4, 5, 6, 0xFFFFFFFF))
            for index in range(4):
                group = groups + index * 0x10
                address = index_arrays + index * 0x20
                write_word(cpu, group, address if counts[index] > 0 else 0)
                write_word(cpu, group + 8, counts[index] & 0xFFFFFFFF)
                for entry, value in enumerate(entries[index]):
                    write_word(cpu, address + entry * 4, value)
            write_word(cpu, active_count_global, len(active))
            for index, value in enumerate(active):
                write_word(cpu, active_indices + index * 4, value)
            cpu.reg_write(UC_MIPS_REG_A0, object_index)
            cpu.reg_write(UC_MIPS_REG_A1, groups + excluded * 0x10)
            self.call(cpu, 0x80044A88)
            expected = any(
                index != excluded and object_index in entries[index][:max(counts[index], 0)]
                for index in active
            )
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), int(expected), f"case {case}")

        write_word(cpu, active_count_global, 1)
        write_word(cpu, active_indices, 0)
        write_word(cpu, groups, 0)
        write_word(cpu, groups + 8, 1)
        cpu.reg_write(UC_MIPS_REG_A0, 1)
        cpu.reg_write(UC_MIPS_REG_A1, groups)
        self.call(cpu, 0x80044A88)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)

    def test_reset_entry_group_backend(self):
        cpu = self.fresh_cpu()
        groups_global, objects_global, table_global = 0x800BE1B0, 0x800BE1BC, 0x800BE1B8
        groups, other_groups = 0x801A1000, 0x801A1100
        indices, other_indices = 0x801A1200, 0x801A1300
        objects, other_objects, entry_table = 0x801A1400, 0x801A1500, 0x801A1600
        actor = 0x801A1700
        assets = [0x801A2000 + index * 0x40 for index in range(3)]
        replacements = [0x801A3000 + index * 0x40 for index in range(3)]
        calls = []
        state = {}

        def intercept(uc, address, size, data):
            if address == 0x80028764:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 0)
                calls.append(("get",))
                if state["mutate_getter"]:
                    write_word(uc, groups_global, other_groups)
                    write_word(uc, groups + 8, 2)
                uc.reg_write(UC_MIPS_REG_V0, actor)
            elif address == 0x80044A88:
                index = uc.reg_read(UC_MIPS_REG_A0)
                group = uc.reg_read(UC_MIPS_REG_A1)
                calls.append(("check", index, group))
                if state["mutate_check"]:
                    self.assertEqual((index, group), (0, groups))
                    write_word(uc, groups, other_indices)
                    write_word(uc, groups + 8, 3)
                    write_word(uc, objects_global, other_objects)
                    write_word(uc, actor + 8, assets[2])
                    state["mutate_check"] = False
                uc.reg_write(UC_MIPS_REG_V0, state["results"][index])
            elif address == 0x80052BDC:
                calls.append(("prepare", uc.reg_read(UC_MIPS_REG_A0),
                              uc.reg_read(UC_MIPS_REG_A1)))
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        def initialize(count):
            calls.clear()
            state.clear()
            state.update(mutate_getter=False, mutate_check=False, results=[0x100, 0, 0x101])
            write_word(cpu, groups_global, groups)
            write_word(cpu, objects_global, objects)
            write_word(cpu, groups, indices)
            write_word(cpu, groups + 8, count & 0xFFFFFFFF)
            write_word(cpu, other_groups, indices)
            write_word(cpu, other_groups + 8, 0)
            write_word(cpu, actor + 8, assets[1])
            for index in range(3):
                write_word(cpu, indices + index * 4, index)
                write_word(cpu, objects + index * 4, assets[index])
                write_word(cpu, other_objects + index * 4, replacements[index])

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for mode in (0, 1, 255, 256, 257, 0xFFFFFFFF):
            for count in (-0x80000000, -1, 0, 1, 3):
                with self.subTest(mode=mode, count=count):
                    initialize(count)
                    cpu.reg_write(UC_MIPS_REG_A0, 0)
                    cpu.reg_write(UC_MIPS_REG_A1, mode)
                    self.call(cpu, 0x80044B24)
                    expected = [("get",)]
                    for index in range(max(count, 0)):
                        if index == 1:
                            continue
                        if mode & 0xFF:
                            expected.append(("check", index, groups))
                        if not mode & 0xFF or not state["results"][index] & 0xFF:
                            expected.append(("prepare", assets[index], 1))
                    self.assertEqual(calls, expected)
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)

        initialize(1)
        state.update(mutate_getter=True, mutate_check=True, results=[0x100, 0x101, 0])
        for index, value in enumerate((0, 2, 1)):
            write_word(cpu, other_indices + index * 4, value)
        write_word(cpu, other_objects + 4, assets[1])
        write_word(cpu, other_objects + 8, assets[2])
        cpu.reg_write(UC_MIPS_REG_A0, 0)
        cpu.reg_write(UC_MIPS_REG_A1, 1)
        self.call(cpu, 0x80044B24)
        self.assertEqual(calls, [
            ("get",), ("check", 0, groups), ("prepare", assets[0], 1),
            ("check", 1, groups),
        ])
        self.assertEqual(read_word(cpu, groups_global), other_groups)
        self.assertEqual(read_word(cpu, objects_global), other_objects)
        self.assertEqual(read_word(cpu, groups + 8), 3)

        initialize(3)
        write_word(cpu, table_global, entry_table)
        write_word(cpu, entry_table, replacements[0])
        cpu.reg_write(UC_MIPS_REG_A0, 0)
        cpu.reg_write(UC_MIPS_REG_A1, 256)
        self.call(cpu, 0x80044458)
        self.assertEqual(calls, [
            ("prepare", replacements[0], 1), ("get",),
            ("prepare", assets[0], 1), ("prepare", assets[2], 1),
        ])
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)

    def test_grounded_sequence_limit_selector(self):
        cpu = self.fresh_cpu()
        table = 0x801CCE14
        flag_address, count_address = GP + 0x624, GP + 0x62C
        rng = random.Random(0x5D5A8)
        boundaries = (-32768, -1, 0, 1, 2, 3, 32767)

        cpu.mem_write(physical(count_address), struct.pack("<h", 1))
        cpu.mem_write(physical(table), struct.pack("<hh", 7, 3))
        for flag in range(256):
            cpu.mem_write(physical(flag_address), bytes((flag,)))
            cpu.reg_write(UC_MIPS_REG_A0, 7)
            cpu.reg_write(UC_MIPS_REG_A1, 0)
            self.call(cpu, 0x8005D5A8)
            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0x1B if flag == 0 else 0)

        for sample in range(2048):
            count = rng.choice((-32768, -1, 0, 1, 2, 3))
            flag = rng.choice((0, 1, 2, 255))
            entries = [(rng.choice(boundaries), rng.choice(boundaries)) for _ in range(4)]
            key = rng.choice([entry[0] for entry in entries] + [17])
            value = rng.choice(boundaries)
            raw_key = rng.getrandbits(16) << 16 | (key & 0xFFFF)
            raw_value = rng.getrandbits(16) << 16 | (value & 0xFFFF)
            cpu.mem_write(physical(flag_address), bytes((flag,)))
            cpu.mem_write(physical(count_address), struct.pack("<h", count))
            for index, (entry_key, limit) in enumerate(entries):
                cpu.mem_write(physical(table + index * 72), struct.pack("<hh", entry_key, limit))
            cpu.reg_write(UC_MIPS_REG_A0, raw_key)
            cpu.reg_write(UC_MIPS_REG_A1, raw_value)
            self.call(cpu, 0x8005D5A8)

            index = 0
            while index < count and entries[index][0] != key:
                index += 1
            if flag == 0:
                expected = 0x1B
            elif entries[index][0] != key:
                expected = 1
            else:
                expected = int(not (0 <= value < entries[index][1]))
            with self.subTest(sample=sample, count=count, flag=flag, key=key, value=value):
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), expected)

    def test_grounded_sequence_selector_with_real_wrapper(self):
        cpu = self.fresh_cpu()
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x8008E1DC:
                calls.append((uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)))
                uc.reg_write(UC_MIPS_REG_V0, 123)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.mem_write(physical(GP + 0x62C), struct.pack("<h", 1))
        cpu.mem_write(physical(0x801CCE14), struct.pack("<hh", -1, 3))
        for flag, value, expected, expected_calls in (
                (0, 2, 0x1B, []),
                (1, 2, 0, [(0xFFFFFFFF, 2)]),
                (255, 3, 1, []),
                (1, 0xFFFF, 1, [])):
            with self.subTest(flag=flag, value=value):
                calls.clear()
                cpu.mem_write(physical(GP + 0x624), bytes((flag,)))
                cpu.reg_write(UC_MIPS_REG_A0, 0x1234FFFF)
                cpu.reg_write(UC_MIPS_REG_A1, 0x56780000 | value)
                self.call(cpu, 0x8005E7B8)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), expected)
                self.assertEqual(calls, expected_calls)

    def test_grounded_sequence_initializer(self):
        cpu = self.fresh_cpu()
        table, first_base, replacement_base = 0x801D7D44, 0x801A1000, 0x801A2000
        row = first_base + 3 * 172
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x80094108:
                calls.append((uc.reg_read(UC_MIPS_REG_A0), read_word(uc, row + 0x90)))
                write_word(uc, table + 4 * 4, replacement_base)
                write_word(uc, row + 0x7C, 0x12345678)
                write_word(uc, row + 0x84, 0x89ABCDEF)
                uc.mem_write(physical(row + 0x72), struct.pack("<H", 0xBEEF))
                write_word(uc, row + 8, 0x76543210)
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.mem_write(physical(first_base), b"\xA5" * 1024)
        cpu.mem_write(physical(replacement_base), b"\x5A" * 1024)
        write_word(cpu, table + 4 * 4, first_base)
        write_word(cpu, row + 0x90, 0xFFFFFFFF)
        cpu.reg_write(UC_MIPS_REG_A0, 0x10004)
        cpu.reg_write(UC_MIPS_REG_A1, 0x10003)
        self.call(cpu, 0x8008E044)

        self.assertEqual(calls, [(0x304, 0xFFFFFFF4)])
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0x7F)
        self.assertEqual(read_word(cpu, row + 0x90), 0xFFFFFFF4)
        self.assertEqual(read_word(cpu, row + 0x80), 0)
        self.assertEqual(read_word(cpu, row + 0x88), 0x12345678)
        self.assertEqual(read_word(cpu, row + 0x8C), 0x89ABCDEF)
        self.assertEqual(read_word(cpu, row + 4), 0x76543210)
        self.assertEqual(read_word(cpu, row + 0x0C), 0x76543210)
        self.assertEqual(cpu.mem_read(physical(row + 0x70), 2), struct.pack("<H", 0xBEEF))
        for offset in (0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x27, 0x28, 0x29, 0x2A, 0x2B):
            self.assertEqual(cpu.mem_read(physical(row + offset), 1), b"\0")
        for index in range(16):
            self.assertEqual(cpu.mem_read(physical(row + 0x2C + index), 1), bytes((index,)))
            self.assertEqual(cpu.mem_read(physical(row + 0x17 + index), 1), b"\x40")
            self.assertEqual(cpu.mem_read(physical(row + 0x4E + index * 2), 2), b"\x7F\0")
        self.assertEqual(cpu.mem_read(physical(row + 0x48), 2), b"\0\0")
        self.assertEqual(cpu.mem_read(physical(row + 0x78), 4), b"\x7F\0\x7F\0")
        self.assertEqual(cpu.mem_read(physical(replacement_base), 1024), b"\x5A" * 1024)

    def test_grounded_sequence_initializer_live_table_reads(self):
        cpu = self.fresh_cpu()
        table = 0x801D7D44
        bases = (0x801A1000, 0x801A2000, 0x801A3000, 0x801A4000)
        row_offset = 172
        calls = []

        def intercept(uc, address, size, data):
            replacement = {
                0x8008E0A4: bases[1],
                0x8008E0C0: bases[2],
                0x8008E0DC: bases[3],
            }.get(address)
            if replacement is not None:
                write_word(uc, table + 2 * 4, replacement)
            elif address == 0x80094108:
                calls.append(uc.reg_read(UC_MIPS_REG_A0))
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for index, base in enumerate(bases):
            cpu.mem_write(physical(base), b"\xA5" * 512)
            write_word(cpu, base + row_offset + 0x90, 0 if index == 3 else 0xFFFFFFFF)
        write_word(cpu, table + 2 * 4, bases[0])
        cpu.reg_write(UC_MIPS_REG_A0, 2)
        cpu.reg_write(UC_MIPS_REG_A1, 1)
        self.call(cpu, 0x8008E044)

        self.assertEqual(calls, [0x102])
        self.assertEqual([read_word(cpu, base + row_offset + 0x90) for base in bases],
                         [0xFFFFFFFE, 0xFFFFFFFD, 0xFFFFFFF7, 4])
        self.assertEqual(cpu.mem_read(physical(bases[0] + row_offset + 0x2C), 16),
                         bytes(range(16)))
        self.assertEqual(cpu.mem_read(physical(bases[1] + row_offset + 0x2C), 16),
                         b"\xA5" * 16)

    def test_grounded_sequence_initializer_signed_state(self):
        cpu = self.fresh_cpu()
        table, base = 0x801D7D44, 0x801A1000
        row = base + 3 * 172
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x80094108:
                calls.append(uc.reg_read(UC_MIPS_REG_A0))
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.mem_write(physical(base), b"\xA5" * 1024)
        write_word(cpu, table + 4 * 4, base + 4 * 172)
        write_word(cpu, row + 0x90, 0x0B)
        cpu.reg_write(UC_MIPS_REG_A0, 0x10004)
        cpu.reg_write(UC_MIPS_REG_A1, 0x1FFFF)
        self.call(cpu, 0x8008E044)

        self.assertEqual(calls, [0xFFFFFF04])
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0x7F)
        self.assertEqual(read_word(cpu, row + 0x90), 4)
        self.assertEqual(cpu.mem_read(physical(row + 0x2C), 16), bytes(range(16)))

    def test_grounded_sequence_initializer_all_low_flags(self):
        cpu = self.fresh_cpu()
        table, base = 0x801D7D44, 0x801A1000
        row = base + 3 * 172
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x80094108:
                calls.append(uc.reg_read(UC_MIPS_REG_A0))
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        write_word(cpu, table + 4 * 4, base)
        for value in range(256):
            flags = 0xA5A50000 | value
            cpu.mem_write(physical(base), b"\xA5" * 1024)
            write_word(cpu, row + 0x90, flags)
            cpu.reg_write(UC_MIPS_REG_A0, 0x10004)
            cpu.reg_write(UC_MIPS_REG_A1, 0x10003)
            self.call(cpu, 0x8008E044)
            self.assertEqual(read_word(cpu, row + 0x90), (flags & ~0x0B) | 4)
        self.assertEqual(calls, [0x304] * 256)

    def test_grounded_sequence_notification(self):
        cpu = self.fresh_cpu()
        count, records, current = 0x801D5914, 0x801CFE8E, 0x801D5F26
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x80091194:
                calls.append((uc.reg_read(UC_MIPS_REG_A0),
                              struct.unpack("<H", uc.mem_read(physical(current), 2))[0]))
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.mem_write(physical(records), b"\0" * (5 * 52))
        for index, key in enumerate((-2, 7, -2, -2)):
            cpu.mem_write(physical(records + index * 52), struct.pack("<h", key))
        cpu.mem_write(physical(count), b"\x04")
        cpu.mem_write(physical(current), struct.pack("<H", 0xBEEF))
        cpu.reg_write(UC_MIPS_REG_A0, 0x1FFFE)
        self.call(cpu, 0x80094108)

        self.assertEqual(calls, [(0, 0), (0, 2), (0, 3)])
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 4)
        self.assertEqual(cpu.mem_read(physical(current), 2), struct.pack("<H", 3))

        calls.clear()
        cpu.mem_write(physical(count), b"\0")
        cpu.mem_write(physical(current), struct.pack("<H", 0xBEEF))
        cpu.reg_write(UC_MIPS_REG_A0, 0x1FFFE)
        self.call(cpu, 0x80094108)
        self.assertEqual(calls, [])
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)
        self.assertEqual(cpu.mem_read(physical(current), 2), struct.pack("<H", 0xBEEF))

    def test_grounded_sequence_notification_live_count(self):
        cpu = self.fresh_cpu()
        count, records, current = 0x801D5914, 0x801CFE8E, 0x801D5F26
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x80091194:
                index = struct.unpack("<H", uc.mem_read(physical(current), 2))[0]
                calls.append(index)
                if index == 0:
                    uc.mem_write(physical(count), b"\x04")
                    uc.mem_write(physical(records + 2 * 52), struct.pack("<h", 10))
                elif index == 2:
                    uc.mem_write(physical(count), b"\x03")
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.mem_write(physical(records), b"\0" * (5 * 52))
        for index, key in enumerate((10, 11, 12, 10)):
            cpu.mem_write(physical(records + index * 52), struct.pack("<h", key))
        cpu.mem_write(physical(count), b"\x02")
        cpu.reg_write(UC_MIPS_REG_A0, 0x1000A)
        self.call(cpu, 0x80094108)

        self.assertEqual(calls, [0, 2])
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 3)
        self.assertEqual(cpu.mem_read(physical(current), 2), struct.pack("<H", 2))

    def test_grounded_sequence_notification_byte_count_limit(self):
        cpu = self.fresh_cpu()
        count, records, current = 0x801D5914, 0x801CFE8E, 0x801D5F26
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x80091194:
                calls.append(struct.unpack("<H", uc.mem_read(physical(current), 2))[0])
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.mem_write(physical(records), b"\0" * (255 * 52))
        for index in range(255):
            cpu.mem_write(physical(records + index * 52), struct.pack("<h", 1))
        cpu.mem_write(physical(count), b"\xFF")
        cpu.reg_write(UC_MIPS_REG_A0, 1)
        self.call(cpu, 0x80094108)

        self.assertEqual(calls, list(range(255)))
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 255)
        self.assertEqual(cpu.mem_read(physical(current), 2), struct.pack("<H", 254))

    def test_grounded_sequence_initializer_with_real_notification(self):
        cpu = self.fresh_cpu()
        character_table, base, replacement = 0x801D7D44, 0x801A1000, 0x801A2000
        count, records, current = 0x801D5914, 0x801CFE8E, 0x801D5F26
        row = base + 3 * 172
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x80091194:
                calls.append((uc.reg_read(UC_MIPS_REG_A0),
                              struct.unpack("<H", uc.mem_read(physical(current), 2))[0]))
                write_word(uc, character_table + 4 * 4, replacement)
                write_word(uc, row + 0x7C, 0x12345678)
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        cpu.mem_write(physical(base), b"\xA5" * 1024)
        cpu.mem_write(physical(replacement), b"\x5A" * 1024)
        write_word(cpu, character_table + 4 * 4, base)
        cpu.mem_write(physical(count), b"\x01")
        cpu.mem_write(physical(records), struct.pack("<h", 0x304))
        cpu.reg_write(UC_MIPS_REG_A0, 4)
        cpu.reg_write(UC_MIPS_REG_A1, 3)
        self.call(cpu, 0x8008E044)

        self.assertEqual(calls, [(0, 0)])
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0x7F)
        self.assertEqual(read_word(cpu, row + 0x88), 0x12345678)
        self.assertEqual(cpu.mem_read(physical(row + 0x2C), 16), bytes(range(16)))
        self.assertEqual(cpu.mem_read(physical(replacement), 1024), b"\x5A" * 1024)

    def test_grounded_sequence_notification_backend(self):
        cpu = self.fresh_cpu()
        table, current = 0x801CFE80, 0x801D5F26
        masks = (0x801D8AD8, 0x801D8AE0, 0x801D043C, 0x801D0444)
        rng = random.Random(0x91194)

        for index in (*range(256), 445, 475, 624, 689, 690, 691, 692):
            with self.subTest(index=index):
                row = table + 52 * index
                cpu.mem_write(physical(row), bytes(rng.randrange(256) for _ in range(52)))
                initial = [rng.randrange(65536) for _ in masks]
                for address, value in zip(masks, initial):
                    cpu.mem_write(physical(address), struct.pack("<H", value))
                cpu.mem_write(physical(current), struct.pack("<H", index))
                before = bytearray(cpu.mem_read(physical(row), 52))
                cpu.reg_write(UC_MIPS_REG_A0, (index % 3) - 1 & 0xFFFFFFFF)
                self.call(cpu, 0x80091194)

                before[0:2] = b"\0\0"
                before[4:6] = b"\0\0"
                before[27] = 0
                for position, address in enumerate(masks):
                    if row <= address and address + 2 <= row + 52:
                        initial[position] = struct.unpack_from("<H", before, address - row)[0]
                low_bit = (1 << index) if index < 16 else 0
                high_bit = (1 << ((index - 16) & 31)) if index >= 16 else 0
                set_low = (initial[0] | low_bit) & 0xFFFF
                set_high = (initial[1] | high_bit) & 0xFFFF
                expected_masks = (
                    set_low, set_high, initial[2] & ~set_low, initial[3] & ~set_high,
                )
                for address, value in zip(masks, expected_masks):
                    self.assertEqual(cpu.mem_read(physical(address), 2), struct.pack("<H", value))
                    if row <= address and address + 2 <= row + 52:
                        struct.pack_into("<H", before, address - row, value)
                self.assertEqual(cpu.mem_read(physical(row), 52), bytes(before))

    def test_grounded_initializer_with_real_notification_backend(self):
        cpu = self.fresh_cpu()
        character_table, base = 0x801D7D44, 0x801A1000
        count, table, current = 0x801D5914, 0x801CFE80, 0x801D5F26
        masks = (0x801D8AD8, 0x801D8AE0, 0x801D043C, 0x801D0444)
        calls = []

        def trace(uc, address, size, data):
            if address == 0x80091194:
                calls.append(struct.unpack("<H", uc.mem_read(physical(current), 2))[0])

        cpu.hook_add(UC_HOOK_CODE, trace)
        cpu.mem_write(physical(base), b"\xA5" * 1024)
        write_word(cpu, character_table + 4 * 4, base)
        cpu.mem_write(physical(table), b"\xA5" * (3 * 52))
        for index, key in enumerate((0x304, 9, 0x304)):
            cpu.mem_write(physical(table + 52 * index + 14), struct.pack("<h", key))
        for address, value in zip(masks, (0x40, 0x03, 0xFF, 0xF5)):
            cpu.mem_write(physical(address), struct.pack("<H", value))
        cpu.mem_write(physical(count), b"\x03")
        cpu.reg_write(UC_MIPS_REG_A0, 4)
        cpu.reg_write(UC_MIPS_REG_A1, 3)
        self.call(cpu, 0x8008E044)

        self.assertEqual(calls, [0, 2])
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0x7F)
        self.assertEqual(cpu.mem_read(physical(current), 2), struct.pack("<H", 2))
        for address, value in zip(masks, (0x45, 0x03, 0xBA, 0xF4)):
            self.assertEqual(cpu.mem_read(physical(address), 2), struct.pack("<H", value))
        for index in (0, 2):
            row = table + 52 * index
            self.assertEqual(cpu.mem_read(physical(row), 2), b"\0\0")
            self.assertEqual(cpu.mem_read(physical(row + 4), 2), b"\0\0")
            self.assertEqual(cpu.mem_read(physical(row + 27), 1), b"\0")
        self.assertEqual(cpu.mem_read(physical(table + 52), 2), b"\xA5\xA5")
        self.assertEqual(cpu.mem_read(physical(base + 3 * 172 + 0x2C), 16), bytes(range(16)))

    def test_grounded_sequence_notification_with_real_backend_byte_limit(self):
        cpu = self.fresh_cpu()
        count, table, current = 0x801D5914, 0x801CFE80, 0x801D5F26
        masks = (0x801D8AD8, 0x801D8AE0, 0x801D043C, 0x801D0444)
        cpu.mem_write(physical(table), b"\xA5" * (256 * 52))
        for index in range(255):
            cpu.mem_write(physical(table + 52 * index + 14), struct.pack("<h", 1))
        for address, value in zip(masks, (0, 0, 0xFFFF, 0xFFFF)):
            cpu.mem_write(physical(address), struct.pack("<H", value))
        cpu.mem_write(physical(count), b"\xFF")
        cpu.reg_write(UC_MIPS_REG_A0, 1)
        self.call(cpu, 0x80094108)

        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 255)
        self.assertEqual(cpu.mem_read(physical(current), 2), struct.pack("<H", 254))
        for address, value in zip(masks, (0xFFFF, 0xFFFF, 0, 0)):
            self.assertEqual(cpu.mem_read(physical(address), 2), struct.pack("<H", value))
        for index in range(255):
            row = table + 52 * index
            self.assertEqual(cpu.mem_read(physical(row), 2), b"\0\0")
            self.assertEqual(cpu.mem_read(physical(row + 4), 2), b"\0\0")
            self.assertEqual(cpu.mem_read(physical(row + 27), 1), b"\0")
        self.assertEqual(cpu.mem_read(physical(table + 255 * 52), 2), b"\xA5\xA5")

    def test_shared_actor_mode_zero_transition_boundary(self):
        cpu = self.fresh_cpu()
        actor, component, shared = 0x801A1000, 0x801A1100, 0x801A1200
        transition_index, limit = 0x800BE0DC, 0x800BDE98
        calls = []
        state = {}

        def intercept(uc, address, size, data):
            if address == 0x80052ABC:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), component)
                write_word(uc, uc.reg_read(UC_MIPS_REG_A1), 0x2EF)
                calls.append(("component",))
            elif address == 0x80022C94:
                sp = uc.reg_read(UC_MIPS_REG_SP)
                calls.append(("select", uc.reg_read(UC_MIPS_REG_A0),
                              uc.reg_read(UC_MIPS_REG_A1), uc.reg_read(UC_MIPS_REG_A2),
                              uc.reg_read(UC_MIPS_REG_A3),
                              tuple(read_word(uc, sp + offset) for offset in (0x10, 0x14, 0x18))))
                uc.reg_write(UC_MIPS_REG_V0, state["selector"])
            elif address == 0x8002944C:
                calls.append(("reset", uc.reg_read(UC_MIPS_REG_A0),
                              uc.reg_read(UC_MIPS_REG_A1)))
                write_word(uc, actor + 4, state["flags_after_reset"])
            elif address == 0x8001B558:
                sp = uc.reg_read(UC_MIPS_REG_SP)
                calls.append(("transition", uc.reg_read(UC_MIPS_REG_A0),
                              uc.reg_read(UC_MIPS_REG_A1), uc.reg_read(UC_MIPS_REG_A2),
                              uc.reg_read(UC_MIPS_REG_A3),
                              tuple(read_word(uc, sp + offset) for offset in (0x10, 0x14, 0x18, 0x1C)),
                              read_word(uc, transition_index)))
            elif address == 0x8002269C:
                calls.append(("clear", uc.reg_read(UC_MIPS_REG_A0),
                              uc.reg_read(UC_MIPS_REG_A1)))
            elif address == 0x80055680:
                calls.append(("assert", uc.reg_read(UC_MIPS_REG_A0)))
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        thresholds = (
            (0, 31), (0, 30), (0xFFFFFFF0, 0x20),
            (0xFFFFFFFF, 30), (0xFFFFFFFF, 29), (0x1234, 0xFFFFFFFF),
        )
        for old_index, maximum in thresholds:
            for selector in (0, 0x100, 0x101):
                for flags_after_reset in (0, 2):
                    for control in (0, 1, 2):
                        with self.subTest(old_index=old_index, maximum=maximum,
                                          selector=selector, flags=flags_after_reset, control=control):
                            calls.clear()
                            state.update(selector=selector, flags_after_reset=flags_after_reset)
                            cpu.mem_write(physical(actor), bytes(0x40))
                            cpu.mem_write(physical(actor), b"\x5A\x66")
                            write_word(cpu, actor + 4, 2)
                            write_word(cpu, actor + 0x0C, 0x12345678)
                            write_word(cpu, actor + 0x10, 0x40)
                            write_word(cpu, actor + 0x1C, component)
                            write_word(cpu, actor + 0x28, shared)
                            cpu.mem_write(physical(0x80186479), b"\0")
                            cpu.mem_write(physical(0x801864BC), bytes((control != 0,)))
                            write_word(cpu, 0x80186490, 0xFFFFFFFF if control == 1 else 0)
                            cpu.mem_write(physical(0x80186464), bytes((control == 2,)))
                            cpu.mem_write(physical(0x801864B8), b"\x55")
                            write_word(cpu, 0x801864C0, 0x12345678)
                            cpu.mem_write(physical(GP + 0x37E), b"\x77")
                            write_word(cpu, transition_index, old_index)
                            write_word(cpu, limit, maximum)
                            cpu.reg_write(UC_MIPS_REG_A0, actor)
                            cpu.reg_write(UC_MIPS_REG_A1, 0x801A1300)
                            cpu.reg_write(UC_MIPS_REG_A2, 0)
                            self.call(cpu, 0x800347EC)

                            adjustment = (0, 0x80, 0x40)[control]
                            expected = [
                                ("component",),
                                ("select", actor, adjustment + 0x88, 0, 0, (0, 0x33A, 0x2A7)),
                            ]
                            if selector & 0xFF != 1:
                                expected.append(("reset", 4, 3))
                                if flags_after_reset & 2:
                                    if ((old_index + 30) & 0xFFFFFFFF) < maximum:
                                        expected.extend([
                                            ("transition", 0x27, 4, 0x12345678, 0x12345678,
                                             (0, 0, 0, 0), maximum),
                                            ("clear", actor, 1),
                                        ])
                                    else:
                                        expected.append(("assert", 0x80012958))
                                self.assertEqual(read_word(cpu, 0x801864C0), 0)
                                self.assertEqual(cpu.mem_read(physical(GP + 0x37E), 1), b"\0")
                                self.assertEqual(cpu.mem_read(physical(0x801864B8), 1), b"\1")
                                self.assertEqual(cpu.mem_read(physical(actor + 1), 1),
                                                 bytes((1 if control == 2 else 2,)))
                            else:
                                self.assertEqual(read_word(cpu, 0x801864C0), 0x12345678)
                                self.assertEqual(cpu.mem_read(physical(actor + 1), 1), b"\x66")
                            self.assertEqual(cpu.mem_read(physical(0x801864BC), 1), b"\0")
                            self.assertEqual(calls, expected)

    def test_reset_entry_dispatch_with_real_group_membership(self):
        cpu = self.fresh_cpu()
        groups, first_indices, second_indices = 0x801A1000, 0x801A1100, 0x801A1200
        objects, entry_table, actor = 0x801A1300, 0x801A1400, 0x801A1500
        assets = [0x801A2000 + index * 0x40 for index in range(3)]
        entry_object = 0x801A3000
        calls = []
        state = {"mutate": False}

        def intercept(uc, address, size, data):
            if address == 0x80028764:
                calls.append(("get", uc.reg_read(UC_MIPS_REG_A0)))
                uc.reg_write(UC_MIPS_REG_V0, actor)
            elif address == 0x80044A88:
                calls.append(("check", uc.reg_read(UC_MIPS_REG_A0),
                              uc.reg_read(UC_MIPS_REG_A1)))
                return
            elif address == 0x80052BDC:
                obj = uc.reg_read(UC_MIPS_REG_A0)
                calls.append(("prepare", obj, uc.reg_read(UC_MIPS_REG_A1)))
                if state["mutate"] and obj == assets[0]:
                    write_word(uc, groups + 0x18, 0)
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        write_word(cpu, 0x800BE1B0, groups)
        write_word(cpu, 0x800BE1BC, objects)
        write_word(cpu, 0x800BE1B8, entry_table)
        write_word(cpu, 0x800BE678, 2)
        write_word(cpu, 0x801D795C, 0)
        write_word(cpu, 0x801D7960, 1)
        write_word(cpu, groups, first_indices)
        write_word(cpu, groups + 8, 3)
        write_word(cpu, groups + 0x10, second_indices)
        write_word(cpu, second_indices, 2)
        write_word(cpu, entry_table, entry_object)
        write_word(cpu, actor + 8, assets[1])
        for index, asset in enumerate(assets):
            write_word(cpu, first_indices + index * 4, index)
            write_word(cpu, objects + index * 4, asset)

        for mode, mutate in ((1, False), (1, True), (0, False)):
            with self.subTest(mode=mode, mutate=mutate):
                calls.clear()
                state["mutate"] = mutate
                write_word(cpu, groups + 0x18, 1)
                cpu.reg_write(UC_MIPS_REG_A0, 0)
                cpu.reg_write(UC_MIPS_REG_A1, mode)
                self.call(cpu, 0x80044458)
                expected = [("prepare", entry_object, 1), ("get", 0)]
                if mode:
                    expected.append(("check", 0, groups))
                expected.append(("prepare", assets[0], 1))
                if mode:
                    expected.append(("check", 2, groups))
                if mutate or not mode:
                    expected.append(("prepare", assets[2], 1))
                self.assertEqual(calls, expected)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)

    def test_controller_callback_tagged_slot_sweep(self):
        cpu = self.fresh_cpu()
        table = 0x80189C90
        calls = []
        mutate = False

        def intercept(uc, address, size, data):
            nonlocal mutate
            if address not in (0x80048504, 0x80048C48):
                return
            tag = uc.reg_read(UC_MIPS_REG_A0)
            calls.append((address, tag))
            if mutate and address == 0x80048504:
                mutate = False
                for index, value in ((0, 99), (1, 0xFB), (2, 0xFF), (49, 7)):
                    uc.mem_write(physical(table + index * 44), bytes((value,)))
            uc.reg_write(UC_MIPS_REG_V0, 0)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for value in range(256):
            with self.subTest(value=value):
                cpu.mem_write(physical(table), bytes((0xFF,)) * (50 * 44))
                cpu.mem_write(physical(table), bytes((value,)))
                calls.clear()
                self.call(cpu, 0x80048488)
                tag = value if value < 128 else value | 0xFFFFFF00
                expected = [] if value == 0xFF else [(0x80048504, tag), (0x80048C48, tag)]
                self.assertEqual(calls, expected)

        cpu.mem_write(physical(table), bytes((0xFF,)) * (50 * 44))
        cpu.mem_write(physical(table), b"\5")
        cpu.mem_write(physical(table + 2 * 44), b"\11")
        calls.clear()
        mutate = True
        self.call(cpu, 0x80048488)
        self.assertEqual(calls, [
            (0x80048504, 5), (0x80048C48, 5),
            (0x80048504, 0xFFFFFFFB), (0x80048C48, 0xFFFFFFFB),
            (0x80048504, 7), (0x80048C48, 7),
        ])

    def test_controller_callback_tagged_slot_cleanup(self):
        cpu = self.poll(0)
        table = 0x80189C90
        initial = bytearray((index * 13 + 3) & 0xFF for index in range(50 * 44))
        tags = (0, 1, 127, 128, 255)
        for index in range(50):
            initial[index * 44] = tags[index % len(tags)]
        for flag in (0, 1, 255):
            for value in (0, 1, 127, 128, 255, 256, 0x80000080, 0xFFFFFFFF):
                for count in (0, 1, 50, 0xFFFFFFFF):
                    with self.subTest(flag=flag, value=value, count=count):
                        expected = bytearray(initial)
                        removed = 0
                        if flag:
                            for index in range(50):
                                if expected[index * 44] == (value & 0xFF):
                                    expected[index * 44] = 0xFF
                                    removed += 1
                        cpu.mem_write(physical(table), bytes(initial))
                        cpu.mem_write(physical(0x80189C71), bytes((flag,)))
                        write_word(cpu, GP + 0x4F0, count)
                        cpu.reg_write(UC_MIPS_REG_A0, value)
                        self.call(cpu, 0x80048C48)
                        self.assertEqual(bytes(cpu.mem_read(physical(table), len(initial))),
                                         bytes(expected))
                        self.assertEqual(read_word(cpu, GP + 0x4F0),
                                         (count - removed) & 0xFFFFFFFF)
                        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)

    def test_controller_callback_tagged_slot_operations(self):
        cpu = self.poll(0)
        table = 0x80189C90
        loaded = 0x80189C71
        initial = bytearray((index * 13 + 3) & 0xFF for index in range(50 * 44))
        tags = (0, 1, 127, 128, 255)
        for index in range(50):
            initial[index * 44] = tags[index % len(tags)]
        events = []
        mutate = {"backend": False, "assertion": False}

        def intercept(uc, address, size, data):
            if address in (0x80050534, 0x80050650):
                payload = uc.reg_read(UC_MIPS_REG_A0)
                self.assertGreaterEqual(payload, table + 4)
                self.assertEqual((payload - table - 4) % 44, 0)
                index = (payload - table - 4) // 44
                self.assertLess(index, 50)
                events.append(("call", address, index))
                if mutate["backend"]:
                    mutate["backend"] = False
                    uc.mem_write(physical(loaded), b"\0")
                    uc.mem_write(physical(table + 49 * 44), b"\1")
                uc.reg_write(UC_MIPS_REG_V0, 0xFFFFFFFF if index % 3 == 0 else 0)
            elif address == 0x800556D0:
                args = tuple(uc.reg_read(register) for register in (
                    UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3
                ))
                events.append(("assert", *args))
                if mutate["assertion"] and args[3] == 0x1BF:
                    uc.mem_write(physical(table + 49 * 44), b"\1")
                uc.reg_write(UC_MIPS_REG_V0, 0)
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for flag in (0, 1, 2, 255):
            for value in (0, 1, 127, 128, 255, 256, 0x80000080, 0xFFFFFFFF):
                for address, backend in ((0x800483C0, 0x80050534), (0x80048504, 0x80050650)):
                    with self.subTest(flag=flag, value=value, function=hex(address)):
                        cpu.mem_write(physical(table), bytes(initial))
                        cpu.mem_write(physical(loaded), bytes((flag,)))
                        events.clear()
                        cpu.reg_write(UC_MIPS_REG_A0, value)
                        self.call(cpu, address)
                        expected = []
                        if address == 0x800483C0 and flag != 1:
                            expected.append(("assert", 0, 0x80013730, 0x80013724, 0x1BF))
                        if address == 0x800483C0 or flag:
                            for index in range(50):
                                if initial[index * 44] != (value & 0xFF):
                                    continue
                                expected.append(("call", backend, index))
                                if index % 3 == 0:
                                    line = 0x1C8 if address == 0x800483C0 else 0x1FA
                                    expected.append(("assert", 0, 0x8001370C, 0x80013724, line))
                        self.assertEqual(events, expected)
                        self.assertEqual(bytes(cpu.mem_read(physical(table), len(initial))),
                                         bytes(initial))
                        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0),
                                         1 if address == 0x800483C0 else 0)

        for address, backend in ((0x800483C0, 0x80050534), (0x80048504, 0x80050650)):
            with self.subTest(mutation="backend", function=hex(address)):
                cpu.mem_write(physical(table), bytes(initial))
                cpu.mem_write(physical(loaded), b"\1")
                events.clear()
                mutate["backend"] = True
                cpu.reg_write(UC_MIPS_REG_A0, 1)
                self.call(cpu, address)
                calls = [event for event in events if event[0] == "call"]
                self.assertEqual(calls, [("call", backend, index)
                                         for index in (*range(1, 50, 5), 49)])
                self.assertEqual(bytes(cpu.mem_read(physical(loaded), 1)), b"\0")

        cpu.mem_write(physical(table), bytes(initial))
        cpu.mem_write(physical(loaded), b"\0")
        events.clear()
        mutate["assertion"] = True
        cpu.reg_write(UC_MIPS_REG_A0, 1)
        self.call(cpu, 0x800483C0)
        self.assertEqual(events[0], ("assert", 0, 0x80013730, 0x80013724, 0x1BF))
        self.assertEqual([event for event in events if event[0] == "call"][-1],
                         ("call", 0x80050534, 49))

    def test_grounded_actor_context_getter(self):
        cpu = self.fresh_cpu()
        table = 0x801D5F70
        contents = bytes(range(64))
        cpu.mem_write(physical(table), contents)
        for actor in (0, 1, 0x801E5000, 0xFFFFFFFF):
            for context in (0, 1, 0x801E6000, 0xFFFFFFFF):
                with self.subTest(actor=actor, context=context):
                    cpu.reg_write(UC_MIPS_REG_A0, actor)
                    cpu.reg_write(UC_MIPS_REG_A1, context)
                    self.call(cpu, 0x80026648)
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), table)
                    self.assertEqual(cpu.mem_read(physical(table), len(contents)), contents)

    def test_grounded_level_record_list_getters(self):
        cpu = self.fresh_cpu()
        records, lists, output = 0x801E5000, 0x801E6000, 0x801E7000
        record_values = (-1, 1, 0x7FFFFFFF)
        list_counts = (-32768, -1, 32767)
        list_pointers = (0x801E8000, 0x801E8100, 0x801E8200)
        write_word(cpu, 0x801D89A4, records)
        write_word(cpu, 0x801D89B0, lists)
        for index, value in enumerate(record_values):
            write_word(cpu, records + index * 0x24 + 0x18, value & 0xFFFFFFFF)
            cpu.mem_write(physical(lists + index * 8 + 2), struct.pack("<h", list_counts[index]))
            write_word(cpu, lists + index * 8 + 4, list_pointers[index])

        indices = (-0x80000000, -1, 0, 1, 2, 3, 0x7FFFFFFF)
        for record_count in (-1, 0, 1, 3):
            write_word(cpu, 0x801D8994, record_count & 0xFFFFFFFF)
            for index in indices:
                with self.subTest(record_count=record_count, index=index):
                    write_word(cpu, output, 0x12345678)
                    cpu.reg_write(UC_MIPS_REG_A0, index & 0xFFFFFFFF)
                    cpu.reg_write(UC_MIPS_REG_A1, output)
                    self.call(cpu, 0x80028C84)
                    valid = 0 <= index < record_count
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), int(valid))
                    self.assertEqual(read_word(cpu, output),
                                     (record_values[index] & 0xFFFFFFFF) if valid else 0x12345678)

        ids = (-0x80000000, -1, 0, 1, 2, 3, 4, 0x7FFFFFFF)
        for list_count in (-1, 0, 1, 3):
            write_word(cpu, 0x801D89AC, list_count & 0xFFFFFFFF)
            for list_id in ids:
                with self.subTest(list_count=list_count, list_id=list_id):
                    write_word(cpu, output, 0x12345678)
                    cpu.reg_write(UC_MIPS_REG_A0, list_id & 0xFFFFFFFF)
                    cpu.reg_write(UC_MIPS_REG_A1, output)
                    self.call(cpu, 0x80028CD4)
                    valid = 0 < list_id <= list_count
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0),
                                     list_pointers[list_id - 1] if valid else 0)
                    self.assertEqual(read_word(cpu, output),
                                     (list_counts[list_id - 1] & 0xFFFFFFFF) if valid else 0)

        write_word(cpu, 0x801D89AC, 1)
        write_word(cpu, 0x801D89B0, lists)
        cpu.mem_write(physical(lists + 2), struct.pack("<h", 0x1000))
        write_word(cpu, 0x1004, 0xDEADBEEF)
        cpu.reg_write(UC_MIPS_REG_A0, 1)
        cpu.reg_write(UC_MIPS_REG_A1, 0x801D89B0)
        self.call(cpu, 0x80028CD4)
        self.assertEqual(read_word(cpu, 0x801D89B0), 0x1000)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0xDEADBEEF)

    def test_grounded_level_list_search_live_mutations(self):
        cpu = self.fresh_cpu()
        input_record, indices, actor_one, actor_two = (
            0x801E3000, 0x801E4000, 0x801E5000, 0x801E5100
        )
        cpu.mem_write(physical(indices), struct.pack("<iii", 0, 1, 2))
        cpu.mem_write(physical(actor_one), bytes((0x12, 0)))
        cpu.mem_write(physical(actor_two), bytes((0xF5, 0)))
        scenario = {}
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x80028C84:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 0)
                write_word(uc, uc.reg_read(UC_MIPS_REG_A1), 1)
                uc.reg_write(UC_MIPS_REG_V0, 1)
            elif address == 0x80028CD4:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), 1)
                scenario["count_address"] = uc.reg_read(UC_MIPS_REG_A1)
                write_word(uc, scenario["count_address"], scenario["count"] & 0xFFFFFFFF)
                uc.reg_write(UC_MIPS_REG_V0, indices)
            elif address == 0x80028764:
                index = uc.reg_read(UC_MIPS_REG_A0)
                calls.append(("select", index))
                if index == 1:
                    if scenario["mutation"] == "shorten":
                        write_word(uc, scenario["count_address"], 2)
                    elif scenario["mutation"] == "extend":
                        write_word(uc, scenario["count_address"], 3)
                    elif scenario["mutation"] == "input":
                        write_word(uc, input_record + 0x0C, 2)
                    uc.reg_write(UC_MIPS_REG_V0, actor_one)
                else:
                    self.assertEqual(index, 2)
                    uc.reg_write(UC_MIPS_REG_V0, actor_two)
            elif address == 0x8004B264:
                self.assertEqual(uc.reg_read(UC_MIPS_REG_A0), actor_two)
                calls.append(("flag",))
                uc.reg_write(UC_MIPS_REG_V0, scenario["flag"] & 0xFFFFFFFF)
            elif address == 0x800556D0:
                calls.append(("assert", uc.reg_read(UC_MIPS_REG_A0),
                              uc.reg_read(UC_MIPS_REG_A1), uc.reg_read(UC_MIPS_REG_A2),
                              uc.reg_read(UC_MIPS_REG_A3)))
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        assertions = [("assert", 0, 0x800121F8, 0x800120AC, 0x12CA)]
        cases = [
            (3, "none", 0, 0, [("select", 1), ("select", 2), ("flag",)]),
            (3, "none", 0x101, 1, [("select", 1), ("select", 2), ("flag",)]),
            (3, "none", -256, 0, [("select", 1), ("select", 2), ("flag",)]),
            (3, "shorten", 1, 1, [("select", 1)] + assertions),
            (2, "extend", 0, 0, [("select", 1), ("select", 2), ("flag",)]),
            (3, "input", 1, 1, [("select", 1)] + assertions),
            (-1, "none", 1, 1, assertions),
        ]
        for count, mutation, flag, expected, expected_calls in cases:
            with self.subTest(count=count, mutation=mutation, flag=flag):
                scenario.update(count=count, mutation=mutation, flag=flag)
                write_word(cpu, input_record + 0x0C, 0)
                calls.clear()
                cpu.reg_write(UC_MIPS_REG_A0, input_record)
                self.call(cpu, 0x80027E24)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), expected)
                self.assertEqual(calls, expected_calls)
                if mutation == "input":
                    self.assertEqual(read_word(cpu, input_record + 0x0C), 2)

    def test_grounded_level_list_to_flag_with_real_helpers(self):
        cpu = self.fresh_cpu()
        records, lists, indices = 0x801E0000, 0x801E1000, 0x801E2000
        input_record, actor_one, actor_two = 0x801E3000, 0x801E4000, 0x801E4100
        write_word(cpu, 0x801D8994, 3)
        write_word(cpu, 0x801D89A4, records)
        write_word(cpu, 0x801D89AC, 1)
        write_word(cpu, 0x801D89B0, lists)
        write_word(cpu, input_record + 0x0C, 0)
        write_word(cpu, records + 0x18, 1)
        write_word(cpu, records + 0x24 + 0x1C, actor_one)
        write_word(cpu, records + 0x48 + 0x1C, actor_two)
        cpu.mem_write(physical(lists + 2), struct.pack("<h", 3))
        write_word(cpu, lists + 4, indices)
        cpu.mem_write(physical(indices), struct.pack("<iii", 0, 1, 2))
        cpu.mem_write(physical(actor_one), bytes((0x12, 1)))
        cpu.mem_write(physical(actor_two), bytes((0xF5, 0xFF)))
        assertions = []

        def intercept(uc, address, size, data):
            if address == 0x800556D0:
                assertions.append(tuple(uc.reg_read(reg) for reg in (
                    UC_MIPS_REG_A0, UC_MIPS_REG_A1, UC_MIPS_REG_A2, UC_MIPS_REG_A3
                )))
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for flag in (0, 1, 0x80, 0xFF):
            with self.subTest(flag=flag):
                cpu.mem_write(physical(0x801D5F70 + 0x40 + 0xFF), bytes((flag,)))
                cpu.reg_write(UC_MIPS_REG_A0, input_record)
                assertions.clear()
                self.call(cpu, 0x80027E24)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), int(flag == 0))
                self.assertEqual(assertions, [])

        for empty in (False, True):
            with self.subTest(empty=empty):
                cpu.mem_write(physical(actor_two), bytes((0x12, 0xFF)))
                cpu.mem_write(physical(lists + 2), struct.pack("<h", 0 if empty else 3))
                cpu.reg_write(UC_MIPS_REG_A0, input_record)
                assertions.clear()
                self.call(cpu, 0x80027E24)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 1)
                self.assertEqual(assertions, [(0, 0x800121F8, 0x800120AC, 0x12CA)])

    def test_grounded_state_flag_lookup_after_context_resolution(self):
        cpu = self.fresh_cpu()
        actor, context, table = 0x801E5000, 0x801E6000, 0x801E7000
        selected = {}
        calls = []

        def intercept(uc, address, size, data):
            if address != 0x80026648:
                return
            calls.append((uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1)))
            uc.mem_write(physical(actor + 1), bytes((selected["state"],)))
            uc.reg_write(UC_MIPS_REG_V0, table)
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        for state in range(256):
            for flag in (0, 1, 0x80, 0xFF):
                with self.subTest(state=state, flag=flag):
                    previous = (state + 1) & 0xFF
                    flags = bytearray([0xA5] * (0x40 + 256))
                    flags[0x40 + previous] = 1 if flag == 0 else 0
                    flags[0x40 + state] = flag
                    cpu.mem_write(physical(table), bytes(flags))
                    cpu.mem_write(physical(actor + 1), bytes((previous,)))
                    selected["state"] = state
                    calls.clear()
                    cpu.reg_write(UC_MIPS_REG_A0, actor)
                    cpu.reg_write(UC_MIPS_REG_A1, context)
                    self.call(cpu, 0x8004B264)
                    self.assertEqual(calls, [(actor, context)])
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), int(flag == 0))
                    self.assertEqual(cpu.mem_read(physical(actor + 1), 1), bytes((state,)))

    def test_grounded_event_callback_countdown_and_mutations(self):
        cpu = self.fresh_cpu()
        counter, event = GP + 0x53C, GP + 0x540
        level_count, level_records = 0x801D8994, 0x801D89A4
        record, level_actor, fallback_actor, ignored_actor = (
            0x801E6000, 0x801E5000, 0x801E5100, 0x801E5200
        )
        initial_event, selected_event, replacement_event = 0x801E7000, 0x801E7010, 0x801E7020
        scenario = {}
        calls = []

        def intercept(uc, address, size, data):
            if address == 0x80040FC0:
                calls.append(("select", uc.reg_read(UC_MIPS_REG_A0)))
                write_word(uc, event, scenario["selected_event"])
                uc.reg_write(UC_MIPS_REG_V0, fallback_actor)
            elif address == 0x8004FFEC:
                calls.append(("release", uc.reg_read(UC_MIPS_REG_A0)))
                write_word(uc, scenario["actor"] + 0x10, scenario["release_flags"])
                write_word(uc, scenario["actor"] + 0x14, 0xDEADBEEF)
                write_word(uc, record + 0x1C,
                           fallback_actor if scenario["actor"] == level_actor else level_actor)
                write_word(uc, event, replacement_event)
                if scenario["mutate_counter"]:
                    uc.mem_write(physical(counter), struct.pack("<H", 17))
                uc.reg_write(UC_MIPS_REG_V0, 3)
            elif address == 0x8003539C:
                calls.append((
                    "update", uc.reg_read(UC_MIPS_REG_A0), uc.reg_read(UC_MIPS_REG_A1),
                    read_word(uc, scenario["actor"] + 0x10),
                    read_word(uc, scenario["actor"] + 0x14), read_word(uc, event),
                    struct.unpack("<H", uc.mem_read(physical(counter), 2))[0],
                ))
            else:
                return
            uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        boundaries = (0, 1, 2, 3, 4, 0x7FFE, 0x7FFF, 0x8000, 0x8001, 0x8002, 0xFFFF)
        counts = (-0x80000000, -1, 0, 1, 0x7FFFFFFF)
        rng = random.Random(0x4B2A4)
        cases = [(value, count) for value in boundaries for count in counts]
        cases.extend((rng.getrandbits(16), rng.choice(counts)) for _ in range(1024))

        for value, count in cases:
            with self.subTest(counter=value, level_count=count):
                reduced = (value - 2) & 0xFFFF
                expected_counter = 0 if reduced & 0x8000 else reduced
                first_event = 0 if value & 0x10 else initial_event
                scenario["actor"] = level_actor if count > 0 else fallback_actor
                scenario["selected_event"] = 0 if value & 0x20 else selected_event
                scenario["release_flags"] = 0xFEED0123 ^ value
                scenario["mutate_counter"] = bool(value & 8)
                cpu.mem_write(physical(counter), struct.pack("<H", value))
                write_word(cpu, event, first_event)
                write_word(cpu, level_count, count & 0xFFFFFFFF)
                write_word(cpu, level_records, record)
                write_word(cpu, record + 0x1C, level_actor)
                write_word(cpu, level_actor + 0x10, 0x12345678)
                write_word(cpu, level_actor + 0x14, 0x87654321)
                write_word(cpu, fallback_actor + 0x10, 0x11223344)
                write_word(cpu, fallback_actor + 0x14, 0x44332211)
                write_word(cpu, ignored_actor + 0x10, 0xA5A5A5A5)
                cpu.reg_write(UC_MIPS_REG_A0, ignored_actor)
                cpu.reg_write(UC_MIPS_REG_A1, 0x1234)
                cpu.reg_write(UC_MIPS_REG_A2, 0x5678)
                cpu.reg_write(UC_MIPS_REG_A3, 0xFFFF)
                calls.clear()
                self.call(cpu, 0x8004B2A4)

                self.assertEqual(read_word(cpu, ignored_actor + 0x10), 0xA5A5A5A5)
                if expected_counter != 0:
                    self.assertEqual(calls, [])
                    self.assertEqual(read_word(cpu, event), first_event)
                    self.assertEqual(read_word(cpu, level_actor + 0x10), 0x12345678)
                    self.assertEqual(read_word(cpu, fallback_actor + 0x10), 0x11223344)
                    final_counter = expected_counter
                else:
                    final_counter = 17 if scenario["mutate_counter"] else 0
                    expected_event = first_event if count > 0 else scenario["selected_event"]
                    expected_calls = [] if count > 0 else [("select", 0)]
                    expected_calls.extend([
                        ("release", expected_event),
                        ("update", scenario["actor"], 1, scenario["release_flags"] & ~0x220,
                         0, 0, final_counter),
                    ])
                    self.assertEqual(calls, expected_calls)
                    self.assertEqual(read_word(cpu, event), 0)
                    other = fallback_actor if count > 0 else level_actor
                    self.assertEqual(read_word(cpu, other + 0x10),
                                     0x11223344 if count > 0 else 0x12345678)
                self.assertEqual(struct.unpack("<H", cpu.mem_read(physical(counter), 2))[0],
                                 final_counter)

    def test_original_sqrt_backend_wrappers(self):
        cpu = self.fresh_cpu()
        output = 0x801E5004
        state = {}
        calls = []

        def intercept(uc, address, size, data):
            if address in (0x80076554, 0x800764D0):
                calls.append((address, uc.reg_read(UC_MIPS_REG_A0)))
                write_word(uc, output, 0x12345678)
                uc.reg_write(UC_MIPS_REG_A1, 0x801E6000)
                uc.reg_write(UC_MIPS_REG_V0, state["result"])
                uc.reg_write(UC_MIPS_REG_PC, uc.reg_read(UC_MIPS_REG_RA))

        cpu.hook_add(UC_HOOK_CODE, intercept)
        values = (0x80000000, 0xFFFFEFFF, 0xFFFFFFFF, 0, 1, 4096, 0x7FFFFFFF)
        for wrapper, backend in ((0x8005359C, 0x80076554), (0x800535CC, 0x800764D0)):
            for value in values:
                for result in values:
                    with self.subTest(wrapper=hex(wrapper), value=value, result=result):
                        cpu.mem_write(physical(output - 4), b"\xA5" * 12)
                        cpu.reg_write(UC_MIPS_REG_A0, value)
                        cpu.reg_write(UC_MIPS_REG_A1, output)
                        state["result"] = result
                        calls.clear()
                        self.call(cpu, wrapper)
                        self.assertEqual(calls, [(backend, value)])
                        self.assertEqual(
                            struct.unpack("<3I", cpu.mem_read(physical(output - 4), 12)),
                            (0xA5A5A5A5, result, 0xA5A5A5A5),
                        )
                        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)

    def test_original_integer_sqrt_and_wrapped_vector_length(self):
        cpu = self.fresh_cpu()
        gte = RotationGteModel()
        cpu.hook_add(UC_HOOK_CODE, gte.hook)
        backend_cases = (
            (0, 0), (1, 1), (2, 1), (4, 2), (9, 3),
            (16, 4), (25, 5), (100, 10), (4096, 64),
        )
        for value, expected in backend_cases:
            with self.subTest(backend_input=value):
                cpu.reg_write(UC_MIPS_REG_A0, value)
                self.call(cpu, 0x800764D0)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), expected)

        vector_address, zero_address, output = 0x801E5000, 0x801E5100, 0x801E5204
        cpu.mem_write(physical(zero_address), b"\0" * 12)
        vectors = (
            ((0, 0, 0), 0), ((1, 0, 0), 1), ((3, 4, 0), 5),
            ((-3, -4, 0), 5), ((6, 8, 0), 10), ((65536, 0, 0), 0),
        )
        for vector, expected in vectors:
            cpu.mem_write(physical(vector_address), struct.pack("<3i", *vector))
            for address in (0x80053904, 0x80062D4C):
                with self.subTest(vector=vector, address=hex(address)):
                    cpu.mem_write(physical(output - 4), b"\xA5" * 12)
                    cpu.reg_write(UC_MIPS_REG_A0, vector_address)
                    cpu.reg_write(UC_MIPS_REG_A1, output if address == 0x80053904 else zero_address)
                    if address == 0x80062D4C:
                        cpu.reg_write(UC_MIPS_REG_A2, output)
                    self.call(cpu, address)
                    self.assertEqual(
                        struct.unpack("<3I", cpu.mem_read(physical(output - 4), 12)),
                        (0xA5A5A5A5, expected, 0xA5A5A5A5),
                    )
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)
        self.assertEqual(gte.leading_counts, len(backend_cases) + 2 * len(vectors))

    def test_original_sqrt_backends_with_signed_and_wrapped_inputs(self):
        cpu = self.fresh_cpu()
        gte = RotationGteModel()
        cpu.hook_add(UC_HOOK_CODE, gte.hook)
        original_domain = bytes(cpu.mem_read(physical(SQRT_LOOKUP_DOMAIN), 1024))
        rng = random.Random(0x76554)
        synthetic_domain = bytes(rng.getrandbits(8) for _ in range(1024))
        boundaries = {0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFE, 0xFFFFFFFF}
        for bit in range(32):
            for delta in (-1, 0, 1):
                sample = ((1 << bit) + delta) & 0xFFFFFFFF
                boundaries.update((sample, (~sample) & 0xFFFFFFFF))
        cases = sorted(boundaries)
        cases.extend(rng.getrandbits(32) for _ in range(8192))

        for name, domain in (("original", original_domain), ("synthetic", synthetic_domain)):
            cpu.mem_write(physical(SQRT_LOOKUP_DOMAIN), domain)
            for address, q12 in ((0x800764D0, False), (0x80076554, True)):
                for value in cases:
                    with self.subTest(domain=name, backend=hex(address), value=value):
                        cpu.reg_write(UC_MIPS_REG_A0, value)
                        self.call(cpu, address)
                        self.assertEqual(
                            cpu.reg_read(UC_MIPS_REG_V0),
                            sqrt_backend_reference(value, domain, q12),
                        )
            self.assertEqual(bytes(cpu.mem_read(physical(SQRT_LOOKUP_DOMAIN), 1024)), domain)
        self.assertEqual(gte.leading_counts, len(cases) * 4)

    def test_original_fixed_point_scalar_product(self):
        cpu = self.fresh_cpu()
        output = 0x801E5000
        boundaries = (0, 1, 0xFFF, 0x1000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        rng = random.Random(0x1007C)
        cases = [(left, right) for left in boundaries for right in boundaries]
        cases.extend((rng.getrandbits(32), rng.getrandbits(32)) for _ in range(512))

        def signed(value):
            return value - 0x100000000 if value & 0x80000000 else value

        for left, right in cases:
            with self.subTest(left=left, right=right):
                cpu.mem_write(physical(output), b"\xA5" * 12)
                cpu.reg_write(UC_MIPS_REG_A0, left)
                cpu.reg_write(UC_MIPS_REG_A1, right)
                cpu.reg_write(UC_MIPS_REG_A2, output + 4)
                self.call(cpu, 0x8001007C)
                expected = ((signed(left) * signed(right)) >> 12) & 0xFFFFFFFF
                self.assertEqual(bytes(cpu.mem_read(physical(output), 12)),
                                 b"\xA5" * 4 + struct.pack("<I", expected) + b"\xA5" * 4)
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)

    def test_original_fixed_point_vector_products(self):
        cpu = self.poll(0)
        memory = 0x801E5000
        rng = random.Random(0xB3D)
        boundaries = (0, 1, 0xFFF, 0x1000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        layouts = ((0, 3, 6), (0, 3, 0), (0, 3, 3), (0, 2, 1))

        def signed(value):
            return value - 0x100000000 if value & 0x80000000 else value

        def product(left, right):
            return ((signed(left) * signed(right)) >> 12) & 0xFFFFFFFF

        for iteration in range(256):
            if iteration < 128:
                values = [boundaries[(iteration + index * 3) % len(boundaries)]
                          for index in range(9)]
            else:
                values = [rng.getrandbits(32) for _ in range(9)]
            for left, right, output in layouts:
                for address in (0x80010038, 0x800100A0, ROTATION_CROSS):
                    with self.subTest(iteration=iteration, layout=(left, right, output),
                                      function=hex(address)):
                        expected = values.copy()
                        if address == 0x80010038:
                            for index in range(3):
                                expected[output + index] = (
                                    expected[left + index] + expected[right + index]
                                ) & 0xFFFFFFFF
                        elif address == 0x800100A0:
                            expected[output] = sum(
                                product(expected[left + index], expected[right + index])
                                for index in range(3)
                            ) & 0xFFFFFFFF
                        else:
                            components = ((1, 2, 2, 1), (2, 0, 0, 2), (0, 1, 1, 0))
                            for index, (left_a, right_a, left_b, right_b) in enumerate(components):
                                expected[output + index] = (
                                    product(expected[left + left_a], expected[right + right_a])
                                    - product(expected[left + left_b], expected[right + right_b])
                                ) & 0xFFFFFFFF
                        cpu.mem_write(physical(memory), struct.pack("<9I", *values))
                        cpu.reg_write(UC_MIPS_REG_A0, memory + 4 * left)
                        cpu.reg_write(UC_MIPS_REG_A1, memory + 4 * right)
                        cpu.reg_write(UC_MIPS_REG_A2, memory + 4 * output)
                        self.call(cpu, address)
                        self.assertEqual(struct.unpack("<9I", cpu.mem_read(physical(memory), 36)),
                                         tuple(expected))
                        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)

    def test_original_vector_add_overlapping_words(self):
        cpu = self.fresh_cpu()
        memory = 0x801E5000
        rng = random.Random(0x10038)
        boundaries = (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
        for iteration in range(8):
            values = (
                [boundaries[(iteration + index) % len(boundaries)] for index in range(9)]
                if iteration < 4 else [rng.getrandbits(32) for _ in range(9)]
            )
            for left in range(7):
                for right in range(7):
                    for output in range(7):
                        with self.subTest(iteration=iteration, left=left, right=right, output=output):
                            expected = values.copy()
                            for index in range(3):
                                expected[output + index] = (
                                    expected[left + index] + expected[right + index]
                                ) & 0xFFFFFFFF
                            cpu.mem_write(physical(memory), struct.pack("<9I", *values))
                            cpu.reg_write(UC_MIPS_REG_A0, memory + 4 * left)
                            cpu.reg_write(UC_MIPS_REG_A1, memory + 4 * right)
                            cpu.reg_write(UC_MIPS_REG_A2, memory + 4 * output)
                            self.call(cpu, 0x80010038)
                            self.assertEqual(struct.unpack("<9I", cpu.mem_read(physical(memory), 36)),
                                             tuple(expected))
                            self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)

    def test_original_basis_transform_matches_fixed_point_model(self):
        cpu = self.fresh_cpu()
        basis_address, input_address, output_address = (
            0x801E5000, 0x801E6000, 0x801E7000
        )
        identity = (4096, 0, 0, 0, 4096, 0, 0, 0, 4096)
        quarter_turn = (0, -4096, 0, 4096, 0, 0, 0, 0, 4096)
        extreme = (-32768, 32767, -1, 32767, -32768, 1, -1, 32767, -32768)
        rng = random.Random(0x77298)
        matrices = (
            identity,
            quarter_turn,
            extreme,
            tuple(rng.randint(-32768, 32767) for _ in range(9)),
        )
        boundaries = (
            0, 1, -1, 32767, 32768, -32768, -32769,
            0x7FFFFFFF, -0x80000000,
        )
        vectors = [
            (value, boundaries[(index + 3) % len(boundaries)],
             boundaries[(index + 6) % len(boundaries)])
            for index, value in enumerate(boundaries)
        ]
        vectors.extend(
            tuple(rng.randint(-0x80000000, 0x7FFFFFFF) for _ in range(3))
            for _ in range(24)
        )
        sentinel = 0x5A5AA5A5
        gte = RotationGteModel()
        cpu.hook_add(UC_HOOK_CODE, gte.hook)
        call_count = 0

        for matrix_index, matrix in enumerate(matrices):
            cpu.mem_write(
                physical(basis_address),
                struct.pack("<9h H 3i", *matrix, 0xAAAA, 17, -23, 41),
            )
            for vector_index, vector in enumerate(vectors):
                cpu.mem_write(
                    physical(input_address),
                    struct.pack("<4i", *vector, sentinel),
                )
                cpu.mem_write(
                    physical(output_address),
                    struct.pack("<4I", 0xA5A5A5A5, 0xA5A5A5A5,
                                0xA5A5A5A5, sentinel),
                )
                cpu.reg_write(UC_MIPS_REG_A0, basis_address)
                cpu.reg_write(UC_MIPS_REG_A1, input_address)
                cpu.reg_write(UC_MIPS_REG_A2, output_address)
                with self.subTest(matrix=matrix_index, vector=vector_index):
                    self.call(cpu, 0x80077298)
                    actual = struct.unpack(
                        "<4i", cpu.mem_read(physical(output_address), 16)
                    )
                    self.assertEqual(actual[:3], gte_basis_transform(matrix, vector))
                    self.assertEqual(actual[3], sentinel)
                    self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), output_address)
                    call_count += 1
                    self.assertEqual(gte.basis_transform_passes, call_count * 2)

    def test_original_rotation_generator(self):
        cpu = self.fresh_cpu()
        rng = random.Random(0x77C50)
        original_table = bytes(cpu.mem_read(physical(ROTATION_TRIG_TABLE), 4096 * 4))
        synthetic_table = bytearray(rng.getrandbits(8) for _ in range(4096 * 4))
        for index, word in (
                (0, 0x7FFF8000), (1, 0x80007FFF), (1024, 0),
                (2048, 0x0001FFFF), (3072, 0xF0001000), (4095, 0x7FFF0000)):
            struct.pack_into("<I", synthetic_table, index * 4, word)
        boundaries = (-32768, -4096, -3072, -2048, -1024, -1, 0,
                      1, 1024, 2048, 3072, 4095, 32767)
        cases = [
            (x, y, z) for x in boundaries for y in boundaries for z in boundaries
        ]
        cases.extend(
            tuple(rng.randint(-32768, 32767) for _ in range(3))
            for _ in range(512)
        )
        base, output_offset = 0x801E5000, 32

        for table_name, table in (("original", original_table), ("synthetic", bytes(synthetic_table))):
            cpu.mem_write(physical(ROTATION_TRIG_TABLE), table)
            for case_index, angles in enumerate(cases):
                for angle_offset in (0, 32, 34, 44, 48, 50):
                    initial = bytearray((index * 37 + case_index) & 0xFF for index in range(96))
                    struct.pack_into("<3h", initial, angle_offset, *angles)
                    expected = bytearray(initial)
                    struct.pack_into(
                        "<9H", expected, output_offset,
                        *rotation_generator_reference(angles, table),
                    )
                    cpu.mem_write(physical(base), bytes(initial))
                    cpu.reg_write(UC_MIPS_REG_A0, base + angle_offset)
                    cpu.reg_write(UC_MIPS_REG_A1, base + output_offset)
                    with self.subTest(table=table_name, case=case_index, angle_offset=angle_offset):
                        self.call(cpu, ROTATION_GENERATOR)
                        self.assertEqual(bytes(cpu.mem_read(physical(base), len(expected))), bytes(expected))
                        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), base + output_offset)

    def test_original_rotation_to_basis_copy_wrapper(self):
        cpu = self.fresh_cpu()
        base = 0x801E5800
        output_offset = 32
        table = bytes(cpu.mem_read(physical(ROTATION_TRIG_TABLE), 4096 * 4))
        cases = (
            (0, 0, 0),
            (0x1234FFFF, 0x80000001, 0xFEDC8000),
            (0x7FFFFFFF, 0x12345678, 0xABCDEFFF),
            (0x00001000, 0xFFFFF800, 0x7FFF0001),
        )

        cpu.mem_write(physical(base), bytes(range(96)))
        cpu.reg_write(UC_MIPS_REG_A0, 0)
        cpu.reg_write(UC_MIPS_REG_A1, 0x90000000)
        self.call(cpu, 0x80054978)
        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0x18)

        for rotation_offset, rotations in (
            (0, cases[0]),
            (0, cases[1]),
            (output_offset, cases[2]),
            (output_offset + 4, cases[3]),
        ):
            initial = bytearray((index * 29 + rotation_offset) & 0xFF for index in range(96))
            struct.pack_into("<3I", initial, rotation_offset, *rotations)
            raw_angles = tuple(
                struct.unpack_from("<H", initial, rotation_offset + index * 4)[0]
                for index in range(3)
            )
            angles = (
                (-raw_angles[0]) & 0xFFFF,
                raw_angles[1],
                (-raw_angles[2]) & 0xFFFF,
            )
            expected = bytearray(initial)
            struct.pack_into(
                "<9H",
                expected,
                output_offset,
                *rotation_generator_reference(angles, table),
            )
            cpu.mem_write(physical(base), bytes(initial))
            cpu.reg_write(UC_MIPS_REG_A0, base + output_offset)
            cpu.reg_write(UC_MIPS_REG_A1, base + rotation_offset)
            with self.subTest(rotation_offset=rotation_offset, rotations=rotations):
                self.call(cpu, 0x80054978)
                self.assertEqual(
                    bytes(cpu.mem_read(physical(base), len(expected))),
                    bytes(expected),
                )
                self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), 0)

    def test_original_rotation_composition_with_modeled_gte(self):
        z_turn = (0, -4096, 0, 4096, 0, 0, 0, 0, 4096)
        x_turn = (4096, 0, 0, 0, 0, -4096, 0, 4096, 0)
        cases = (
            (z_turn, x_turn, (0, 0, 4096, 4096, 0, 0, 0, 4096, 0), False),
            ((32767,) * 9, (32767,) * 9, (32767,) * 9, False),
            ((1, 0, 0, 0, 4096, 0, 0, 0, 4096),
             (-1, 0, 0, 0, 4096, 0, 0, 0, 4096),
             (-1, 0, 0, 0, 4096, 0, 0, 0, 4096), False),
            ((4096, 0, 0, 0, 4096, 0, 0, 0, 4096),
             (4096, 0, 0, 0, 4096, 0, 0, 0, -4096),
             (4096, 0, 0, 0, 4096, 0, 0, 0, -4096), False),
            (z_turn, z_turn, (-4096, 0, 0, 0, -4096, 0, 0, 0, 4096), True),
        )
        for basis, rotation, expected, alias in cases:
            with self.subTest(basis=basis, rotation=rotation, alias=alias):
                cpu = self.poll(0)
                first = 0x801E5000
                second = first if alias else 0x801E6000
                cpu.mem_write(physical(first), struct.pack("<9h H 3i", *basis, 0xAAAA, 1, 2, 3))
                if not alias:
                    cpu.mem_write(physical(second), struct.pack("<9h H 3i", *rotation, 0xBBBB, 4, 5, 6))
                original_translation = bytes(cpu.mem_read(physical(second + 20), 12))
                cpu.reg_write(UC_MIPS_REG_A0, first)
                cpu.reg_write(UC_MIPS_REG_A1, second)
                gte = RotationGteModel()
                cpu.hook_add(UC_HOOK_CODE, gte.hook)
                self.call(cpu, ROTATION_MULTIPLIER)
                self.assertEqual(struct.unpack("<9h", cpu.mem_read(physical(second), 18)), expected)
                self.assertEqual(read_word(cpu, second + 16) >> 16, 0xFFFF if expected[8] < 0 else 0)
                self.assertEqual(bytes(cpu.mem_read(physical(second + 20), 12)), original_translation)
                self.assertEqual(gte.rotations, 3)

    def test_original_rotation_composition_outputs_and_overlaps(self):
        identity = (4096, 0, 0, 0, 4096, 0, 0, 0, 4096)
        turn = (0, -4096, 0, 4096, 0, 0, 0, 0, 4096)
        cases = [
            (identity, turn),
            (turn, identity),
            ((32767,) * 9, (32767,) * 9),
            ((-32768,) * 9, (32767,) * 9),
            ((1, 0, 0, 0, 4096, 0, 0, 0, 4096),
             (-1, 0, 0, 0, 4096, 0, 0, 0, 4096)),
        ]
        rng = random.Random(0x7699C)
        cases.extend(
            (
                tuple(rng.randint(-32768, 32767) for _ in range(9)),
                tuple(rng.randint(-32768, 32767) for _ in range(9)),
            )
            for _ in range(32)
        )
        offsets = ((8, 72), (8, 8), (8, 12), (12, 8), (8, 20), (20, 8))
        base = 0x801E5000
        cpu = self.fresh_cpu()
        gte = RotationGteModel()
        cpu.hook_add(UC_HOOK_CODE, gte.hook)

        for address, output_first in ((0x80076890, True), (0x8007699C, False)):
            for case_index, (first_matrix, second_matrix) in enumerate(cases):
                for first_offset, second_offset in offsets:
                    initial = bytearray((index * 37 + case_index) & 0xFF for index in range(128))
                    struct.pack_into(
                        "<9h H 3i", initial, first_offset,
                        *first_matrix, 0xAAAA, 11, -22, 33,
                    )
                    struct.pack_into(
                        "<9h H 3i", initial, second_offset,
                        *second_matrix, 0xBBBB, -44, 55, -66,
                    )
                    first = struct.unpack_from("<9h", initial, first_offset)
                    second = struct.unpack_from("<9h", initial, second_offset)
                    columns = [
                        gte_rotate_vector(first, (second[column], second[column + 3], second[column + 6]))
                        for column in range(3)
                    ]
                    result = tuple(columns[column][row] for row in range(3) for column in range(3))
                    output_offset = first_offset if output_first else second_offset
                    expected = bytearray(initial)
                    struct.pack_into(
                        "<9h H", expected, output_offset, *result,
                        0xFFFF if result[8] < 0 else 0,
                    )
                    cpu.mem_write(physical(base), bytes(initial))
                    cpu.reg_write(UC_MIPS_REG_A0, base + first_offset)
                    cpu.reg_write(UC_MIPS_REG_A1, base + second_offset)
                    previous_rotations = gte.rotations
                    with self.subTest(address=hex(address), case=case_index,
                                      first=first_offset, second=second_offset):
                        self.call(cpu, address)
                        self.assertEqual(bytes(cpu.mem_read(physical(base), len(expected))), bytes(expected))
                        self.assertEqual(cpu.reg_read(UC_MIPS_REG_V0), base + output_offset)
                        self.assertEqual(gte.rotations, previous_rotations + 3)

    def test_queued_input_reaches_orientation_with_modeled_gte(self):
        records, actor, holder, component, move_info = (
            0x801E0000, 0x801E1000, 0x801E2000, 0x801E3000, 0x801E4000
        )
        expected_trace = [
            "queue processor", "queue-update gate", "queue snapshot", "controller callback",
            "queue processor", "queue-update gate", "queue snapshot", "heading handler",
            "grounded handler", "grounded turn", "movement consumer", "rotation wrapper",
            "rotation generator", "rotation composition", "rotation conversion",
            "cross product", "cross product",
        ]
        for bit, expected_turn in ((13, 11), (15, -11)):
            with self.subTest(bit=bit):
                cpu = self.poll(1 << bit)
                for address in NOOP_BACKENDS:
                    cpu.mem_write(physical(address), struct.pack("<II", 0x03E00008, 0))
                cpu.mem_write(physical(QUEUE_UPDATE_FLAG), b"\0")
                cpu.mem_write(physical(0x80052ABC), struct.pack("<III", 0xACA00000, 0x03E00008, 0))
                write_word(cpu, records + 0x1C, actor)
                write_word(cpu, actor + 0x04, 0x80)
                write_word(cpu, actor + 0x10, 0x20)
                write_word(cpu, actor + 0x1C, holder)
                write_word(cpu, actor + 0x28, move_info)
                write_word(cpu, holder + 0x14, component)
                original_component = struct.pack(
                    "<9H H 3i", 4096, 0, 0, 0, 4096, 0, 0, 0, 4096, 0xA5A5, 123, -456, 789
                )
                cpu.mem_write(physical(component), original_component)
                self.activate_heading_table(cpu)
                write_word(cpu, 0x801D8994, 1)
                write_word(cpu, 0x801D89A4, records)
                write_word(cpu, MAPPING_ROW, 0)
                cpu.mem_write(physical(ROW_SELECTOR), b"\0\0")
                write_word(cpu, BITMAP_STATE, 0)
                cpu.mem_write(physical(0x800BE0E0), bytes((bit == 13,)))

                seen = []
                angles_seen = []
                gte = RotationGteModel()

                def trace(uc, address, size, data):
                    if address in TRACE:
                        seen.append(TRACE[address])
                    if address == ROTATION_GENERATOR:
                        angle_ptr = uc.reg_read(UC_MIPS_REG_A0)
                        angles_seen.append(struct.unpack("<3H", uc.mem_read(physical(angle_ptr), 6)))

                cpu.hook_add(UC_HOOK_CODE, trace)
                cpu.hook_add(UC_HOOK_CODE, gte.hook)
                self.call(cpu, 0x8001B1E4)
                self.assertEqual(seen, expected_trace[:4])
                self.assertEqual(read_word(cpu, PENDING_EVENTS), 1)
                cpu.mem_write(physical(0x800BDED8), struct.pack("<H", 1))
                write_word(cpu, 0x80185A6C, actor)
                self.call(cpu, 0x8001B1E4)
                self.assertEqual(seen, expected_trace)
                self.assertEqual(angles_seen, [(0, expected_turn & 0xFFFF, 0)])
                self.assertEqual(
                    struct.unpack("<3I", cpu.mem_read(physical(move_info + 0x48), 12)),
                    (0, 0, 0),
                )
                self.assertEqual((gte.rotations, gte.leading_counts), (3, 3))
                result = bytes(cpu.mem_read(physical(component), 32))
                marker = 69 if bit == 13 else -69
                self.assertEqual(struct.unpack_from("<9h", result),
                                 (4103, 0, marker, 0, 4096, 0, -marker, 0, 4103))
                self.assertEqual(result[18:], original_component[18:])


if __name__ == "__main__":
    unittest.main()
