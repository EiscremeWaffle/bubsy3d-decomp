import argparse
import hashlib
import json
import os
from pathlib import Path
import struct

from tools.verify_original import ROOT, inspect_executable, verify_executable


MAGIC = b"PS-X EXE"
FORMAT_HINTS = {
    ".HOG": "HOG-named container; internal format unresolved",
    ".TZP": "TZP-named resource; compression and internal format unresolved",
    ".TIM": "TIM-named image resource; header not recognized",
    ".VH": "VH-named sound-bank header; header not recognized",
    ".VB": "VB-named sound-bank sample data; inferred from extension",
    ".SEP": "SEP-named sequence data; inferred from extension",
    ".STR": "STR-named screen or stream resource; encoding unresolved",
    ".XA": "XA-named audio resource; encoding unresolved",
    ".DUM": "DUM-named menu resource; internal format unresolved",
    ".CCS": "CCS-named resource; internal format unresolved",
    ".DAT": "DAT-named data; internal format unresolved",
    ".BAK": "BAK-named auxiliary file; contents unresolved",
}


def inspect_file(path, relative_path):
    before = path.stat()
    digest = hashlib.sha256()
    sha1 = hashlib.sha1()
    size = 0
    prefix = b""
    tail = b""
    signatures = set()
    with path.open("rb") as source:
        while chunk := source.read(1024 * 1024):
            if not prefix:
                prefix = chunk[:0x800]
            digest.update(chunk)
            sha1.update(chunk)
            search = tail + chunk
            position = search.find(MAGIC)
            while position >= 0:
                signatures.add(size - len(tail) + position)
                position = search.find(MAGIC, position + 1)
            size += len(chunk)
            tail = search[-(len(MAGIC) - 1):]
    after = path.stat()
    if before.st_size != size or before.st_mtime_ns != after.st_mtime_ns:
        raise ValueError(f"File changed during inventory: {relative_path}")
    record = {
        "path": relative_path,
        "size": size,
        "sha256": digest.hexdigest(),
        "sha1": sha1.hexdigest(),
        "format": "unresolved",
        "format_evidence": FORMAT_HINTS.get(path.suffix.upper(), "No recognized header or documented format"),
        "psx_magic_offsets": sorted(signatures),
    }
    if prefix.startswith(MAGIC):
        header = inspect_executable(path.read_bytes())
        record.update(format="psx_executable", format_evidence="Validated PS-X EXE header", executable=header)
    elif prefix[:4] == b"\x10\x00\x00\x00" and path.suffix.upper() == ".TIM":
        record.update(format="tim_candidate", format_evidence="PS1 TIM magic; image blocks not validated")
    elif prefix[:4] == b"pBAV":
        record.update(format="vab_header_candidate", format_evidence="PS1 VAB header magic; sound bank not decoded")
    elif relative_path.upper() == "SYSTEM.CNF":
        record.update(format="boot_configuration", format_evidence="PlayStation boot configuration filename")
    elif prefix.startswith(b"CD-ROM Generator for Windows"):
        record.update(format="disc_authoring_metadata", format_evidence="CD-ROM Generator application identifier")
    return record


def scan_disc(disc_dir):
    disc_dir = disc_dir.resolve()
    if not disc_dir.is_dir() or disc_dir == ROOT or disc_dir.is_relative_to(ROOT):
        raise ValueError("Provide the extracted disc directory, not the source repository")
    records = []
    directories = []

    def fail_walk(error):
        raise error

    for current, children, filenames in os.walk(disc_dir, topdown=True, onerror=fail_walk, followlinks=False):
        directory = Path(current)
        included = []
        for child in sorted(children):
            path = directory / child
            if path.resolve() == ROOT:
                continue
            if path.is_symlink() or path.is_junction():
                raise ValueError(f"Refusing linked disc directory: {path.name}")
            included.append(child)
        children[:] = included
        directories.append(directory.relative_to(disc_dir).as_posix())
        for filename in sorted(filenames):
            path = directory / filename
            if path.is_symlink():
                raise ValueError(f"Refusing linked disc file: {filename}")
            records.append(inspect_file(path, path.relative_to(disc_dir).as_posix()))
    records.sort(key=lambda record: record["path"])
    return {
        "schema_version": 1,
        "scope": "Extracted disc file inventory, not a whole-game code or function map",
        "excluded": ["Local source repository and its generated files"],
        "file_count": len(records),
        "total_file_bytes": sum(record["size"] for record in records),
        "directories": sorted(directories),
        "files": records,
    }


def matches_startup(reference, candidate):
    if len(reference) != 172 or len(candidate) != 172:
        return False
    for original, actual in zip(struct.unpack("<43I", reference), struct.unpack("<43I", candidate)):
        opcode = original >> 26
        mask = 0xFFFF0000 if opcode in (9, 15, 35, 43) else 0xFC000000 if opcode == 3 else 0xFFFFFFFF
        if original & mask != actual & mask:
            return False
    return True


def executable_maps(inventory, disc_dir, boot_manifest):
    boot = next(entry for entry in boot_manifest["executables"] if entry["filename"] == "SLUS_001.10")
    boot_offset = 0x800 + int(boot["entry_point"], 16) - int(boot["load_address"], 16)
    boot_data = (disc_dir / boot["filename"]).read_bytes()
    reference = boot_data[boot_offset:boot_offset + 172]
    executables = []
    modules = []
    units = []
    for record in inventory["files"]:
        if record["format"] != "psx_executable":
            continue
        header = record["executable"]
        expected = {
            "filename": record["path"], "size": record["size"],
            "sha1": record["sha1"], "sha256": record["sha256"],
            "entry_point": header["entry_point"], "load_address": header["load_address"],
            "payload_size": header["payload_size"],
        }
        path = disc_dir / record["path"]
        verify_executable(path, expected)
        data = path.read_bytes()
        load = int(header["load_address"], 16)
        entry = int(header["entry_point"], 16)
        end = load + int(header["payload_size"], 16)
        if entry + 172 > end:
            raise ValueError(f"Startup range extends outside the loaded payload for {record['path']}")
        offset = 0x800 + entry - load
        code = data[offset:offset + 172]
        if not matches_startup(reference, code) or code[-4:] != struct.pack("<I", 0x4D):
            raise ValueError(f"Startup layout is not confirmed for {record['path']}")
        module_name = "boot" if record["path"] == "SLUS_001.10" else "menu" if record["path"] == "MENU.EXE" else Path(record["path"]).parent.as_posix().lower()
        units.append({
            "name": f"{module_name}/startup", "filename": record["path"],
            "symbol": f"{module_name}_entrypoint", "start": f"0x{entry:08X}",
            "end": f"0x{entry + 172:08X}",
        })
        spans = []
        for start, stop, status in ((load, entry, "unmapped_code_or_data"), (entry, entry + 172, "verified_startup_code"), (entry + 172, end, "unmapped_code_or_data")):
            if stop > start:
                spans.append({
                    "start": f"0x{start:08X}", "end": f"0x{stop:08X}",
                    "size": stop - start, "status": status,
                    "file_start": f"0x{0x800 + start - load:X}",
                    "file_end": f"0x{0x800 + stop - load:X}",
                })
        modules.append({
            "path": record["path"], "load_start": f"0x{load:08X}", "load_end": f"0x{end:08X}",
            "entry_point": f"0x{entry:08X}", "payload_bytes": end - load,
            "known_code_bytes": 172, "unmapped_payload_bytes": end - load - 172,
            "spans": spans,
        })
        executables.append(expected)
    manifest = {key: boot_manifest[key] for key in ("name", "serial", "revision_status")}
    manifest["executables"] = executables
    scope = {
        "scope": "Startup routines of every header-recognized USA executable; not whole-game code progress",
        "artifact": "SLUS_001.10_startup_report", "units": units,
    }
    coverage = {
        "scope": "All header-recognized executable payloads, with unresolved code/data explicitly marked",
        "not_covered": "Compressed or raw code without a PS-X EXE signature may exist in opaque resources",
        "module_count": len(modules),
        "payload_bytes": sum(module["payload_bytes"] for module in modules),
        "known_code_bytes": sum(module["known_code_bytes"] for module in modules),
        "modules": modules,
    }
    return manifest, scope, coverage


def render_map(inventory, coverage):
    lines = [
        "# USA Disc Directory and Executable Map", "",
        "Generated from the local extracted disc by `python -m tools.map_disc --write-module-maps --write-documentation`.", "",
        f"- Files: {inventory['file_count']}",
        f"- Directories including root: {len(inventory['directories'])}",
        f"- Extracted file bytes: {inventory['total_file_bytes']:,}",
        f"- Header-recognized executable modules: {coverage['module_count']}",
        f"- Combined executable payload bytes: {coverage['payload_bytes']:,}",
        f"- Verified startup code bytes: {coverage['known_code_bytes']:,}",
        f"- Remaining executable payload bytes, code/data unresolved: {coverage['payload_bytes'] - coverage['known_code_bytes']:,}", "",
        "This is a complete inventory of this extraction, not proof of an unmodified original-release disc",
        "or a complete function/code map. No file contents or game instruction bytes are published here.", "",
        "[Full file fingerprints and format evidence](../config/disc-map.json)",
        "[Executable fingerprints](../config/executable-map.json)",
        "[Address coverage with file offsets](../config/executable-coverage.json)", "",
        "## Interpretation", "",
        "- Level directories, COMMON, MENU, MOVIES, and XA group files by observed location; their roles are filename-based hints.",
        "- The extraction contains L0 through L20 except L3. Do not invent L3 or conclude that files are missing without checking the original disc.",
        "- HOG, TZP, DUM, STR, and other opaque resources are inventoried but their internal records/compression are not mapped.",
        "- TIM and VAB magic identifies candidates; their complete image/audio structures are not validated.",
        "- Only the listed executables contain the PS-X EXE signature anywhere in the scanned files. Raw or compressed code without it may still exist.",
        "- Every executable loads at the same base address. These are separate module images, not one simultaneously resident RAM layout.",
        "- Repeated startup functions are counted per module, not as unique source functions. Everything remains 0% decompiled.", "",
        "## Executable Modules", "",
        "End addresses are exclusive. Payload bytes include both code and data and must not be used as the code-progress denominator.", "",
        "| Disc path | Loaded start | Loaded end | Entry point | Payload bytes | Verified startup bytes |",
        "| --- | --- | --- | --- | ---: | ---: |",
    ]
    for module in coverage["modules"]:
        lines.append(f"| `{module['path']}` | {module['load_start']} | {module['load_end']} | {module['entry_point']} | {module['payload_bytes']} | {module['known_code_bytes']} |")
    lines.extend(["", "## Directory Summary", "", "Counts are direct children, not recursive subtree totals.", "", "| Directory | Direct files | File bytes |", "| --- | ---: | ---: |"])
    grouped = {directory: [] for directory in inventory["directories"]}
    for record in inventory["files"]:
        grouped[Path(record["path"]).parent.as_posix()].append(record)
    for directory, records in grouped.items():
        lines.append(f"| `{directory}` | {len(records)} | {sum(record['size'] for record in records)} |")
    lines.extend(["", "## Every File", "", "SHA256 prefixes are for scanning; full SHA256 and SHA1 values are in the JSON inventory."])
    for directory, records in grouped.items():
        lines.extend(["", f"### {directory}", "", "| File | Bytes | Format/evidence status | SHA256 prefix |", "| --- | ---: | --- | --- |"])
        for record in records:
            filename = Path(record["path"]).name.replace("|", "\\|")
            lines.append(f"| `{filename}` | {record['size']} | {record['format']} | `{record['sha256'][:12]}` |")
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description="Inventory every extracted disc file without publishing game data")
    parser.add_argument("--disc-dir", type=Path, default=ROOT.parent)
    parser.add_argument("--output", type=Path, default=ROOT / "config" / "disc-map.json")
    parser.add_argument("--write-module-maps", action="store_true", help="Update executable fingerprints, startup scope, and payload coverage maps")
    parser.add_argument("--write-documentation", action="store_true", help="Generate the human-readable directory and module map")
    args = parser.parse_args()
    if args.write_documentation and not args.write_module_maps:
        parser.error("--write-documentation requires --write-module-maps")
    manifest = json.loads((ROOT / "config" / "usa.json").read_text(encoding="utf-8"))
    for expected in manifest["executables"]:
        verify_executable(args.disc_dir / expected["filename"], expected)
    inventory = scan_disc(args.disc_dir)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(inventory, indent=2) + "\n", encoding="utf-8")
    if args.write_module_maps:
        executable_manifest, scope, coverage = executable_maps(inventory, args.disc_dir, manifest)
        for filename, document in (("executable-map.json", executable_manifest), ("startup-units.json", scope), ("executable-coverage.json", coverage)):
            (ROOT / "config" / filename).write_text(json.dumps(document, indent=2) + "\n", encoding="utf-8")
        if args.write_documentation:
            (ROOT / "docs" / "disc-map.md").write_text(render_map(inventory, coverage), encoding="utf-8")
    executable_count = sum(record["format"] == "psx_executable" for record in inventory["files"])
    print(f"Mapped {inventory['file_count']} files in {len(inventory['directories'])} directories")
    print(f"Total extracted file bytes: {inventory['total_file_bytes']}")
    print(f"Recognized PS-X executables: {executable_count}; opaque resources remain unclassified internally")


if __name__ == "__main__":
    main()