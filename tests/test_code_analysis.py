import copy
import unittest

from tools.code_analysis import contiguous_ranges, render_code_map, trace_reachable, validate_discovery
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

    def trace_fixture(self, instructions, size=24):
        load = 0x80010000
        expected = {"filename": "fixture.EXE", "sha256": "fixture", "load_address": hex(load), "entry_point": hex(load), "payload_size": hex(size)}
        factory = lambda word, address: instructions.get(address - load, FixtureInstruction())
        return trace_reachable(bytes(HEADER_SIZE + size), expected, instruction_factory=factory)

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