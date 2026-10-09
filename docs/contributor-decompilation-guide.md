# Contributor Decompilation Guide

This guide records the working procedure for reconstructing code in this repository. The target is the USA Bubsy 3D executable set, but this is not a complete game rebuild and the original SDK/toolchain is not yet identified.

## What Counts as Progress

Keep three claims separate:

- **Behavior reconstructed:** C models observed behavior, usually with a host-testable API.
- **Byte matched:** the configured source profile compiles, links with observed helper/global addresses, and its function bytes exactly equal the fingerprint-verified executable range.
- **Fuzzy similarity:** objdiff's diagnostic similarity. A fuzzy score is not exact credit and must never be manually converted into matched bytes.

Only the exact allowlist in `config/base-matches.json` contributes exact code credit. `tools/progress.py` rejects missing symbols, wrong sizes, wrong bytes, unexpected data totals, and full fuzzy scores that have not been promoted. Do not edit report measures by hand.

The current validated scope maps 2,988,972 code bytes. Exact progress requires 14,945 bytes to reach 0.5%; the current report validates 15,176 exact code bytes across 92 functions (0.507733%) and 196 exact data bytes. New exact coverage includes 20 assembly-backed copies of `func_80010128`, one assembly-backed L0 Q20.12 dot product, 22 C copies of `func_80072F4C`, and 19 additional copies each of two existing actor-flag/cursor helpers. `bubsy_process_actor_event` remains a fuzzy candidate, not an exact match.

## Setup

Use an executable set extracted from your own disc. Setup verifies hashes before copying files into ignored `original/usa/` storage. See `readme2.md` for the full setup and disc-layout commands.

On Windows x86_64, install Python requirements and verified research tools:

```powershell
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
.\.venv\Scripts\python.exe -m tools.fetch_tools
.\.venv\Scripts\python.exe -m tools.verify_original
```

The matching pipeline currently uses Zig/Clang for the baseline object and checksum-pinned GCC 2.6.3 PSX, GCC 2.7.2 PSX, MASPSX, and LLD for registered function profiles. GCC executables are static 32-bit Linux programs: on Windows they run through a default x86_64 Linux WSL distribution. `ld.lld` must support MIPS ELF32. GCC is run on a temporary Linux filesystem because old GCC can fail to stat source files on the Windows-mounted drive. No per-user files under `/home/<user>` are required.

The 2.6.3 archive SHA256 is `01e6e8c4933414ea3f8d8e3bc766a1f5fafd4fc0110e0b75d1f691bd791989b1`; the 2.7.2 archive and MASPSX revision are pinned in `tools/fetch_tools.py`. Pins establish reproducibility, not proof that either compiler built the original game.

## Function Workflow

1. **Start from the map.** Find the function's executable, address, and exclusive end in `config/code-units.json`; check callers and neighboring functions. Treat inferred names as provisional. Do not assume a nearby code range is a complete function unless its exits and boundaries are supported.
2. **Verify the original.** Run `python -m tools.verify_original`. Decode the function from the verified payload, accounting for the PS-X EXE header and load address. Branches execute their delay slots; include the `jr` delay slot in the function size.
3. **Trace before naming.** Record argument registers, saved registers, stack size/offsets, globals, field widths, call arguments, branch destinations, and delay-slot effects. Calculate branch destinations from instruction addresses instead of inferring them from a decompiler label. Use `rabbitizer`/`spimdisasm` or a focused Python snippet to print bounded instruction ranges.
4. **Build a small semantic C model first.** Add explicit structs and `_Static_assert` offset checks where layouts are supported. Keep host-testable logic separate from target runtime ABI when necessary. Add tests for signed boundaries, branch predicates, stores, and return values. Avoid assigning gameplay meanings to numeric IDs without evidence.
5. **Try the matching compiler.** Use an isolated compiler profile and compare the linked symbol bytes to the verified executable. Compiler version, flags, compatibility headers, source hash, and binary hashes are recorded by `tools/gcc_match.py`. Test alternate compiler versions when repeated register-allocation or scheduling differences suggest the compiler may be wrong; keep results per symbol until a whole translation unit is reproduced.
6. **Prefer C constraints over instruction strings.** Fixed-register variables, typed volatile accesses, empty compiler barriers, and ordinary C expressions can influence allocation/order without embedding machine instructions. A `__asm__` block containing MIPS mnemonics is still assembly-backed, even if the rest of the function is C. Be precise in docs and profiles about which is used.
7. **Use a byte gate.** `build_gcc_symbol` compiles, assembles, links helper symbols to observed addresses, and checks the symbol bounds. Exact credit requires same start address, size, and bytes. If it differs, inspect the first byte/instruction mismatch and make one local hypothesis-driven change.
8. **Register only exact results.** Add an entry to `matching_sources` and `symbols` only after the source-built bytes match the original. Leave partial implementations in `fuzzy_sources`. Then run the complete progress build, validator, tests, and stage command.

Typical report commands:

```powershell
.\.venv\Scripts\python.exe -m tools.progress build
.\.venv\Scripts\python.exe -m tools.progress validate
.\.venv\Scripts\python.exe -m tools.progress stage
.\.venv\Scripts\python.exe -S -m unittest discover -s tests -v
```

The native C harness is separate from Python tests. CI's compile command is in `.github/workflows/checks.yml`; compile with `-std=c11 -Wall -Wextra -Werror`, then run the resulting binary.

## Toolchain and Delay-Slot Lessons

- GCC 2.7.2 rejects the newer `+m` constraint; `=m` works when reserving stack storage.
- Its preprocessor is sensitive to backslash-newline macros in CRLF files. Keep compatibility macros such as `_Static_assert` on one line.
- MASPSX reads stdin when launched non-interactively. Use `--force-stdin` and pass the compiler assembly as subprocess input; otherwise it can wait indefinitely.
- Do not delete every MASPSX `# DEBUG: branch/jump` nop. For compiler-generated code, `tools/gcc_match.normalize_delay_slots` tracks the original `.set reorder`/`.set noreorder` mode and retains required delay slots. It rejects branch-expansion mismatches.
- For hand-authored inline assembly, put `.set noreorder` around explicitly ordered branches/calls. Restore `.set reorder` before a compiler-generated return if its final nop is required.
- Relocatable object bytes alone are not enough for calls and local jumps. The matching tool links at the symbol's real address, resolves only explicit `func_XXXXXXXX` helpers and configured globals, then compares bytes.
- A prior report omitted exact data because unused helper readonly tables were pulled into the Clang bundle. Keep completed GCC-only sources out of fuzzy Clang includes; preserve independent data-symbol byte checks instead of patching report totals.

## Current Work Queue

The seven actor sequence helpers, model-resource selector, and assembly-backed death handler match under checksum-pinned GCC 2.6.3/MASPSX; this establishes a useful matching profile, not the original game's compiler. GCC 2.7.2 differed on the remainder helper's multiply-high register selection. The event dispatcher remains fuzzy: both tested GCC versions currently emit 324 bytes against its 316-byte mapped range. `shared_actor_state_dispatch` is the active related reconstruction. Its host-testable models cover the opening key/mask clear, mode-1 numeric update, and mode-0 clamp. The target-only C draft now models the observed `0x10000` mode-2/3 callback gate, callback-plan offsets, and temporary `progress_34` behavior. GCC 2.6.3 emits 516 bytes against the 516-byte mapped function, but several branch destinations and one store instruction still differ. Do not add either dispatcher to exact progress until linked bytes match the full mapped range.

When aiming for a progress threshold, calculate required **exact bytes** from the current report denominator and prioritize functions with clear boundaries and tractable control flow. A large function is useful only if it can be finished and byte verified; its fuzzy percentage does not advance exact progress.

## Safe Repository Hygiene

Original executables, disc assets, downloaded toolchains, generated disassembly, objects, linked ELFs, and proof JSON belong under ignored `original/` or `build/` locations. Do not force-add them. Before committing, inspect `git status`, check the full diff, run `git diff --check`, rebuild and validate the report, and stage only source/config/docs/tests plus `config/code-report.json`. Push only after the report workflow's validation passes.
