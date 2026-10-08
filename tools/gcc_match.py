import hashlib
import io
import json
import os
from pathlib import Path
import re
import subprocess
import sys

from tools.fetch_tools import GCC_HASH, MASPSX_HASH, fetch_matching_tools
from tools.verify_original import ROOT


def run_tool(command, input_text=""):
    result = subprocess.run(command, cwd=ROOT, input=input_text, capture_output=True, text=True, timeout=120)
    if result.returncode:
        raise RuntimeError(f"Tool failed: {command[0]}\n{result.stdout}\n{result.stderr}")
    return result.stdout


def linux_command(command):
    if os.name == "nt":
        wsl = Path(os.environ.get("SystemRoot", "C:/Windows")) / "System32" / "wsl.exe"
        return [str(wsl), "--exec", *command]
    return command


def linux_path(path):
    path = path.resolve()
    if os.name == "nt":
        return run_tool(linux_command(["wslpath", "-a", "-u", str(path)])).strip()
    return path.as_posix()


def helper_definitions(names):
    definitions = []
    for name in sorted(set(names)):
        match = re.fullmatch(r"func_([0-9A-Fa-f]{8})", name)
        if match is None:
            raise ValueError(f"Unmapped GCC external symbol: {name}")
        definitions.append(f"--defsym={name}=0x{match.group(1)}")
    return definitions


def build_gcc_symbol(source_spec, symbol_spec, directory, zig):
    from elftools.elf.elffile import ELFFile
    from tools.progress import compiled_symbol_bytes

    if source_spec["compiler"] != "gcc-2.7.2-psx":
        raise ValueError("Unsupported matching compiler profile")
    directory.mkdir(parents=True, exist_ok=True)
    gcc_directory, maspsx_directory = fetch_matching_tools()
    name = symbol_spec["name"]
    source = ROOT / source_spec["source"]
    assembly = directory / f"{name}.gcc.s"
    workspace = run_tool(linux_command(["mktemp", "-d", "/tmp/bubsy-gcc.XXXXXXXXXX"])).strip()
    if re.fullmatch(r"/tmp/bubsy-gcc\.[A-Za-z0-9]+", workspace) is None:
        raise ValueError("Unexpected temporary GCC workspace path")
    try:
        inputs = [source, *(ROOT / header for header in source_spec["headers"])]
        if len({path.name for path in inputs}) != len(inputs):
            raise ValueError("GCC input files have conflicting basenames")
        for path in inputs:
            run_tool(linux_command(["cp", linux_path(path), f"{workspace}/{path.name}"]))
        run_tool(linux_command([
            "/usr/bin/env", f"GCC_EXEC_PREFIX={linux_path(gcc_directory)}/",
            linux_path(gcc_directory / "gcc"), *source_spec["flags"],
            f"-I{workspace}", f"{workspace}/{source.name}", "-o", f"{workspace}/output.s",
        ]))
        run_tool(linux_command(["cp", f"{workspace}/output.s", linux_path(assembly)]))
    finally:
        run_tool(linux_command(["rm", "-r", "--", workspace]))
    converted = run_tool([
        sys.executable, str(maspsx_directory / "maspsx.py"),
        "--aspsx-version", "2.30", "--force-stdin",
    ], assembly.read_text(encoding="ascii"))
    converted_lines = [
        line for line in converted.splitlines()
        if not (line.strip().startswith("nop") and "# DEBUG: branch/jump" in line)
    ]
    converted_assembly = directory / f"{name}.maspsx.s"
    converted_assembly.write_text("\n".join(converted_lines) + "\n", encoding="ascii")
    object_path = directory / f"{name}.o"
    run_tool([
        str(zig), "cc", "-target", "mipsel-linux-musl", "-march=mips1", "-mabi=32",
        "-c", str(converted_assembly), "-o", str(object_path),
    ])
    elf = ELFFile(io.BytesIO(object_path.read_bytes()))
    if elf.header["e_machine"] != "EM_MIPS" or elf.header["e_type"] != "ET_REL" or not elf.little_endian or elf.elfclass != 32:
        raise ValueError("Expected a little-endian MIPS ELF32 GCC object")
    compiled_symbol_bytes(elf, name, "STT_FUNC")
    table = elf.get_section_by_name(".symtab")
    symbol = table.get_symbol_by_name(name)[0]
    address = int(symbol_spec["start"], 16)
    text_address = address - symbol["st_value"]
    undefined = [item.name for item in table.iter_symbols() if item["st_shndx"] == "SHN_UNDEF" and item.name]
    linker_script = directory / f"{name}.ld"
    linker_script.write_text(
        f"SECTIONS {{ .text 0x{text_address:X} : SUBALIGN(4) {{ *(.text) }} }}\n",
        encoding="ascii",
    )
    linked_path = directory / f"{name}.elf"
    linker_version = run_tool(linux_command(["ld.lld", "--version"])).strip()
    run_tool(linux_command([
        "ld.lld", "-m", "elf32ltsmip", "-T", linux_path(linker_script),
        f"--entry={name}", linux_path(object_path), "-o", linux_path(linked_path),
        *helper_definitions(undefined),
    ]))
    linked = ELFFile(io.BytesIO(linked_path.read_bytes()))
    linked_symbol = linked.get_section_by_name(".symtab").get_symbol_by_name(name)[0]
    if linked_symbol["st_value"] != address:
        raise ValueError(f"GCC handler {name} linked at the wrong address")
    code = compiled_symbol_bytes(linked, name, "STT_FUNC")
    expected_size = int(symbol_spec["end"], 16) - address
    if len(code) != expected_size or len(code) != symbol_spec["size"]:
        raise ValueError(f"GCC handler {name} has the wrong byte size")
    proof = {
        "symbol": name, "address": symbol_spec["start"], "size": len(code),
        "source": source_spec["source"], "source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
        "compiler": source_spec["compiler"], "flags": source_spec["flags"],
        "gcc_archive_sha256": GCC_HASH, "maspsx_archive_sha256": MASPSX_HASH,
        "linker": linker_version, "compiled_sha256": hashlib.sha256(code).hexdigest(),
        "headers": {
            header: hashlib.sha256((ROOT / header).read_bytes()).hexdigest()
            for header in source_spec["headers"]
        },
    }
    (directory / f"{name}.proof.json").write_text(json.dumps(proof, indent=2) + "\n", encoding="utf-8")
    return code