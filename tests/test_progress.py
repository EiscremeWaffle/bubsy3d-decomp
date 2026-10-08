import copy
import hashlib
import io
from pathlib import Path
import struct
import tarfile
import tempfile
import unittest
from unittest.mock import Mock, patch

from tools.fetch_tools import download_checked, extract_tar_checked, fetch_matching_tools
from tools.gcc_match import helper_definitions, normalize_delay_slots
from tools.progress import BASE_MATCHES_PATH, SCOPE_PATH, SNAPSHOT_PATH, compiled_symbol_bytes, extract_startup, load_json, matching_source_declarations, stage_report, unit_symbols, validate_report
from tools.verify_original import HEADER_SIZE, ROOT


class ProgressTests(unittest.TestCase):
    def test_delay_slot_normalization_uses_compiler_reorder_modes(self):
        source = ".set reorder\nbne $2,$0,label\nsb $0,0($5)\n.set noreorder\njal func_8001A670\nmove $4,$16\n"
        converted = ".set noreorder\nbne $2,$0,label\nnop # DEBUG: branch/jump\nsb $0,0($5)\njal func_8001A670\nnop # DEBUG: branch/jump\nmove $4,$16\n"
        expected = converted.replace("jal func_8001A670\nnop # DEBUG: branch/jump\n", "jal func_8001A670\n")
        self.assertEqual(normalize_delay_slots(converted, source), expected)
        with self.assertRaisesRegex(ValueError, "branches differ"):
            normalize_delay_slots(".set noreorder\n", source)

    def test_matching_tools_reject_unknown_compiler(self):
        with self.assertRaisesRegex(ValueError, "Unsupported matching compiler"):
            fetch_matching_tools("unregistered-compiler")

    def test_delay_slot_normalization_preserves_required_return_nop(self):
        required = ".set noreorder\nj $31\nnop # DEBUG: branch/jump\n.end helper\n"
        self.assertEqual(normalize_delay_slots(required), required)
        duplicate = ".set noreorder\njal func_8001A670\nnop # DEBUG: branch/jump\nmove $4,$16\n"
        self.assertEqual(normalize_delay_slots(duplicate), duplicate.replace("nop # DEBUG: branch/jump\n", ""))
        reordered = ".set noreorder\n.set reorder\nj $31\nnop # DEBUG: branch/jump\n.end helper\n"
        self.assertEqual(normalize_delay_slots(reordered), reordered.replace("nop # DEBUG: branch/jump\n", ""))

    def test_matching_source_requires_compiled_byte_equality(self):
        symbol = {"name": "handler", "start": "0x80010000", "end": "0x80010004", "size": 4}
        spec = {"symbols": [symbol], "matching_sources": [{"symbols": ["handler"]}]}
        mapped = [{"symbol": "handler", "start": symbol["start"], "end": symbol["end"]}]
        reference = bytes(HEADER_SIZE) + b"CODE"
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with patch("tools.gcc_match.build_gcc_symbol", return_value=b"CODE"):
                declarations = matching_source_declarations(spec, reference, 0x80010000, mapped, root)
            self.assertEqual(len(declarations), 1)
            self.assertEqual((root / "matching/handler/handler.bin").read_bytes(), b"CODE")
            with patch("tools.gcc_match.build_gcc_symbol", return_value=b"FAIL"):
                with self.assertRaisesRegex(ValueError, "does not byte-match"):
                    matching_source_declarations(spec, reference, 0x80010000, mapped, root)
            self.assertEqual((root / "matching/handler/handler.bin").read_bytes(), b"CODE")
            spec["symbols"] = []
            with self.assertRaisesRegex(ValueError, "exact allowlist"):
                matching_source_declarations(spec, reference, 0x80010000, mapped, root)

    def test_matching_linker_requires_explicit_helper_addresses(self):
        self.assertEqual(helper_definitions(["func_8001A670", "func_8001A670"]),
                         ["--defsym=func_8001A670=0x8001A670"])
        for name in ("unknown_global", "func_8001A67X"):
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, "Unmapped"):
                helper_definitions([name])
        self.assertEqual(helper_definitions(["g_value"], {"g_value": "0x801D8A0C"}),
                         ["--defsym=g_value=0x801D8A0C"])
        with self.assertRaisesRegex(ValueError, "Invalid mapped global"):
            helper_definitions(["g_value"], {"g_value": "0x100"})

    def test_compiled_symbol_bytes_uses_symbol_bounds(self):
        elf = Mock()
        section = Mock()
        section.__getitem__ = Mock(return_value=0x80037000)
        section.data.return_value = b"prefix" + b"code" + b"suffix"
        elf.get_section.return_value = section
        symbol = {"st_info": {"type": "STT_FUNC"}, "st_shndx": 1,
                  "st_value": 0x80037006, "st_size": 4}
        elf.get_section_by_name.return_value.get_symbol_by_name.return_value = [symbol]
        self.assertEqual(compiled_symbol_bytes(elf, "handler", "STT_FUNC"), b"code")
        for field, value in (("st_value", 0x80036FFF), ("st_size", 100), ("st_shndx", "SHN_ABS")):
            with self.subTest(field=field), self.assertRaises(ValueError):
                elf.get_section_by_name.return_value.get_symbol_by_name.return_value = [{**symbol, field: value}]
                compiled_symbol_bytes(elf, "handler", "STT_FUNC")

    def setUp(self):
        self.scope = load_json(SCOPE_PATH)
        self.report = load_json(SNAPSHOT_PATH)
        self.expected_code_bytes = sum(int(symbol["end"], 16) - int(symbol["start"], 16) for unit in self.scope["units"] for symbol in unit_symbols(unit))

    def test_real_snapshot_has_verified_getter_match(self):
        self.assertEqual(validate_report(self.report, self.scope), self.expected_code_bytes)
        spec = load_json(ROOT / "config" / "base-matches.json")["units"]["l0/functions"]
        expected_symbols = spec["symbols"]
        report_unit = next(unit for unit in self.report["units"] if unit["name"] == "l0/functions")
        for expected in expected_symbols:
            function = next(function for function in report_unit["functions"] if function["name"] == expected["name"])
            with self.subTest(symbol=expected["name"]):
                self.assertEqual(int(function["size"]), expected["size"])
                self.assertEqual(function["fuzzy_match_percent"], 100)
        self.assertEqual(int(self.report["measures"]["matched_code"]), sum(symbol["size"] for symbol in expected_symbols))
        data_symbols = [symbol for source in spec["data_sources"] for symbol in source["symbols"]]
        expected_data = sum(symbol["size"] for symbol in data_symbols)
        self.assertEqual(int(self.report["measures"]["total_data"]), expected_data)
        self.assertEqual(int(self.report["measures"]["matched_data"]), expected_data)
        self.assertEqual(float(self.report["measures"]["matched_data_percent"]), 100.0)
        exact_names = {symbol["name"] for symbol in expected_symbols}
        fuzzy_names = {symbol for source in spec["fuzzy_sources"] for symbol in source["symbols"]}
        for name in fuzzy_names:
            function = next(function for function in report_unit["functions"] if function["name"] == name)
            with self.subTest(fuzzy_candidate=name):
                self.assertGreater(function["fuzzy_match_percent"], 0)
                self.assertLess(function["fuzzy_match_percent"], 100)
                self.assertNotIn(name, exact_names)
        self.assertEqual(int(self.report["measures"]["matched_functions"]), len(exact_names))
        if fuzzy_names:
            self.assertGreater(float(self.report["measures"]["fuzzy_match_percent"]), float(self.report["measures"]["matched_code_percent"]))
        else:
            self.assertAlmostEqual(float(self.report["measures"]["fuzzy_match_percent"]), float(self.report["measures"]["matched_code_percent"]), places=6)

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

    def test_rejects_wrong_or_missing_exact_data_match(self):
        for field, value in (("total_data", "195"), ("matched_data", "195")):
            report = copy.deepcopy(self.report)
            report["measures"][field] = value
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, "data measures differ"):
                validate_report(report, self.scope)

    def test_rejects_invented_progress_in_target_only_unit(self):
        report = copy.deepcopy(self.report)
        unit = next(unit for unit in report["units"] if unit["name"] == "l1/functions")
        unit["measures"]["matched_code"] = 1
        with self.assertRaisesRegex(ValueError, "without a compiled base"):
            validate_report(report, self.scope)

    def test_rejects_fuzzy_score_for_unlisted_function(self):
        report = copy.deepcopy(self.report)
        unit = next(unit for unit in report["units"] if unit["name"] == "l1/functions")
        unit["functions"][0]["fuzzy_match_percent"] = 1
        with self.assertRaisesRegex(ValueError, "without an exact or explicitly fuzzy source candidate"):
            validate_report(report, self.scope)

    def test_rejects_unverified_candidate_at_full_match(self):
        report = copy.deepcopy(self.report)
        unit = next(unit for unit in report["units"] if unit["name"] == "l0/functions")
        base_matches = load_json(BASE_MATCHES_PATH)
        spec = base_matches["units"]["l0/functions"]
        exact_names = {symbol["name"] for symbol in spec["symbols"]}
        candidate = next(function for function in unit["functions"] if function["name"] not in exact_names)
        spec["fuzzy_sources"].append({"source": "src/player_model/player_actor_flags.c", "symbols": [candidate["name"]]})
        candidate["fuzzy_match_percent"] = 100
        with patch("tools.progress.load_json", return_value=base_matches), self.assertRaisesRegex(ValueError, "verify its bytes"):
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
        with self.assertRaisesRegex(ValueError, "byte-verified exact-match allowlist"):
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

    def test_rejects_tool_archive_path_traversal(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            archive = root / "unsafe.tar.gz"
            with tarfile.open(archive, "w:gz") as package:
                member = tarfile.TarInfo("../escape")
                member.size = 4
                package.addfile(member, io.BytesIO(b"evil"))
            with self.assertRaises(tarfile.FilterError):
                extract_tar_checked(archive, root / "tools")
            self.assertFalse((root / "escape").exists())


if __name__ == "__main__":
    unittest.main()