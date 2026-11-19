import os
from pathlib import Path
import shutil
import subprocess
import tempfile

from tools.verify_original import ROOT


def run_native_probe(test, sources, cases, input_layout, output_layout, *, compiler_flags=(), expected_error=None):
    configured_cc = os.environ.get("CC")
    compiler = shutil.which(configured_cc or "cc")
    command = [compiler] if compiler else []
    if not command and os.name == "nt":
        zig = ROOT / "tools" / "bin" / "zig-x86_64-windows-0.14.1" / "zig.exe"
        if zig.is_file():
            command = [str(zig), "cc"]
    test.assertTrue(command, "a native C compiler or the project's Zig installation is required")

    with tempfile.TemporaryDirectory() as temporary:
        executable = Path(temporary) / ("heading_probe.exe" if os.name == "nt" else "heading_probe")
        compile_result = subprocess.run(
            command + [
                "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
                "-Isrc/player_model", "-Isrc/game", *compiler_flags, *sources, "-o", str(executable),
            ],
            cwd=ROOT, capture_output=True, text=True, check=False,
        )
        test.assertEqual(compile_result.returncode, 0, compile_result.stderr)
        probe = subprocess.run(
            [str(executable), "--binary"],
            input=b"".join(input_layout.pack(*entry) for entry in cases),
            capture_output=True, check=False,
        )
        if expected_error is not None:
            test.assertNotEqual(probe.returncode, 0)
            test.assertIn(expected_error, probe.stderr)
            test.assertEqual(probe.stdout, b"")
            return []
        test.assertEqual(probe.returncode, 0, probe.stderr.decode(errors="replace"))
        test.assertEqual(len(probe.stdout), output_layout.size * len(cases))
        return list(output_layout.iter_unpack(probe.stdout))
