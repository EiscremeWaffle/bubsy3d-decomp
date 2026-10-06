import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys


ROOT = Path(__file__).resolve().parents[1]
HEADER_SIZE = 0x800


def inspect_executable(data):
    if len(data) < HEADER_SIZE or data[:8] != b"PS-X EXE":
        raise ValueError("Not a PS-X EXE with a complete 0x800-byte header")
    entry_point, global_pointer, load_address, payload_size = struct.unpack_from(
        "<4I", data, 0x10
    )
    if payload_size > len(data) - HEADER_SIZE:
        raise ValueError("PS-X EXE payload extends beyond the file")
    if not load_address <= entry_point < load_address + payload_size:
        raise ValueError("Entry point is outside the loaded payload")
    return {
        "entry_point": f"0x{entry_point:08X}",
        "global_pointer": f"0x{global_pointer:08X}",
        "load_address": f"0x{load_address:08X}",
        "payload_size": f"0x{payload_size:08X}",
    }


def verify_executable(path, expected):
    data = path.read_bytes()
    actual = {
        "size": len(data),
        "sha1": hashlib.sha1(data).hexdigest(),
        "sha256": hashlib.sha256(data).hexdigest(),
        **inspect_executable(data),
    }
    for field in ("size", "sha1", "sha256", "entry_point", "load_address", "payload_size"):
        if actual[field] != expected[field]:
            raise ValueError(
                f"{path.name}: {field} mismatch: expected {expected[field]}, got {actual[field]}"
            )
    return actual


def main():
    parser = argparse.ArgumentParser(description="Verify the recorded USA executable fingerprints")
    parser.add_argument(
        "--disc-dir", type=Path, default=ROOT / "original" / "usa",
        help="Directory containing executables extracted from your own disc",
    )
    args = parser.parse_args()
    manifest = json.loads((ROOT / "config" / "executable-map.json").read_text(encoding="utf-8"))
    try:
        for expected in manifest["executables"]:
            actual = verify_executable(args.disc_dir / expected["filename"], expected)
            print(f"OK {expected['filename']}: {actual['sha256']}")
            print(f"  entry={actual['entry_point']} load={actual['load_address']} size={actual['payload_size']}")
    except (OSError, ValueError) as error:
        print(f"Verification failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())