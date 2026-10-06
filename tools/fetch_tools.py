import hashlib
from pathlib import Path
import platform
import urllib.request
import zipfile

from tools.verify_original import ROOT


TOOL_DIR = ROOT / "tools" / "bin"
ZIG_URL = "https://ziglang.org/download/0.14.1/zig-x86_64-windows-0.14.1.zip"
ZIG_HASH = "554f5378228923ffd558eac35e21af020c73789d87afeabf4bfd16f2e6feed2c"
OBJDIFF_URL = "https://github.com/encounter/objdiff/releases/download/v3.8.2/objdiff-cli-windows-x86_64.exe"
OBJDIFF_HASH = "36d229ac6ce74a26b47f42cf86ea8e808d8cd834a54aa62d7ed3f47b765d971b"


def download_checked(url, expected_hash, destination):
    if destination.exists():
        data = destination.read_bytes()
    else:
        print(f"Downloading {url}", flush=True)
        request = urllib.request.Request(url, headers={"User-Agent": "bubsy3d-decomp-setup"})
        with urllib.request.urlopen(request, timeout=120) as response:
            data = response.read()
    if hashlib.sha256(data).hexdigest() != expected_hash:
        raise ValueError(f"SHA256 mismatch for {destination.name}; refusing to use download")
    destination.parent.mkdir(parents=True, exist_ok=True)
    if not destination.exists():
        destination.write_bytes(data)
    return destination


def main():
    if platform.system() != "Windows" or platform.machine().lower() not in ("amd64", "x86_64"):
        raise SystemExit("Automatic tool download currently supports Windows x86_64 only")
    archive = download_checked(ZIG_URL, ZIG_HASH, TOOL_DIR / "zig-0.14.1.zip")
    with zipfile.ZipFile(archive) as package:
        root = TOOL_DIR.resolve()
        for member in package.infolist():
            path = (root / member.filename).resolve()
            if not path.is_relative_to(root):
                raise ValueError("Unsafe path in Zig archive")
        package.extractall(TOOL_DIR)
    download_checked(OBJDIFF_URL, OBJDIFF_HASH, TOOL_DIR / "objdiff-cli.exe")
    print("Ready: Zig 0.14.1 and objdiff-cli 3.8.2 (SHA256 verified)")


if __name__ == "__main__":
    main()