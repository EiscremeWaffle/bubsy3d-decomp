import copy
import struct
import unittest

from tools.code_analysis import (
    contiguous_ranges, manual_trace_entries, promote_manual_switch_roots,
    render_code_map, trace_reachable, validate_discovery,
)
from tools.progress import ROOT, load_json
from tools.verify_original import HEADER_SIZE


class FixtureInstruction:
    def __init__(self, kind="normal", target=None):
        self.kind = kind
        self.target = target

    def isValid(self):
        return self.kind != "invalid"

    def hasDelaySlot(self):
        return self.kind in ("call", "return", "loop", "indirect_call")

    def isReturn(self):
        return self.kind == "return"

    def isJumpWithAddress(self):
        return self.kind == "call"

    def getInstrIndexAsVram(self):
        return self.target

    def doesLink(self):
        return self.kind in ("call", "indirect_call")

    def isBranch(self):
        return self.kind == "loop"

    def getBranchVramGeneric(self):
        return self.target

    def isUnconditionalBranch(self):
        return self.kind == "loop"

    def isJump(self):
        return self.kind in ("return", "call", "indirect_call")


class CodeAnalysisTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.document = load_json(ROOT / "config" / "code-map.json")
        cls.manifest = load_json(ROOT / "config" / "executable-map.json")

    def trace_fixture(self, instructions, size=24, extra_entries=()):
        load = 0x80010000
        expected = {"filename": "fixture.EXE", "sha256": "fixture", "load_address": hex(load), "entry_point": hex(load), "payload_size": hex(size)}
        factory = lambda word, address: instructions.get(address - load, FixtureInstruction())
        return trace_reachable(bytes(HEADER_SIZE + size), expected, extra_entries=extra_entries, instruction_factory=factory)

    def test_call_and_return_delay_slots_are_counted_once(self):
        result = self.trace_fixture({0: FixtureInstruction("call", 0x80010010), 8: FixtureInstruction("return"), 16: FixtureInstruction("return")})
        self.assertEqual(result["reachable_code_bytes"], 24)
        self.assertEqual(len(result["function_entry_candidates"]), 2)
        self.assertEqual(result["unresolved"], [])

    def test_unconditional_loop_stops_without_falling_into_data(self):
        result = self.trace_fixture({0: FixtureInstruction("loop", 0x80010000)})
        self.assertEqual(result["reachable_code_bytes"], 8)
        self.assertEqual(result["unresolved"], [])

    def test_invalid_delay_slot_is_flagged_and_not_decoded_as_code(self):
        result = self.trace_fixture({0: FixtureInstruction("call", 0x80010010), 4: FixtureInstruction("invalid")})
        self.assertEqual(result["reachable_code_bytes"], 4)
        self.assertEqual(result["unresolved"][0]["reason"], "invalid_delay_slot")

    def test_indirect_call_remains_unresolved_but_return_path_is_traced(self):
        result = self.trace_fixture({0: FixtureInstruction("indirect_call"), 8: FixtureInstruction("return")}, size=16)
        self.assertEqual(result["reachable_code_bytes"], 16)
        self.assertEqual(result["unresolved"][0]["reason"], "indirect_call")

    def test_range_coalescing_does_not_fill_unvisited_gaps(self):
        self.assertEqual(contiguous_ranges({0x1000, 0x1004, 0x1010}), [[0x1000, 0x1008], [0x1010, 0x1014]])

    def test_manual_trace_entries_do_not_create_function_candidates(self):
        load = 0x80010000
        config = {"trace_entries": {"fixture.EXE": [{"address": hex(load + 8)}]}}
        extra_entries = manual_trace_entries(config, "fixture.EXE")
        result = self.trace_fixture({0: FixtureInstruction("return"), 8: FixtureInstruction("return")}, size=16, extra_entries=extra_entries)
        self.assertEqual(extra_entries, {load + 8})
        self.assertEqual(result["reachable_code_bytes"], 16)
        self.assertEqual(result["function_entry_candidates"], [hex(load)])

    def test_manual_function_splits_enclosing_reachable_fragment(self):
        load = 0x80010000
        data = bytearray(HEADER_SIZE + 40)
        struct.pack_into("<I", data, HEADER_SIZE + 8, 0x03E00008)
        struct.pack_into("<I", data, HEADER_SIZE + 24, 0x03E00008)
        expected = {"load_address": hex(load), "payload_size": hex(40)}
        discovery = {
            "ranges": [{"start": f"0x{load:08X}", "end": f"0x{load + 40:08X}"}],
            "indirect_references": [],
            "manual_roots": [
                {"address": f"0x{load + 8:08X}", "end": f"0x{load + 16:08X}",
                 "symbol": "first", "evidence": "test"},
                {"address": f"0x{load + 24:08X}", "end": f"0x{load + 32:08X}",
                 "symbol": "second", "evidence": "test"},
            ],
            "symbols": [{"start": f"0x{load:08X}", "end": f"0x{load + 40:08X}",
                         "size": 40, "kind": "reachable_fragment"}],
            "anchored_functions": 0,
            "reachable_fragments": 1,
        }

        result = promote_manual_switch_roots(data, expected, discovery)
        self.assertEqual(result["anchored_functions"], 2)
        self.assertEqual(result["reachable_fragments"], 3)
        self.assertEqual(
            [(int(symbol["start"], 16) - load, int(symbol["end"], 16) - load, symbol["kind"])
             for symbol in result["symbols"]],
            [(0, 8, "reachable_fragment"), (8, 16, "anchored_function"),
             (16, 24, "reachable_fragment"), (24, 32, "anchored_function"),
             (32, 40, "reachable_fragment")],
        )
        self.assertEqual(sum(symbol["size"] for symbol in result["symbols"]), 40)

    def test_manual_function_rejects_overlapping_bounded_symbol(self):
        load = 0x80010000
        data = bytearray(HEADER_SIZE + 16)
        struct.pack_into("<I", data, HEADER_SIZE + 4, 0x03E00008)
        expected = {"load_address": hex(load), "payload_size": hex(16)}
        discovery = {
            "ranges": [{"start": f"0x{load:08X}", "end": f"0x{load + 16:08X}"}],
            "indirect_references": [],
            "manual_roots": [{"address": f"0x{load + 4:08X}", "end": f"0x{load + 12:08X}",
                              "symbol": "manual", "evidence": "test"}],
            "symbols": [{"start": f"0x{load:08X}", "end": f"0x{load + 16:08X}",
                         "size": 16, "kind": "anchored_function"}],
            "anchored_functions": 1,
            "reachable_fragments": 0,
        }
        with self.assertRaisesRegex(ValueError, "overlaps another bounded symbol"):
            promote_manual_switch_roots(data, expected, discovery)

    def test_real_discovery_partitions_traced_code_for_every_module(self):
        total = validate_discovery(self.document, self.manifest)
        self.assertEqual(total, self.document["code_bytes"])
        scope = load_json(ROOT / "config" / "code-units.json")
        for module in self.document["modules"]:
            mapped = {(symbol["start"], symbol["end"], symbol["kind"]) for symbol in module["symbols"]}
            reported = {(symbol["start"], symbol["end"], symbol["kind"]) for unit in scope["units"] if unit["filename"] == module["path"] for symbol in unit["symbols"]}
            self.assertEqual(mapped, reported)
            load, end = int(module["load_start"], 16), int(module["load_end"], 16)
            unknown = sum(int(span["end"], 16) - int(span["start"], 16) for span in module["unclassified_ranges"])
            self.assertEqual(module["reachable_code_bytes"] + unknown, end - load)

    def test_readable_code_map_matches_current_discovery(self):
        self.assertEqual((ROOT / "docs" / "code-map.md").read_text(encoding="utf-8"), render_code_map(self.document))

    def test_manual_player_roots_are_present_and_bounded(self):
        roots = load_json(ROOT / "config" / "manual-code-roots.json")["modules"]["L0/L0.EXE"]
        module = next(item for item in self.document["modules"] if item["path"] == "L0/L0.EXE")
        for root in roots:
            with self.subTest(symbol=root["symbol"]):
                symbol = next(item for item in module["symbols"] if item.get("symbol") == root["symbol"])
                self.assertEqual(symbol["start"], root["address"])
                self.assertEqual(symbol["kind"], "anchored_function")
                self.assertTrue(root["evidence"])

    def test_grounded_computed_switch_is_bounded_by_its_table(self):
        roots = load_json(ROOT / "config" / "manual-code-roots.json")["modules"]["L0/L0.EXE"]
        root = next(item for item in roots if item["address"] == "0x8004B35C")
        module = next(item for item in self.document["modules"] if item["path"] == "L0/L0.EXE")
        symbol = next(item for item in module["symbols"] if item.get("symbol") == root["symbol"])

        self.assertEqual(symbol["start"], root["address"])
        self.assertEqual(symbol["end"], root["end"])
        self.assertEqual(symbol["kind"], "anchored_function")
        self.assertEqual(root["computed_jump_tables"], [{
            "site": "0x8004B4D0", "table": "0x800139A8", "count": 33,
        }])

    def test_heading_callback_switch_is_bounded_by_its_table(self):
        roots = load_json(ROOT / "config" / "manual-code-roots.json")["modules"]["L0/L0.EXE"]
        root = next(item for item in roots if item["address"] == "0x80036018")
        module = next(item for item in self.document["modules"] if item["path"] == "L0/L0.EXE")
        symbol = next(item for item in module["symbols"] if item.get("symbol") == root["symbol"])
        self.assertEqual((symbol["start"], symbol["end"], symbol["kind"]),
                         (root["address"], root["end"], "anchored_function"))
        self.assertEqual(root["computed_jump_tables"], [{
            "site": "0x80036404", "table": "0x80012B00", "count": 49,
        }])

    def one_module(self):
        module = copy.deepcopy(self.document["modules"][0])
        manifest = {"executables": [entry for entry in self.manifest["executables"] if entry["filename"] == module["path"]]}
        return {"modules": [module]}, manifest

    def test_rejects_overlapping_symbols(self):
        document, manifest = self.one_module()
        document["modules"][0]["symbols"].append(copy.deepcopy(document["modules"][0]["symbols"][0]))
        with self.assertRaisesRegex(ValueError, "overlap"):
            validate_discovery(document, manifest)

    def test_rejects_stale_original_fingerprint(self):
        document, manifest = self.one_module()
        document["modules"][0]["sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "fingerprint"):
            validate_discovery(document, manifest)

    def test_rejects_invalid_control_flow_in_publishable_map(self):
        document, manifest = self.one_module()
        document["modules"][0]["unresolved"].append({"address": "0x80010000", "reason": "invalid_instruction_on_reachable_path"})
        with self.assertRaisesRegex(ValueError, "control flow"):
            validate_discovery(document, manifest)


if __name__ == "__main__":
    unittest.main()