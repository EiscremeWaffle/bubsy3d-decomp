import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys

from verify_original import ROOT, verify_executable


def main():
    parser = argparse.ArgumentParser(description="Import verified executables and initialize local PS1 analysis")
    parser.add_argument("--disc-dir", required=True, type=Path)
    parser.add_argument("--analyze", action="store_true", help="Generate provisional splat configs in build/analysis")
    args = parser.parse_args()
    manifest = json.loads((ROOT / "config" / "executable-map.json").read_text(encoding="utf-8"))
    destination = ROOT / "original" / "usa"
    try:
        for expected in manifest["executables"]:
            verify_executable(args.disc_dir / expected["filename"], expected)
            existing = destination / expected["filename"]
            if existing.exists():
                verify_executable(existing, expected)
        destination.mkdir(parents=True, exist_ok=True)
        for expected in manifest["executables"]:
            source = (args.disc_dir / expected["filename"]).resolve()
            target = destination / expected["filename"]
            if source != target.resolve() and not target.exists():
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(source, target)
            verify_executable(target, expected)
            print(f"Ready: {target.relative_to(ROOT)}", flush=True)
        if args.analyze:
            for expected in manifest["executables"]:
                directory = ROOT / "build" / "analysis" / expected["filename"].lower()
                directory.mkdir(parents=True, exist_ok=True)
                if any(directory.glob("*.yaml")):
                    print(f"Keeping existing analysis config: {directory.relative_to(ROOT)}")
                    continue
                subprocess.run(
                    [sys.executable, "-m", "splat", "create_config", str(destination / expected["filename"])],
                    cwd=directory, check=True,
                )
            print("Generated configs are estimates, not a verified matching-build layout.")
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Setup failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())