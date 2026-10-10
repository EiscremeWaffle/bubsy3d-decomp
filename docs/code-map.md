# Executable Code Discovery Map

This is the current decomp.dev report scope, not a certified whole-game function map.

- Executable modules: 22
- Mapped instruction bytes: 3,002,136
- Bounded function candidates: 10,409
- Explicitly labeled reachable fragments: 1,167
- Unclassified executable payload bytes: 12,816,616
- Indirect sites requiring further review: 2,390
- Exact source matches configured: 18404 bytes across 93 function(s) (0.613030%)

- Exact executable C data matches configured: 196 bytes across 1 data symbol(s)
- Other disc files: 621,466,361 bytes across 424 files (inventory only; not objdiff data coverage)
- Unclassified executable payload: 12,816,616 bytes (code or data; not counted as data)
No section map is present in the PS-X EXE headers; total executable data size is not yet known.
Objdiff reports fuzzy similarity separately in `config/code-report.json`; partial scores do not add exact matched bytes or functions.

Counts include repeated routines in separate executable images; they are not unique source-function counts.
Fragment symbols are code-block placeholders, not declarations of complete functions.

[All ranges, symbols, original fingerprints, pointer evidence, and unresolved sites](../config/code-map.json)
[Report object groups and symbols](../config/code-units.json)
[Player-specific first pass and evidence](player-bubsy.md)

## Discovery Evidence

The pass starts at validated PS-X EXE entry points and follows valid MIPS-I/GTE control flow,
including delay slots, direct calls, branches, and returns. It iterates statically referenced
callback slots and candidate jump-table entries in the original image, and constructed code
addresses corroborated by a stack-frame/return-address-save prologue.

spimdisasm bounds function candidates only within traced contiguous ranges. Supported exits
and closed branch ranges are required. Other reached instructions remain explicitly named fragments.
The original bytes of every emitted symbol are checked in relocatable MIPS-I ELF32 objects.

This remains a static-analysis baseline: pointer-table bounds and inferred function identities
require review. Dynamic callbacks, function pointers assigned at runtime, switch tables not
resolved statically, dead/unreferenced code, and opaque resource formats can hide further code.
BIOS/SDK dispatch wrappers also appear among the indirect sites; not every unresolved site
necessarily represents additional game code. Unclassified bytes are not declared code or data.

The research target objects pack selected original ranges. They do not preserve the full executable
layout or reconstruct original relocations/translation units. One getter is verified through the
pinned Clang MIPS-II scheduling profile; this does not establish the original game's compiler.

## Module Coverage

| Module | Code bytes | Bounded candidates | Fragments | Unclassified payload | Indirect sites |
| --- | ---: | ---: | ---: | ---: | ---: |
| `L0/L0.EXE` | 256360 | 835 | 64 | 458392 | 126 |
| `L1/L1.EXE` | 130260 | 462 | 51 | 629548 | 104 |
| `L10/L10.EXE` | 130260 | 462 | 51 | 621356 | 104 |
| `L11/L11.EXE` | 130260 | 462 | 51 | 625452 | 104 |
| `L12/L12.EXE` | 130260 | 462 | 51 | 629548 | 104 |
| `L13/L13.EXE` | 129824 | 462 | 51 | 634080 | 104 |
| `L14/L14.EXE` | 130260 | 462 | 51 | 629548 | 104 |
| `L15/L15.EXE` | 130260 | 462 | 51 | 625452 | 104 |
| `L16/L16.EXE` | 129816 | 462 | 51 | 613608 | 104 |
| `L17/L17.EXE` | 130260 | 462 | 51 | 547628 | 104 |
| `L18/L18.EXE` | 130260 | 462 | 51 | 561964 | 104 |
| `L19/L19.EXE` | 130260 | 462 | 51 | 590636 | 104 |
| `L2/L2.EXE` | 130260 | 462 | 51 | 619308 | 104 |
| `L20/L20.EXE` | 130272 | 462 | 51 | 617248 | 104 |
| `L4/L4.EXE` | 130224 | 462 | 51 | 611152 | 104 |
| `L5/L5.EXE` | 130260 | 462 | 51 | 619308 | 104 |
| `L6/L6.EXE` | 130224 | 462 | 51 | 600912 | 104 |
| `L7/L7.EXE` | 130260 | 462 | 51 | 637740 | 104 |
| `L8/L8.EXE` | 130224 | 462 | 51 | 598864 | 104 |
| `L9/L9.EXE` | 130260 | 462 | 51 | 629548 | 104 |
| `MENU.EXE` | 134772 | 396 | 67 | 358796 | 144 |
| `SLUS_001.10` | 137040 | 400 | 67 | 356528 | 144 |

## Reproduce and Publish

```powershell
.\.venv\Scripts\python.exe -m tools.code_analysis --functions --write-map
.\.venv\Scripts\python.exe -m tools.progress build
python -m tools.progress stage
python -m unittest discover -s tests -v
```

Commit and push the metadata and project code, never original binaries or generated assembly/objects.
After the push workflow succeeds, select default version `SLUS_001.10_code` in decomp.dev management.
The report artifact is `SLUS_001.10_code_report`; its inner file is `report.json`.
See [integration instructions](decomp-dev.md) for publication and site setup.
