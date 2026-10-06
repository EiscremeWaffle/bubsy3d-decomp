import hashlib
from pathlib import Path
import struct
import tempfile
import unittest

from tools.verify_original import HEADER_SIZE, inspect_executable, verify_executable


class VerifyOriginalTests(unittest.TestCase):
    def make_executable(self):
        data = bytearray(HEADER_SIZE + 16)
        data[:8] = b"PS-X EXE"
        struct.pack_into("<4I", data, 0x10, 0x80010000, 0, 0x80010000, 16)
        return bytes(data)

    def test_valid_header(self):
        actual = inspect_executable(self.make_executable())
        self.assertEqual(actual["load_address"], "0x80010000")
        self.assertEqual(actual["payload_size"], "0x00000010")

    def test_rejects_invalid_magic_and_short_header(self):
        for data in (b"PS-X EXE", bytes(HEADER_SIZE)):
            with self.subTest(size=len(data)), self.assertRaises(ValueError):
                inspect_executable(data)

    def test_rejects_truncated_payload(self):
        with self.assertRaisesRegex(ValueError, "beyond the file"):
            inspect_executable(self.make_executable()[:-1])

    def test_rejects_entry_outside_payload(self):
        data = bytearray(self.make_executable())
        struct.pack_into("<I", data, 0x10, 0x80010010)
        with self.assertRaisesRegex(ValueError, "outside"):
            inspect_executable(data)

    def test_verifies_hash_and_rejects_modified_payload(self):
        data = self.make_executable()
        expected = {
            "size": len(data),
            "sha1": hashlib.sha1(data).hexdigest(),
            "sha256": hashlib.sha256(data).hexdigest(),
            **inspect_executable(data),
        }
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "synthetic.psx"
            path.write_bytes(data)
            self.assertEqual(verify_executable(path, expected)["size"], len(data))
            modified = bytearray(data)
            modified[-1] = 1
            path.write_bytes(modified)
            with self.assertRaisesRegex(ValueError, "mismatch"):
                verify_executable(path, expected)


if __name__ == "__main__":
    unittest.main()