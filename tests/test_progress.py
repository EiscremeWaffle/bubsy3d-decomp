import copy
import hashlib
from pathlib import Path
import struct
import tempfile
import unittest

from tools.fetch_tools import download_checked
from tools.progress import SCOPE_PATH, SNAPSHOT_PATH, extract_startup, load_json, stage_report, unit_symbols, validate_report
from tools.verify_original import HEADER_SIZE, ROOT


class ProgressTests(unittest.TestCase):
    def setUp(self):
        self.scope = load_json(SCOPE_PATH)
        self.report = load_json(SNAPSHOT_PATH)
        self.expected_code_bytes = sum(int(symbol["end"], 16) - int(symbol["start"], 16) for unit in self.scope["units"] for symbol in unit_symbols(unit))

    def test_real_snapshot_has_verified_getter_match(self):
        self.assertEqual(validate_report(self.report, self.scope), self.expected_code_bytes)
        expected_symbols = load_json(ROOT / "config" / "base-matches.json")["units"]["l0/functions"]["symbols"]
        report_unit = next(unit for unit in self.report["units"] if unit["name"] == "l0/functions")
        for expected in expected_symbols:
            function = next(function for function in report_unit["functions"] if function["name"] == expected["name"])
            with self.subTest(symbol=expected["name"]):
                self.assertEqual(int(function["size"]), expected["size"])
                self.assertEqual(function["fuzzy_match_percent"], 100)
        self.assertEqual(int(self.report["measures"]["matched_code"]), sum(symbol["size"] for symbol in expected_symbols))

    def test_stages_exact_snapshot_with_discoverable_filename(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "publish" / "report.json"
            self.assertEqual(stage_report(output), output)
            self.assertEqual(output.read_bytes(), SNAPSHOT_PATH.read_bytes())
            self.assertEqual(validate_report(load_json(output), self.scope), self.expected_code_bytes)

    def test_rejects_undiscoverable_staged_filename(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "startup-report.json"
            with self.assertRaisesRegex(ValueError, "filename report.json"):
                stage_report(output)
            self.assertFalse(output.exists())

    def test_rejects_missing_or_duplicate_units(self):
        for units in (self.report["units"][:1], [self.report["units"][0]] * 2):
            report = copy.deepcopy(self.report)
            report["units"] = units
            with self.subTest(units=len(units)), self.assertRaises(ValueError):
                validate_report(report, self.scope)

    def test_rejects_wrong_code_denominator(self):
        for location in ("total", "unit"):
            report = copy.deepcopy(self.report)
            measures = report["measures"] if location == "total" else report["units"][0]["measures"]
            measures["total_code"] = "495616"
            with self.subTest(location=location), self.assertRaises(ValueError):
                validate_report(report, self.scope)

    def test_rejects_invented_progress_in_target_only_unit(self):
        report = copy.deepcopy(self.report)
        unit = next(unit for unit in report["units"] if unit["name"] == "l1/functions")
        unit["measures"]["matched_code"] = 1
        with self.assertRaisesRegex(ValueError, "without a compiled base"):
            validate_report(report, self.scope)

    def test_rejects_missing_expected_exact_match(self):
        report = copy.deepcopy(self.report)
        unit = next(unit for unit in report["units"] if unit["name"] == "l0/functions")
        function = next(function for function in unit["functions"] if function["name"] == "level_get_player_model_global")
        function["fuzzy_match_percent"] = 0
        with self.assertRaisesRegex(ValueError, "byte-exact source match"):
            validate_report(report, self.scope)

    def test_rejects_match_totals_below_expected_base(self):
        report = copy.deepcopy(self.report)
        report["measures"]["matched_code"] = 15
        with self.assertRaisesRegex(ValueError, "omits the verified C match"):
            validate_report(report, self.scope)

    def test_rejects_completed_code_even_with_partial_base(self):
        report = copy.deepcopy(self.report)
        report["measures"]["complete_code"] = 1
        with self.assertRaisesRegex(ValueError, "unsupported complete code"):
            validate_report(report, self.scope)

    def test_rejects_completed_target_unit(self):
        report = copy.deepcopy(self.report)
        report["units"][0]["metadata"]["complete"] = True
        with self.assertRaises(ValueError):
            validate_report(report, self.scope)

    def test_rejects_wrong_function_symbol_or_size(self):
        for field, value in (("name", "unrelated_function"), ("size", "4"), ("fuzzy_match_percent", 100)):
            report = copy.deepcopy(self.report)
            report["units"][0]["functions"][0][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                validate_report(report, self.scope)

    def test_extracts_aligned_entry_range_ending_in_break(self):
        expected = {"entry_point": "0x80010000", "load_address": "0x80010000", "payload_size": "0x10"}
        unit = {"start": "0x80010000", "end": "0x80010010"}
        code = struct.pack("<4I", 0, 0, 0, 0x4D)
        data = bytes(HEADER_SIZE) + code
        self.assertEqual(extract_startup(data, expected, unit), code)
        for end in ("0x8001000F", "0x80010014", "0x8001000C"):
            with self.subTest(end=end), self.assertRaises(ValueError):
                extract_startup(data, expected, {**unit, "end": end})

    def test_rejects_start_other_than_header_entry(self):
        expected = {"entry_point": "0x80010000", "load_address": "0x80010000", "payload_size": "0x10"}
        with self.assertRaises(ValueError):
            extract_startup(bytes(HEADER_SIZE + 16), expected, {"start": "0x80010004", "end": "0x80010010"})

    def test_rejects_wrong_cached_tool_checksum(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "cached-tool"
            path.write_bytes(b"synthetic tool")
            digest = hashlib.sha256(path.read_bytes()).hexdigest()
            self.assertEqual(download_checked("https://example.invalid/tool", digest, path), path)
            with self.assertRaisesRegex(ValueError, "SHA256"):
                download_checked("https://example.invalid/tool", "0" * 64, path)


if __name__ == "__main__":
    unittest.main()