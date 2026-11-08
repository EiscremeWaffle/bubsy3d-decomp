## Status

This is a research scaffold, not a working whole-game source rebuild or a port.
The validated report currently contains 63,972 exact code bytes across 263
functions (`2.084216%` of mapped code). These totals include repeated module
copies and both C-based and assembly-backed matches. The actor-event
dispatcher `bubsy_process_actor_event` now matches from readable C without
instruction snippets. The shared actor-state dispatcher is
byte-matched from high-level C under a documented per-function
`-fno-cse-follow-jumps` reproduction profile, and the 2,200-byte shared
actor state-2 handler is byte-matched from high-level C using MASPSX
`--expand-div` and gp-relative small-data externs.
The readable-C conversion pass has also replaced the assembly-backed event-byte
helpers `func_8002E7D8`/`func_8002E7FC` and vector-scale wrapper `func_80053A08`
with byte-exact plain C. The complete 3,228-byte grounded handler is now
byte-matched readable C too, including an independently verified 132-byte
compiler-generated switch table. The 1,380-byte movement preparer
`func_80025A98` now matches from plain C with no inline assembly or register pins.
The 884-byte queue processor `func_8001B1E4` also matches from readable C, with
small target-only MIPS load and delay-slot sequences, and has native routing
and snapshot-exit regression coverage.
The grounded event callback and a related level-list/state-flag search are
matched C, including the plain-C list getter `func_80028CD4`.
The 1,304-byte grounded stride producer `func_80035ACC` also matches from
readable C, connecting grounded case 6 to animation-gated stride updates
and the pending-vector producer. Its mode comes from the level/model-mode
getter, not an identified controller input.
The complementary 968-byte deceleration producer `func_80038844` now matches
from plain C with natural register allocation and no inline assembly. Its
native tests cover 1,310,720 boundary combinations plus helper mutations.
Five formerly uncredited grounded-path helpers also match readable C:
`func_800556D0`, `func_8001B558`, `func_80034EF8`, `func_8003539C`, and
`func_80039158`, totaling 4,720 bytes.
The readable-C pass has additionally converted nine old assembly-backed
functions (1,852 bytes), and removed instruction snippets from seven existing
C matches without changing any matched bytes.
The shared grounded dispatcher's temporary basis rotation and stride
restoration are native-tested. Its 216-byte rotation wrapper `func_800547C8`
and 132-byte basis-restoration leaf `func_800549E8` now match plain C without
inline assembly or register constraints. Their 368-byte column-reconstruction
callee `func_800542A4` now matches plain C too. Native tests cover 131,072
reconstruction cases and 120 orientation/failure cases in the already-matched
`func_800550A4`. Engine/GTE backends and readable C for the complete
controller-to-heading path remain unrecovered. The movement preparer's 600-byte column-scaling
helper `func_8005541C` now matches plain C too, with 589,824 input/scale
combinations and 24 clamp/callback-mutation cases passing native and sanitizer
tests. Its multiply backend still contains instruction snippets.
The shared movement initializer `func_8002611C` also matches all 636 bytes
from plain C, setting up target/stride/basis/direction without producing or
enqueueing a movement step. Its 216 boundary cases, failure exits and callback
mutations pass native and sanitizer tests; this is not identified pad input.
Its 288-byte setup caller `func_80025FFC` now matches plain C as well.
Existing move-info returns success without reinitializing; 176 setup/result
combinations plus early exits, repeat calls and helper mutations pass native
and sanitizer tests. Its 408-byte move-info allocator `func_800222EC` now
matches plain C too. Whole-allocation comparisons cover 343 origin boundaries,
1,792 overlap/fill cases, untouched bytes, and null publication/failure;
native and sanitizer tests pass. The component-basis allocator
`func_80054624` also matches plain C; the allocation backend is BIOS `malloc`,
and the real setup/allocation/table-lookup integration tests cover allocation
failure and repeat setup. Three shared record lookups, two sequence wrappers,
and three runtime helpers also match plain C.
The physical pad initializer `func_8004FCBC`, reader `func_8004FCF8`, input
poller `func_8001A6E0`, stream-sample writer `func_8004FF28`, and default
event-table publisher `func_8002278C` now match readable C. Five input-event
gate and counter helpers also match readable C. Original-MIPS execution links
synthetic physical pad packets to turn events through the real callback. A
second fixture executes the original queue, grounded turn, rotation generator,
composition, and conversion through to the final synthetic orientation, with
synthetic actor/move-request state, other engine stubs, and a narrow software
model for the required GTE operations. This is not a hardware test. The callback
`func_80023E94` still lacks readable matching C; the queue processor is now a
readable-C exact match. The unstubbed controller-to-orientation path and GTE
composition internals remain unfinished.
The 272-byte event-queue snapshot `func_8001B8B8` now matches readable C in
all 20 level executables. Its forward word-copy helper `func_80010220` also
matches all 22 executable copies; it requires aligned arrays and a nonzero
count and does not provide `memmove` overlap semantics. The queue-update gate
`func_80032D7C` also matches all 120 L0 bytes from plain C.
Current exact coverage contains 232 instruction-free GCC C matches (47,264
bytes), 24 assembly-assisted C matches (6,236 bytes), five assembly-backed
transcriptions (10,440 bytes), and two Clang-built getters (32 bytes). The
assembly-assisted matches are the 20 cross-product copies, the dot product,
the scalar multiply, the queue processor, and the sequence-notification backend.
Instruction-free sources may still use symbol aliases, register constraints,
and empty compiler barriers.
The assembly-assisted and assembly-backed matches remain unfinished for
instruction-free C decompilation.
Portable host models remain covered by native regression tests. The current report maps
3,069,356 instruction bytes across all 22 recognized executables, including
explicitly labeled reachable fragments. The original SDK version and complete
section layout remain unresolved. Assembly-backed matches are not fully
recovered high-level C, and many actor-state helper meanings remain unknown.
It is still not a complete whole-game denominator.
See the [current executable code map](code-map.md) for evidence and limitations.
The first player-specific source reconstruction is documented in
[docs/player-bubsy.md](player-bubsy.md).
Contributors extending the byte-matched source work should start with the
[decompilation workflow guide](contributor-decompilation-guide.md).

The boot and menu reference fingerprints are in [config/usa.json](../config/usa.json).
All 22 executable fingerprints, including 20 level modules, are in
[config/executable-map.json](../config/executable-map.json).
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
the game executables. The optional original-MIPS controller tests in
`tests/test_original_controller_route.py` run when both the verified L0
executable and the `unicorn` Python package are available; otherwise they
are explicitly skipped. They use synthetic controller/actor state, engine
stubs, and the narrow GTE model described in
[the grounded path](player-bubsy.md#shared-lookups-and-conditional-input-to-turn-trace).
A separate executable-code workflow validates and uploads the checked-in
report metadata on pushes to `main`; it does not rebuild the game.

To run the optional controller tests locally with a verified L0 executable:

```powershell
.\.venv\Scripts\python.exe -m pip install unicorn==2.1.4
.\.venv\Scripts\python.exe -m unittest discover -s tests -p test_original_controller_route.py -v
```

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

The [directory map](disc-map.md) lists all 446 files and 40 directories in
this extraction, totaling 637,330,169 file bytes. Every file has a SHA256 and SHA1
fingerprint plus cautious format evidence in [config/disc-map.json](../config/disc-map.json).

[config/executable-coverage.json](../config/executable-coverage.json) accounts for
all 15,818,752 loaded executable payload bytes using the original startup baseline.
The newer [config/code-map.json](../config/code-map.json) maps 3,069,356 instruction
bytes and explicitly lists the other 12,749,580 bytes as unclassified code/data.
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

See [docs/decomp-dev.md](decomp-dev.md) for GitHub and progress-site setup.

## Next Research Milestones

1. Independently confirm the USA disc revision and inspect additional loaded code.
2. Establish code/data/BSS and function boundaries for all 22 executables and any overlays.
3. Identify the original compiler, options, assembler, linker, and SDK from evidence.
4. Produce a byte-identical assembly rebuild of each executable as a baseline.
5. Replace functions with matching C and generate real objdiff progress reports.

No license has been chosen for the independently written project code yet.
Choose one before accepting contributions; game and SDK files are not project code.
