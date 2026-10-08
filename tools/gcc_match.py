import hashlib
import io
import json
import os
from pathlib import Path
import re
import subprocess
import sys

from tools.fetch_tools import GCC_COMPILERS, MASPSX_HASH, fetch_matching_tools
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


def helper_definitions(names, globals=None):
    globals = globals or {}
    definitions = []
    for name in sorted(set(names)):
        match = re.fullmatch(r"func_([0-9A-Fa-f]{8})", name)
        if match is not None:
            address = int(match.group(1), 16)
        elif name in globals and re.fullmatch(r"[A-Za-z_]\w*", name):
            address = int(globals[name], 16)
            if not 0x80000000 <= address <= 0xFFFFFFFF:
                raise ValueError(f"Invalid mapped global address: {name}")
        else:
            raise ValueError(f"Unmapped GCC external symbol: {name}")
        definitions.append(f"--defsym={name}=0x{address:08X}")
    return definitions


def normalize_delay_slots(assembly, compiler_assembly=None):
    lines = assembly.splitlines()
    result = []
    noreorder = False
    branch_pattern = re.compile(r"^(?:b|beq|bne|beqz|bnez|bgtz|blez|bgez|bltz|j|jr|jal|jalr)\s")
    source_modes = []
    if compiler_assembly is not None:
        source_noreorder = False
        for source_line in compiler_assembly.splitlines():
            source_line = source_line.split("#", 1)[0].strip()
            if re.fullmatch(r"\.set\s+noreorder", source_line):
                source_noreorder = True
            elif re.fullmatch(r"\.set\s+reorder", source_line):
                source_noreorder = False
            elif branch_pattern.match(source_line):
                source_modes.append(source_noreorder)
    branch_index = 0
    explicit_slot = False
    for index, line in enumerate(lines):
        stripped = line.strip()
        if re.fullmatch(r"\.set\s+noreorder", stripped):
            noreorder = True
        elif re.fullmatch(r"\.set\s+reorder", stripped):
            noreorder = False
            if compiler_assembly is not None:
                line = ".set noreorder"
        if compiler_assembly is not None and branch_pattern.match(stripped):
            if branch_index >= len(source_modes):
                raise ValueError("Unsupported branch expansion in assembler conversion")
            explicit_slot = source_modes[branch_index]
            branch_index += 1
        if stripped.startswith("nop") and "# DEBUG: branch/jump" in line:
            following = next((item.strip() for item in lines[index + 1:] if item.strip() and not item.lstrip().startswith("#")), "")
            if compiler_assembly is not None:
                if explicit_slot and not following.startswith(".end"):
                    continue
            elif not (noreorder and following.startswith(".end")):
                continue
        result.append(line)
    if compiler_assembly is not None and branch_index != len(source_modes):
        raise ValueError("Compiler branches differ from assembler conversion")
    return "\n".join(result) + "\n"


def build_gcc_symbol(source_spec, symbol_spec, directory, zig):
    from elftools.elf.elffile import ELFFile
    from tools.progress import compiled_symbol_bytes

    if source_spec["compiler"] not in GCC_COMPILERS:
        raise ValueError("Unsupported matching compiler profile")
    directory.mkdir(parents=True, exist_ok=True)
    gcc_directory, maspsx_directory = fetch_matching_tools(source_spec["compiler"])
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
    compiler_assembly = assembly.read_text(encoding="ascii")
    converted = run_tool([
        sys.executable, str(maspsx_directory / "maspsx.py"),
        "--aspsx-version", "2.30", "--force-stdin",
    ], compiler_assembly)
    converted_assembly = directory / f"{name}.maspsx.s"
    converted_assembly.write_text(normalize_delay_slots(converted, compiler_assembly), encoding="ascii")
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
        *helper_definitions(undefined, source_spec.get("globals")),
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
        "globals": source_spec.get("globals", {}),
        "gcc_archive_sha256": GCC_COMPILERS[source_spec["compiler"]][1], "maspsx_archive_sha256": MASPSX_HASH,
        "linker": linker_version, "compiled_sha256": hashlib.sha256(code).hexdigest(),
        "headers": {
            header: hashlib.sha256((ROOT / header).read_bytes()).hexdigest()
            for header in source_spec["headers"]
        },
    }
    (directory / f"{name}.proof.json").write_text(json.dumps(proof, indent=2) + "\n", encoding="utf-8")
    return code