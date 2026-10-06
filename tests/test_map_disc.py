import hashlib
from pathlib import Path
import struct
import tempfile
import unittest

from tools.map_disc import inspect_file, matches_startup, render_map, scan_disc
from tools.progress import ROOT, load_json


class DiscMapTests(unittest.TestCase):
    def test_scans_every_file_and_empty_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "COMMON").mkdir()
            (root / "EMPTY").mkdir()
            (root / "COMMON" / "sample.TZP").write_bytes(b"synthetic resource")
            (root / "empty.DAT").write_bytes(b"")
            inventory = scan_disc(root)
            self.assertEqual(inventory["directories"], [".", "COMMON", "EMPTY"])
            self.assertEqual(inventory["file_count"], 2)
            self.assertEqual(inventory["total_file_bytes"], len(b"synthetic resource"))
            self.assertEqual(inventory["files"][1]["sha256"], hashlib.sha256(b"").hexdigest())
            self.assertEqual(inventory, scan_disc(root))

    def test_detects_embedded_magic_across_stream_chunks_without_claiming_code(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "synthetic.HOG"
            offset = 1024 * 1024 - 3
            path.write_bytes(bytes(offset) + b"PS-X EXE" + bytes(16))
            record = inspect_file(path, "synthetic.HOG")
            self.assertEqual(record["psx_magic_offsets"], [offset])
            self.assertEqual(record["format"], "unresolved")

    def test_rejects_source_repository_as_disc_input(self):
        with self.assertRaises(ValueError):
            scan_disc(ROOT)

    def test_readable_map_matches_complete_tracked_inventory(self):
        inventory = load_json(ROOT / "config" / "disc-map.json")
        coverage = load_json(ROOT / "config" / "executable-coverage.json")
        self.assertEqual((ROOT / "docs" / "disc-map.md").read_text(encoding="utf-8"), render_map(inventory, coverage))

    def test_startup_pattern_allows_immediates_but_not_opcode_or_control_changes(self):
        words = [0] * 43
        words[0] = 0x3C028009
        words[1] = 0x24421234
        words[2] = 0x0C010000
        words[-1] = 0x4D
        reference = struct.pack("<43I", *words)
        words[0] = 0x3C02800A
        words[1] = 0x24425678
        words[2] = 0x0C020000
        self.assertTrue(matches_startup(reference, struct.pack("<43I", *words)))
        words[1] = 0x34425678
        self.assertFalse(matches_startup(reference, struct.pack("<43I", *words)))
        self.assertFalse(matches_startup(reference, reference[:-4]))

    def test_tracked_maps_cover_all_executable_files_and_payload_spans(self):
        inventory = load_json(ROOT / "config" / "disc-map.json")
        manifest = load_json(ROOT / "config" / "executable-map.json")
        coverage = load_json(ROOT / "config" / "executable-coverage.json")
        scope = load_json(ROOT / "config" / "startup-units.json")
        paths = [entry["path"] for entry in inventory["files"]]
        self.assertEqual(paths, sorted(set(paths)))
        self.assertEqual(len(paths), inventory["file_count"])
        self.assertEqual(sum(entry["size"] for entry in inventory["files"]), inventory["total_file_bytes"])
        originals = {entry["path"]: entry for entry in inventory["files"] if entry["format"] == "psx_executable"}
        self.assertEqual(set(originals), {entry["filename"] for entry in manifest["executables"]})
        self.assertEqual(set(originals), {unit["filename"] for unit in scope["units"]})
        self.assertEqual(set(originals), {module["path"] for module in coverage["modules"]})
        for entry in manifest["executables"]:
            self.assertEqual(entry["sha256"], originals[entry["filename"]]["sha256"])
        for module in coverage["modules"]:
            cursor = int(module["load_start"], 16)
            for span in module["spans"]:
                self.assertEqual(int(span["start"], 16), cursor)
                cursor = int(span["end"], 16)
                self.assertEqual(span["size"], cursor - int(span["start"], 16))
                self.assertEqual(int(span["file_end"], 16) - int(span["file_start"], 16), span["size"])
            self.assertEqual(cursor, int(module["load_end"], 16))
            self.assertEqual(sum(span["size"] for span in module["spans"]), module["payload_bytes"])
            self.assertEqual(module["known_code_bytes"] + module["unmapped_payload_bytes"], module["payload_bytes"])
        self.assertEqual(len(coverage["modules"]), coverage["module_count"])
        self.assertEqual(sum(module["payload_bytes"] for module in coverage["modules"]), coverage["payload_bytes"])
        self.assertEqual(sum(module["known_code_bytes"] for module in coverage["modules"]), coverage["known_code_bytes"])


if __name__ == "__main__":
    unittest.main()