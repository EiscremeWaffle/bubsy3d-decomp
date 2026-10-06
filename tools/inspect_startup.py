import argparse
import json
import struct

import rabbitizer

from verify_original import HEADER_SIZE, ROOT, verify_executable


def main():
    parser = argparse.ArgumentParser(description="Inspect verified USA executable entry points")
    parser.add_argument("--count", type=int, default=64)
    args = parser.parse_args()
    if not 1 <= args.count <= 512:
        parser.error("--count must be between 1 and 512")
    manifest = json.loads((ROOT / "config" / "usa.json").read_text(encoding="utf-8"))
    for expected in manifest["executables"]:
        path = ROOT / "original" / "usa" / expected["filename"]
        verify_executable(path, expected)
        data = path.read_bytes()
        entry = int(expected["entry_point"], 16)
        offset = HEADER_SIZE + entry - int(expected["load_address"], 16)
        print(f"{path.name}: entry=0x{entry:08X}, file offset=0x{offset:X}")
        for index in range(args.count):
            address = entry + index * 4
            word = struct.unpack_from("<I", data, offset + index * 4)[0]
            instruction = rabbitizer.Instruction(word, address)
            print(f"{address:08X}: {word:08X}  {instruction.disassemble()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())