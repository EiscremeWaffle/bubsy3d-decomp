# Contributing

Thanks for helping investigate Bubsy 3D. This repository is an evidence-based decompilation research project, not a complete game source port. The safest contributions make one behavior clearer, preserve the original evidence, and keep exact-byte progress separate from estimates.

## Start Here

1. Read the [project overview](README.md) and [current code map](docs/code-map.md) to understand the scope and remaining gaps.
2. Follow [Local Setup](docs/readme2.md#local-setup) to prepare your own extracted USA executables. The setup verifies fingerprints before using them; original game files stay local and ignored.
3. Choose a bounded function or a small unanswered question. Check its callers and neighboring symbols before assigning a gameplay name. [Player notes](docs/player-bubsy.md) collect current evidence and open questions.
4. Use the [decompilation workflow guide](docs/contributor-decompilation-guide.md) for disassembly, compiler profiles, byte comparison, and report commands.

## Working Rules

- Treat the verified executable bytes as the authority. Record addresses, bounds, argument registers, stack layout, globals, field widths, branches, and delay slots before translating behavior.
- Keep names and gameplay interpretations provisional unless the evidence supports them. A state ID or helper call alone does not establish walking, jumping, or collision semantics.
- Write host-testable C for behavior that is understood. Add focused cases for branch boundaries, signed arithmetic, outputs, and state changes. Keep target ABI details separate where necessary.
- Exact credit requires a configured source profile to compile, link at the mapped address, and match the complete original symbol byte-for-byte. A matching size or 100% fuzzy score is not sufficient.
- Put incomplete reconstructions in `fuzzy_sources`. Never edit generated report totals by hand or promote a candidate until the byte gate passes.
- Keep original executables, disc assets, downloaded toolchains, generated assembly, objects, and linked ELF files out of commits. They belong in ignored `original/` or `build/` locations.

## Verify Changes

From the repository root on Windows, run:

```powershell
.\.venv\Scripts\python.exe -S -m unittest discover -s tests -v
.\.venv\Scripts\python.exe -m tools.progress validate
```

For changes to exact matches or code-map scope, also run:

```powershell
.\.venv\Scripts\python.exe -m tools.progress build
.\.venv\Scripts\python.exe -m tools.progress stage
```

The native player-model harness is defined in `.github/workflows/checks.yml`; use its compile inputs and `-std=c11 -Wall -Wextra -Werror` flags when changing those models. On Windows, byte-matching profiles may use WSL for the pinned PSX GCC toolchain.

## Share a Contribution

Keep changes focused and explain what is observed, what remains uncertain, and how you verified it. Before committing, review `git status`, inspect the complete diff, and run `git diff --check`. Do not commit or push original game data or generated build artifacts.

## Reference Docs

- [Local setup and project status](docs/readme2.md)
- [Executable code map and discovery limits](docs/code-map.md)
- [Bubsy/player evidence and open questions](docs/player-bubsy.md)
- [Disc inventory](docs/disc-map.md)
- [Detailed byte-matching workflow](docs/contributor-decompilation-guide.md)
- [Publishing metadata to decomp.dev](docs/decomp-dev.md)
