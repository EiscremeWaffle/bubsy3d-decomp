import argparse
import json
import re
import struct
from pathlib import Path

import rabbitizer

from tools.verify_original import HEADER_SIZE, ROOT, verify_executable


TERMS = (
    b"BUBSWIM", b"BUB.TZP", b"Bubsy", b"BUBSY_ID", b"gBubsyInfo",
    b"deathType", b"bubsy.c", b"PLIS", b"gobj->moveInfo",
    b"gMoveRequestCount", b"MAX_MOVE_REQUEST_COUNT",
)
LOWER_OPCODES = {9, 13, 32, 33, 35, 36, 37, 40, 41, 43, 48, 49, 53, 56, 57, 61}


def string_targets(data, load_address):
    targets = []
    for match in re.finditer(rb"[^\x00]{4,}\x00", data):
        text = match.group()[:-1]
        if any(term.lower() in text.lower() for term in TERMS):
            address = load_address + match.start() - HEADER_SIZE
            targets.append({"address": address, "text": text.decode("ascii", "replace")})
    return targets


def find_references(data, load_address, targets):
    words = struct.unpack_from(f"<{(len(data) - HEADER_SIZE) // 4}I", data, HEADER_SIZE)
    target_addresses = {target["address"] for target in targets}
    references = {address: [] for address in target_addresses}
    for index, high_word in enumerate(words):
        if high_word >> 26 != 15:
            continue
        register = (high_word >> 16) & 31
        high = (high_word & 0xFFFF) << 16
        for low_index in range(index + 1, min(index + 5, len(words))):
            low_word = words[low_index]
            opcode = low_word >> 26
            if (low_word >> 21) & 31 != register or opcode not in LOWER_OPCODES:
                continue
            immediate = low_word & 0xFFFF
            if opcode != 13 and immediate & 0x8000:
                immediate -= 0x10000
            target = (high + immediate) & 0xFFFFFFFF
            if target in references:
                references[target].append({
                    "high_address": load_address + index * 4,
                    "low_address": load_address + low_index * 4,
                })
    return words, references


def containing_symbols(code_map, module_path, address):
    module = next((item for item in code_map["modules"] if item["path"] == module_path), None)
    if module is None:
        return []
    return [
        {"start": symbol["start"], "end": symbol["end"], "kind": symbol["kind"]}
        for symbol in module["symbols"]
        if int(symbol["start"], 16) <= address < int(symbol["end"], 16)
    ]


def main():
    parser = argparse.ArgumentParser(description="Find code references to Bubsy and character-resource identifiers")
    parser.add_argument("--module", default="L0/L0.EXE")
    parser.add_argument("--term", action="append", help="Restrict output to matching strings or filenames; may be repeated")
    parser.add_argument("--address", type=lambda value: int(value, 0), help="Disassemble an executable virtual address instead of string references")
    parser.add_argument("--callers", type=lambda value: int(value, 0), help="Find direct MIPS J/JAL call sites for a virtual address")
    parser.add_argument("--global-address", type=lambda value: int(value, 0), help="Find direct MIPS HI/LO references to a global address")
    parser.add_argument("--bytes", type=int, default=0x180, help="Disassembly size when --address is used")
    parser.add_argument("--context", type=int, default=4)
    args = parser.parse_args()
    if not 0 <= args.context <= 20:
        parser.error("--context must be between 0 and 20")
    manifest = json.loads((ROOT / "config" / "executable-map.json").read_text(encoding="utf-8"))
    expected = next((item for item in manifest["executables"] if item["filename"] == args.module), None)
    if expected is None:
        parser.error("No executable matches --module")
    path = ROOT / "original" / "usa" / args.module
    verify_executable(path, expected)
    data = path.read_bytes()
    load = int(expected["load_address"], 16)
    if args.callers is not None:
        end = load + int(expected["payload_size"], 16)
        if args.callers % 4 or not load <= args.callers < end:
            parser.error("--callers target must be aligned inside the loaded executable")
        words = struct.unpack_from(f"<{(len(data) - HEADER_SIZE) // 4}I", data, HEADER_SIZE)
        sites = []
        for index, word in enumerate(words):
            if word >> 26 not in (2, 3):
                continue
            address = load + index * 4
            target = ((address + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
            if target == args.callers:
                sites.append(address)
        for address in sites:
            print(f"{address:08X}: {rabbitizer.Instruction(words[(address - load) // 4], address, rabbitizer.InstrCategory.R3000GTE).disassemble()}")
        print(f"{len(sites)} direct caller(s)")
        return
    if args.global_address is not None:
        words = struct.unpack_from(f"<{(len(data) - HEADER_SIZE) // 4}I", data, HEADER_SIZE)
        count = 0
        for index, high_word in enumerate(words):
            if high_word >> 26 != 15:
                continue
            register = (high_word >> 16) & 31
            high = (high_word & 0xFFFF) << 16
            for low_index in range(index + 1, min(index + 5, len(words))):
                low_word = words[low_index]
                opcode = low_word >> 26
                if (low_word >> 21) & 31 != register or opcode not in LOWER_OPCODES:
                    continue
                immediate = low_word & 0xFFFF
                if opcode != 13 and immediate & 0x8000:
                    immediate -= 0x10000
                if ((high + immediate) & 0xFFFFFFFF) != args.global_address:
                    continue
                address = load + index * 4
                count += 1
                print(f"Reference {address:08X}..{load + low_index * 4:08X}")
                for current in range(max(0, index - 2), min(len(words), low_index + 3)):
                    current_address = load + current * 4
                    instruction = rabbitizer.Instruction(words[current], current_address, rabbitizer.InstrCategory.R3000GTE)
                    print(f"  {current_address:08X}: {words[current]:08X} {instruction.disassemble()}")
        print(f"{count} reference(s)")
        return
    if args.address is not None:
        end = load + int(expected["payload_size"], 16)
        if args.bytes <= 0 or args.bytes > 0x1000 or args.address % 4 or not load <= args.address < end or args.address + args.bytes > end:
            parser.error("--address and --bytes must select an aligned range inside the loaded executable")
        for address in range(args.address, args.address + args.bytes, 4):
            offset = HEADER_SIZE + address - load
            word = struct.unpack_from("<I", data, offset)[0]
            instruction = rabbitizer.Instruction(word, address, rabbitizer.InstrCategory.R3000GTE)
            print(f"{address:08X}: {word:08X} {instruction.disassemble()}")
        return
    targets = string_targets(data, load)
    if args.term:
        targets = [target for target in targets if any(term.lower() in target["text"].lower() for term in args.term)]
    words, references = find_references(data, load, targets)
    code_map = json.loads((ROOT / "config" / "code-map.json").read_text(encoding="utf-8"))
    for target in targets:
        print(f"{target['address']:08X} {target['text']}")
        for reference in references[target["address"]]:
            high = reference["high_address"]
            low = reference["low_address"]
            print(f"  ref {high:08X}..{low:08X}; mapped={containing_symbols(code_map, args.module, high)}")
            first = max(0, (high - load) // 4 - args.context)
            last = min(len(words), (low - load) // 4 + args.context + 2)
            for index in range(first, last):
                address = load + index * 4
                instruction = rabbitizer.Instruction(words[index], address, rabbitizer.InstrCategory.R3000GTE)
                print(f"    {address:08X}: {words[index]:08X} {instruction.disassemble()}")


if __name__ == "__main__":
    main()