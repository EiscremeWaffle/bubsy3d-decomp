## Status

This is a research scaffold, not a working whole-game source rebuild or a port.
The validated report currently contains 2,744 exact code bytes across 11
functions: two Clang-built getters, eight GCC C functions, and one GCC
assembly-backed function. Eight of nine previous fuzzy candidates are exact;
the actor-event dispatcher remains fuzzy while its C reconstruction is refined.
Portable host models remain covered by native regression tests. The current report maps
2,988,972 instruction bytes across all 22 recognized executables, including
explicitly labeled reachable fragments. The original SDK version and complete
section layout remain unresolved. Assembly-backed matches are not fully
recovered high-level C, and the larger Bubsy actor updater remains incomplete.
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
.\.venv\Scripts\python.exe -m tools.progress stage
```

The downloader fetches checksum-pinned Zig 0.14.1, objdiff-cli 3.8.2,
GCC 2.6.3/2.7.2 PSX from old-gcc release 0.17, and MASPSX revision
`e85ecb373828aabea96d7e6400974d4f91ce8c74` into ignored local storage.
Per-function matching profiles reproduce the verified C and assembly-backed
functions; they do not identify the complete original game toolchain.
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
The validator credits only registered source-built functions whose bytes are
verified against the fingerprinted executables. It rejects fuzzy candidates at
100% until independently verified and promoted, unsupported exact totals, and
missing data credit. The staged report must equal the validated snapshot.

### Matching Handler Prerequisites

The Windows x86_64 build requires a default x86_64 Linux WSL distribution that
can execute the static 32-bit GCC binaries, plus `ld.lld` with MIPS ELF32 support
(tested with LLD 21.1.8). Check it from PowerShell:

```powershell
wsl.exe --exec ld.lld --version
```

If LLD is missing, install it yourself inside the default Ubuntu WSL
distribution with `sudo apt-get install lld`. The build uses Windows Python for
MASPSX and Zig for MIPS assembly, and copies only declared tracked source/header
inputs into a temporary Linux directory for GCC. That avoids the old compiler's
filesystem-stat limitation on Windows-mounted files. No manual files under a
particular user's home directory or earlier probe outputs are required.

`config/base-matches.json` declares the source, compatibility header, compiler
flags, symbol address, and byte size. `tools/gcc_match.py` rebuilds and links the
handler; `tools/progress.py` checks its bytes before including compiled output
in the base object. Generated proof metadata, objects, and binaries remain under
`build/` and must not be committed. Only source, configuration, tests, docs, and
the validated report metadata are tracked.

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
