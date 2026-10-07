import argparse
import io
import json
from pathlib import Path
import shutil
import struct
import subprocess

from tools.verify_original import HEADER_SIZE, ROOT, verify_executable


SCOPE_PATH = ROOT / "config" / "code-units.json"
SNAPSHOT_PATH = ROOT / "config" / "code-report.json"
BASE_MATCHES_PATH = ROOT / "config" / "base-matches.json"
ZIG = ROOT / "tools" / "bin" / "zig-x86_64-windows-0.14.1" / "zig.exe"
OBJDIFF = ROOT / "tools" / "bin" / "objdiff-cli.exe"


def load_json(path):
    return json.loads(path.read_text(encoding="utf-8"))


def extract_startup(data, expected, unit):
    start = int(unit["start"], 16)
    end = int(unit["end"], 16)
    load = int(expected["load_address"], 16)
    payload_end = load + int(expected["payload_size"], 16)
    if start != int(expected["entry_point"], 16):
        raise ValueError("Startup range must begin at the verified entry point")
    if not load <= start < end <= payload_end or start % 4 or end % 4:
        raise ValueError("Startup range must be aligned and inside the loaded payload")
    code = data[HEADER_SIZE + start - load:HEADER_SIZE + end - load]
    if len(code) != end - start or code[-4:] != struct.pack("<I", 0x0000004D):
        raise ValueError("Startup range must end immediately after its observed break instruction")
    return code


def unit_symbols(unit):
    return unit["symbols"] if "symbols" in unit else [unit]


def validate_report(report, scope):
    symbols = {unit["name"]: {symbol["symbol"]: int(symbol["end"], 16) - int(symbol["start"], 16) for symbol in unit_symbols(unit)} for unit in scope["units"]}
    expected = {name: sum(sizes.values()) for name, sizes in symbols.items()}
    units = report.get("units", [])
    if len(units) != len(expected) or {unit["name"] for unit in units} != set(expected):
        raise ValueError("Report units do not match the documented code-map scope")
    total_size = sum(expected.values())
    base_matches = load_json(BASE_MATCHES_PATH)["units"]
    expected_matched_code = sum(symbol["size"] for unit in base_matches.values() for symbol in unit["symbols"])
    expected_matched_functions = sum(len(unit["symbols"]) for unit in base_matches.values())
    fuzzy_symbols = {
        name: {symbol for source in spec.get("fuzzy_sources", []) for symbol in source["symbols"]}
        for name, spec in base_matches.items()
    }
    for unit in units:
        measures = unit["measures"]
        if int(measures.get("total_code", 0)) != expected[unit["name"]]:
            raise ValueError("Report unit code size does not match the verified range")
        functions = unit.get("functions", [])
        actual_symbols = {function["name"]: int(function["size"]) for function in functions}
        if actual_symbols != symbols[unit["name"]] or len(functions) != len(actual_symbols):
            raise ValueError("Report symbols do not match the mapped functions and fragments")
        if int(measures.get("total_functions", 0)) != len(actual_symbols) or int(measures.get("total_units", 0)) != 1:
            raise ValueError("Report unit function/symbol and unit counts are inconsistent")
        matched_functions = {function["name"]: function for function in functions}
        expected_base = base_matches.get(unit["name"])
        exact_symbols = {symbol["name"] for symbol in expected_base["symbols"]} if expected_base else set()
        allowed_fuzzy_symbols = fuzzy_symbols.get(unit["name"], set())
        if exact_symbols & allowed_fuzzy_symbols:
            raise ValueError("A symbol cannot be both an exact match and a fuzzy candidate")
        if not allowed_fuzzy_symbols.issubset(actual_symbols):
            raise ValueError("A configured fuzzy candidate is missing from the target code map")
        for symbol in (expected_base["symbols"] if expected_base else []):
            function = matched_functions.get(symbol["name"])
            if function is None or int(function["size"]) != symbol["size"] or float(function.get("fuzzy_match_percent", 0)) != 100:
                raise ValueError(f"Expected byte-exact source match missing for {symbol['name']}")
        unit_measures = unit["measures"]
        if int(unit_measures.get("matched_code", 0)) > int(unit_measures.get("total_code", 0)) or int(unit_measures.get("matched_functions", 0)) > int(unit_measures.get("total_functions", 0)):
            raise ValueError("Matched unit measures exceed their target denominator")
        exact_bytes = sum(symbol["size"] for symbol in (expected_base["symbols"] if expected_base else []))
        exact_function_count = len(expected_base["symbols"]) if expected_base else 0
        if expected_base:
            if int(unit_measures.get("matched_code", 0)) != exact_bytes or int(unit_measures.get("matched_functions", 0)) != exact_function_count:
                raise ValueError("Matched unit measures differ from the byte-verified exact-match allowlist")
        elif any(float(unit_measures.get(field, 0)) != 0 for field in ("matched_code", "matched_functions", "fuzzy_match_percent")):
            raise ValueError("A unit without a compiled base object cannot claim source progress")
        if any(float(unit_measures.get(field, 0)) != 0 for field in ("complete_code", "complete_units")):
            raise ValueError("Target units must not be marked as fully decompiled")
    measures = report["measures"]
    if int(measures.get("total_code", 0)) != total_size or int(measures.get("total_functions", 0)) != sum(len(group) for group in symbols.values()) or int(measures.get("total_units", 0)) != len(expected):
        raise ValueError("Report totals do not match the code-map scope")
    total_measures = report["measures"]
    if int(total_measures.get("matched_code", 0)) != expected_matched_code or int(total_measures.get("matched_functions", 0)) != expected_matched_functions:
        raise ValueError("Aggregate matched measures differ from the byte-verified exact-match allowlist")
    if int(total_measures.get("matched_code", 0)) > total_size or int(total_measures.get("matched_functions", 0)) > int(total_measures.get("total_functions", 0)):
        raise ValueError("Aggregate matches exceed the target denominator")
    if any(float(total_measures.get(field, 0)) != 0 for field in ("complete_code", "complete_units", "complete_data", "total_data", "matched_data")):
        raise ValueError("Aggregate report claims unsupported complete code or data matches")
    if any(not 0 <= float(measures.get("fuzzy_match_percent", 0)) <= 100 for measures in [total_measures, *(unit["measures"] for unit in units)]):
        raise ValueError("Fuzzy-match percentages must be within 0..100")
    for unit in units:
        if unit.get("metadata", {}).get("complete", False):
            raise ValueError("Target-only units must not be marked complete")
        base_spec = base_matches.get(unit["name"])
        exact_symbols = {symbol["name"] for symbol in base_spec["symbols"]} if base_spec else set()
        allowed_fuzzy_symbols = fuzzy_symbols.get(unit["name"], set())
        for function in unit.get("functions", []):
            fuzzy = float(function.get("fuzzy_match_percent", 0))
            name = function["name"]
            if name in exact_symbols and fuzzy != 100:
                raise ValueError("A configured byte-exact base function must remain a full match")
            if name in allowed_fuzzy_symbols and fuzzy == 100:
                raise ValueError(f"Fuzzy candidate {name} is a full match; verify its bytes and promote it to the exact allowlist")
            if fuzzy and name not in exact_symbols and name not in allowed_fuzzy_symbols:
                raise ValueError("A function without an exact or explicitly fuzzy source candidate cannot claim a source match")
    if report.get("version") != 2:
        raise ValueError("Expected objdiff 3.8.2 report format version 2")
    return total_size


def build_report():
    from elftools.elf.elffile import ELFFile

    scope = load_json(SCOPE_PATH)
    manifest = load_json(ROOT / "config" / "executable-map.json")
    executables = {entry["filename"]: entry for entry in manifest["executables"]}
    units = []
    for unit in scope["units"]:
        expected = executables[unit["filename"]]
        original = ROOT / "original" / "usa" / unit["filename"]
        verify_executable(original, expected)
        data = original.read_bytes()
        load = int(expected["load_address"], 16)
        end = load + int(expected["payload_size"], 16)
        symbols = unit_symbols(unit)
        pieces = []
        directives = []
        for symbol in symbols:
            start, stop = int(symbol["start"], 16), int(symbol["end"], 16)
            if start % 4 or stop % 4 or not load <= start < stop <= end:
                raise ValueError("Code symbol is not aligned inside the loaded executable")
            offset = HEADER_SIZE + start - load
            pieces.append(data[offset:offset + stop - start])
            directives.extend([
                f'.global {symbol["symbol"]}', f'.type {symbol["symbol"]},@function',
                f'{symbol["symbol"]}:', f'.incbin "{original.as_posix()}", {offset}, {stop - start}',
                f'.size {symbol["symbol"]}, . - {symbol["symbol"]}',
            ])
        code = b"".join(pieces)
        directory = ROOT / "build" / "code" / unit["name"]
        directory.mkdir(parents=True, exist_ok=True)
        assembly = directory / "target.s"
        assembly.write_text(
            '.section .text,"ax",@progbits\n'
            '.balign 4\n'
            + "\n".join(directives) + "\n",
            encoding="utf-8",
        )
        target = directory / "target.o"
        subprocess.run(
            [str(ZIG), "cc", "-target", "mipsel-linux-musl", "-march=mips1", "-mabi=32", "-c", str(assembly), "-o", str(target)],
            cwd=ROOT, check=True,
        )
        elf = ELFFile(io.BytesIO(target.read_bytes()))
        if elf.header["e_machine"] != "EM_MIPS" or elf.header["e_type"] != "ET_REL" or not elf.little_endian or elf.elfclass != 32 or elf.header["e_flags"] & 0xF0000000:
            raise ValueError("Expected a little-endian MIPS-I ELF32 relocatable target")
        if elf.get_section_by_name(".text").data() != code:
            raise ValueError(f"Target object changed the original bytes for {unit['name']}")
        for symbol in unit_symbols(unit):
            matches = elf.get_section_by_name(".symtab").get_symbol_by_name(symbol["symbol"])
            if not matches or matches[0]["st_size"] != int(symbol["end"], 16) - int(symbol["start"], 16) or matches[0]["st_info"]["type"] != "STT_FUNC":
                raise ValueError("Target code symbol has an incorrect size or type")
        units.append({
            "name": unit["name"],
            "target_path": target.relative_to(ROOT).as_posix(),
            "metadata": {"complete": False},
        })
        if unit["name"] in load_json(BASE_MATCHES_PATH)["units"]:
            base_spec = load_json(BASE_MATCHES_PATH)["units"][unit["name"]]
            base_object = ROOT / "build" / "base" / unit["name"] / "player_model.o"
            base_object.parent.mkdir(parents=True, exist_ok=True)
            fuzzy_sources = base_spec.get("fuzzy_sources", [])
            source_to_compile = ROOT / base_spec["source"]
            if fuzzy_sources:
                source_to_compile = base_object.parent / "fuzzy_sources.c"
                includes = [base_spec["source"], *(source["source"] for source in fuzzy_sources)]
                source_to_compile.write_text(
                    "\n".join(f'#include "{source}"' for source in includes) + "\n",
                    encoding="utf-8",
                )
            subprocess.run(
                [str(ZIG), "cc", *base_spec["flags"], "-I.", "-Isrc/player_model", "-c", str(source_to_compile), "-o", str(base_object)],
                cwd=ROOT, check=True,
            )
            base_elf = ELFFile(io.BytesIO(base_object.read_bytes()))
            if base_elf.header["e_machine"] != "EM_MIPS" or base_elf.header["e_type"] != "ET_REL" or not base_elf.little_endian or base_elf.elfclass != 32:
                raise ValueError("Expected a little-endian MIPS ELF32 player base object")
            original_bytes = b"".join(pieces)
            for matched in base_spec["symbols"]:
                matched_symbol = base_elf.get_section_by_name(f".text.{matched['name']}")
                expected_symbol = next((symbol for symbol in symbols if symbol["symbol"] == matched["name"]), None)
                if expected_symbol is None:
                    raise ValueError(f"Matched base symbol {matched['name']} is not in the target unit")
                expected_bytes = data[HEADER_SIZE + int(expected_symbol["start"], 16) - load:HEADER_SIZE + int(expected_symbol["end"], 16) - load]
                if matched_symbol is None or matched_symbol.data() != expected_bytes or len(expected_bytes) != matched["size"]:
                    raise ValueError(f"Player base source does not byte-match original symbol {matched['name']}")
            units[-1]["base_path"] = base_object.relative_to(ROOT).as_posix()
            units[-1]["metadata"]["source_path"] = base_spec["source"]
        print(f"Verified {unit['name']}: {len(code)} original code bytes", flush=True)
    configuration = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/v3.8.2/config.schema.json",
        "min_version": "3.8.2", "build_target": False, "build_base": False, "units": units,
    }
    (ROOT / "objdiff.json").write_text(json.dumps(configuration, indent=2) + "\n", encoding="utf-8")
    output = ROOT / "build" / "code" / "report.json"
    subprocess.run([str(OBJDIFF), "report", "generate", "-o", str(output)], cwd=ROOT, check=True)
    report = load_json(output)
    total_size = validate_report(report, scope)
    SNAPSHOT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    measures = report["measures"]
    print(
        f"Saved code-map report: {total_size} mapped bytes; "
        f"{measures.get('matched_code', 0)} exact matched bytes; "
        f"{measures.get('fuzzy_match_percent', 0):.6f}% fuzzy similarity. "
        "Whole-game completeness not established."
    )


def stage_report(output_path=ROOT / "build" / "progress" / "report.json"):
    if output_path.name != "report.json":
        raise ValueError("decomp.dev publication requires the staged filename report.json")
    validate_report(load_json(SNAPSHOT_PATH), load_json(SCOPE_PATH))
    output_path.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(SNAPSHOT_PATH, output_path)
    return output_path


def main():
    parser = argparse.ArgumentParser(description="Build or validate the explicitly scoped discovered-code progress baseline")
    parser.add_argument("command", choices=("build", "validate", "stage"))
    args = parser.parse_args()
    if args.command == "build":
        build_report()
    elif args.command == "stage":
        print(f"Staged validated report: {stage_report().relative_to(ROOT)}")
    else:
        total_size = validate_report(load_json(SNAPSHOT_PATH), load_json(SCOPE_PATH))
        report = load_json(SNAPSHOT_PATH)
        measures = report["measures"]
        print(
            f"Valid code report: {total_size} mapped bytes, "
            f"{int(measures.get('matched_code', 0))} matched bytes, "
            f"{int(measures.get('matched_functions', 0))} exact matched functions, "
            f"{float(measures.get('fuzzy_match_percent', 0)):.6f}% fuzzy similarity; "
            "whole-game completeness not established"
        )


if __name__ == "__main__":
    main()