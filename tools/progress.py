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
    measures = report["measures"]
    if int(measures.get("total_code", 0)) != total_size or int(measures.get("total_functions", 0)) != sum(len(group) for group in symbols.values()) or int(measures.get("total_units", 0)) != len(expected):
        raise ValueError("Report totals do not match the code-map scope")
    for measures in [report["measures"], *(unit["measures"] for unit in units)]:
        for field in ("matched_code", "matched_functions", "complete_code", "complete_units", "total_data", "matched_data", "complete_data", "fuzzy_match_percent", "matched_code_percent", "matched_functions_percent", "complete_code_percent"):
            if float(measures.get(field, 0)) != 0:
                raise ValueError(f"This target-only baseline cannot claim {field}")
    for unit in units:
        if unit.get("metadata", {}).get("complete", False):
            raise ValueError("Target-only units must not be marked complete")
        for function in unit.get("functions", []):
            if float(function.get("fuzzy_match_percent", 0)) != 0:
                raise ValueError("Target-only functions must not claim a match")
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
    print(f"Saved code-map report: {total_size} mapped bytes, 0% decompiled. Whole-game completeness not established.")


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
        print(f"Valid target-only snapshot: {total_size} mapped code bytes; whole-game completeness not established")


if __name__ == "__main__":
    main()