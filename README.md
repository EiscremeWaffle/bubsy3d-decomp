# Bubsy 3D: Furbitten Planet (USA)
[![Code Progress]][progress]
[<img src="https://decomp.dev/EiscremeWaffle/bubsy3d-decomp.svg?w=512&h=256" width="512" height="256">][Progress]
=============
[Code Progress]: https://decomp.dev/EiscremeWaffle/bubsy3d-decomp.svg?mode=shield&measure=code&label=Code
[progress]: https://decomp.dev/EiscremeWaffle/bubsy3d-decomp

A early work-in-progress decompilation of Bubsy 3d Furbitten Planet.

Spefically USA rev0: `SLUS_001.10`

## Status

This is a research scaffold, not a working source rebuild or a port. No matching
compiler, linker, SDK version, or complete section layout has been established.
One C reconstruction now covers the shared Bubsy/Pliskin normal/swimming model
selection routine, with a passing native test harness. It is not yet built as a
matching base object, so decomp.dev remains 0% decompiled. The current map traces
2,981,616 instruction bytes across all 22 recognized executables, grouped into
10,370 bounded function candidates and 1,167 explicitly labeled reachable fragments.
It is still not a complete whole-game denominator.
See the [current executable code map](docs/code-map.md) for evidence and limitations.
The first player-specific source reconstruction is documented in
[docs/player-bubsy.md](docs/player-bubsy.md).

The boot and menu reference fingerprints are in [config/usa.json](config/usa.json).
All 22 executable fingerprints, including 20 level modules, are in
[config/executable-map.json](config/executable-map.json).
The USA serial is known, but the precise disc
revision still needs independent verification against a trusted disc inventory.
Do not label this a confirmed original-release revision based on the serial alone.

## Local Setup

Use Python 3.12 or newer and executables extracted from your own disc.
Run these commands from this repository directory in PowerShell:

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
.\.venv\Scripts\python.exe tools/setup.py --disc-dir ".." --analyze
.\.venv\Scripts\python.exe tools/verify_original.py
```

`".."` is appropriate for the current layout, where the extracted disc files
are in the parent folder. On another machine, supply that machine's disc directory.
On Linux, use `.venv/bin/python` in place of `.\.venv\Scripts\python.exe`.

Setup verifies all 22 executable fingerprints before copying them to `original/usa/`,
preserving their relative level-folder paths.
Existing local copies must match and are not overwritten. The original disc
extraction remains unchanged. No game data is downloaded by these tools.

With `--analyze`, splat creates provisional configs under `build/analysis/`.
Its inferred code/data boundaries and PSYQ compiler setting are hypotheses, not
evidence of a specific compiler version. Review them before disassembling or
promoting a layout into the tracked configuration.

## Checks

```powershell
python -m unittest discover -s tests -v
```

GitHub Actions runs these tests using synthetic inputs only. It does not need
the game executables. A separate executable-code workflow validates and uploads
the checked-in report metadata on pushes to `main`; it does not rebuild the game.

## Executable Code Baseline

From the repository root on Windows x86_64:

```powershell
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
.\.venv\Scripts\python.exe -m tools.fetch_tools
.\.venv\Scripts\python.exe -m tools.code_analysis --functions --write-map
.\.venv\Scripts\python.exe -m tools.progress build
.\.venv\Scripts\python.exe -m tools.progress validate
```

The downloader fetches checksum-pinned Zig 0.14.1 and objdiff-cli 3.8.2 into
ignored local storage. These are research tools, not the original game compiler.
The analyzer follows control flow and corroborated pointer references with
rabbitizer and spimdisasm. The builder verifies executable hashes and checks
selected MIPS-I ELF32 target bytes against those originals. It generates
`objdiff.json` locally and saves report metadata to `config/code-report.json`.
No game instructions are included in the tracked report.

The treemap has `functions` and `fragments` groups for each module. Fragments are
partial code blocks, not proven complete routines; counts include duplicated code
across modules. The previous 3,784-byte startup-only snapshot remains historical
metadata. Select version `SLUS_001.10_code` on decomp.dev to see the expanded map.

The snapshot must be regenerated and reviewed locally whenever its scope changes.
The current validator intentionally rejects nonzero matching claims until the
project establishes source-built objects and replaces this target-only baseline.

## Complete Disc Inventory

The [directory map](docs/disc-map.md) lists all 446 files and 40 directories in
this extraction, totaling 637,330,169 file bytes. Every file has a SHA256 and SHA1
fingerprint plus cautious format evidence in [config/disc-map.json](config/disc-map.json).

[config/executable-coverage.json](config/executable-coverage.json) accounts for
all 15,818,752 loaded executable payload bytes using the original startup baseline.
The newer [config/code-map.json](config/code-map.json) maps 2,981,616 instruction
bytes and explicitly lists the other 12,837,136 bytes as unclassified code/data.
An inventory or a load region is not a complete internal function map.

To reproduce the maps and update the report using your extracted disc:

```powershell
python -m tools.map_disc --disc-dir ".." --write-module-maps --write-documentation
.\.venv\Scripts\python.exe tools/setup.py --disc-dir ".."
.\.venv\Scripts\python.exe -m tools.code_analysis --functions --write-map
.\.venv\Scripts\python.exe -m tools.progress build
python -m tools.progress stage
python -m unittest discover -s tests -v
```

GitHub Actions publishes a separate `SLUS_001.10_disc_map` metadata artifact.
decomp.dev consumes the executable code report, not the asset-directory inventory; it
does not display an arbitrary directory tree or count asset bytes as code progress.
Opaque resource internals, compressed/raw overlays, full code/data boundaries,
function boundaries, original relocations, and unique shared code remain research tasks.

## Repository Layout

- `config/`: disc inventory, executable fingerprints, discovery maps, and progress reports.
- `tools/`: verification and local analysis setup.
- `tests/`: binary-free regression tests.
- `original/`: ignored local executables.
- `build/`: ignored generated analysis and future build outputs.
- Future `src/` and `include/`: independently written reconstruction source.

Keep disc images, original executables, extracted assets, generated disassembly,
and proprietary SDK/compiler files out of Git. Ignore rules are safeguards, not
a substitute for reviewing `git status` before committing. Do not force-add them.

See [docs/decomp-dev.md](docs/decomp-dev.md) for GitHub and progress-site setup.

## Next Research Milestones

1. Independently confirm the USA disc revision and inspect additional loaded code.
2. Establish code/data/BSS and function boundaries for all 22 executables and any overlays.
3. Identify the original compiler, options, assembler, linker, and SDK from evidence.
4. Produce a byte-identical assembly rebuild of each executable as a baseline.
5. Replace functions with matching C and generate real objdiff progress reports.

No license has been chosen for the independently written project code yet.
Choose one before accepting contributions; game and SDK files are not project code.
