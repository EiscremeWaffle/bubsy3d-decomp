# Contributor Decompilation Guide

This guide records the working procedure for reconstructing code in this repository. The target is the USA Bubsy 3D executable set, but this is not a complete game rebuild and the original SDK/toolchain is not yet identified.

## What Counts as Progress

Keep three claims separate:

- **Behavior reconstructed:** C models observed behavior, usually with a host-testable API.
- **Byte matched:** the configured source profile compiles, links with observed helper/global addresses, and its function bytes exactly equal the fingerprint-verified executable range.
- **Fuzzy similarity:** objdiff's diagnostic similarity. A fuzzy score is not exact credit and must never be manually converted into matched bytes.

Only the exact allowlist in `config/base-matches.json` contributes exact code credit. `tools/progress.py` rejects missing symbols, wrong sizes, wrong bytes, unexpected data totals, and full fuzzy scores that have not been promoted. Do not edit report measures by hand.

The current validated scope maps 3,040,136 code bytes; the current report validates 42,688 exact code bytes across 153 functions (1.404148%) and 196 exact data bytes. New exact coverage includes the readable-C grounded stride producers `func_80035ACC` and `func_80038844`, three bounded move-request callers (`func_80015A54`, `func_8003CC78`, and `func_80041218`) captured as assembly-backed transcriptions, plus the readable-C movement preparer `func_80025A98`, the movement consumer, shared component-transform helpers `func_80054B20` and `func_80054C7C`, and shared `func_800347EC` actor-state handler. The complete grounded handler now matches from readable C. Coverage also includes the C vector functions `vector_length_q12` (`func_8005385C` linker label), `vector_range_scale_q12` (`func_8005395C`), and `vector_range_condition_q12` (`func_80053D68`), 20 assembly-backed copies of `func_80010128`, one assembly-backed L0 Q20.12 dot product, 22 C copies of `func_80072F4C`, and 19 additional copies each of two existing actor-flag/cursor helpers. `shared_actor_state_dispatch` is byte-matched from high-level C under a documented per-function `-fno-cse-follow-jumps` profile. `shared_actor_state_2_handler` is byte-matched from high-level C using MASPSX `--expand-div` and declared gp-relative small-data externs. `bubsy_process_actor_event` now matches all 316 bytes from readable C without instruction snippets.

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

`tools.progress build` has a 600-second overall subprocess deadline by default; set `--timeout-seconds` to adjust it, up to 2700 seconds (45 minutes). For a full rebuild on slower Windows/WSL setups, use `python -m tools.progress build --timeout-seconds 2700`. Timed-out commands have their process tree terminated.

The native C harness is separate from Python tests. CI's compile command is in `.github/workflows/checks.yml`; compile with `-std=c11 -Wall -Wextra -Werror`, then run the resulting binary.

## Toolchain and Delay-Slot Lessons

- GCC 2.7.2 rejects the newer `+m` constraint; `=m` works when reserving stack storage.
- Its preprocessor is sensitive to backslash-newline macros in CRLF files. Keep compatibility macros such as `_Static_assert` on one line.
- MASPSX reads stdin when launched non-interactively. Use `--force-stdin` and pass the compiler assembly as subprocess input; otherwise it can wait indefinitely.
- Do not delete every MASPSX `# DEBUG: branch/jump` nop. For compiler-generated code, `tools/gcc_match.normalize_delay_slots` tracks the original `.set reorder`/`.set noreorder` mode and retains required delay slots. It rejects branch-expansion mismatches.
- For hand-authored inline assembly, put `.set noreorder` around explicitly ordered branches/calls. Restore `.set reorder` before a compiler-generated return if its final nop is required.
- Relocatable object bytes alone are not enough for calls and local jumps. The matching tool links at the symbol's real address, resolves only explicit `func_XXXXXXXX` helpers and configured globals, then compares bytes.
- A source with compiler-generated read-only data can declare `rodata: {"start": "0x800139A8", "size": 132}` in its matching profile. The linker places that section at the original address; the build checks its exact bounds and records its hash, and the progress builder independently compares all its bytes with the verified executable. A matching switch instruction sequence alone is not enough.
- A prior report omitted exact data because unused helper readonly tables were pulled into the Clang bundle. Keep completed GCC-only sources out of fuzzy Clang includes; preserve independent data-symbol byte checks instead of patching report totals.

## Current Work Queue

The seven actor sequence helpers, model-resource selector, and assembly-backed death handler match under checksum-pinned GCC 2.6.3/MASPSX; this establishes a useful matching profile, not the original game's compiler. GCC 2.7.2 differed on the remainder helper's multiply-high register selection. `bubsy_process_actor_event` now matches all 316 bytes from readable C using register constraints and empty barriers. `shared_actor_state_2_handler` matches all 2,200 bytes from high-level C; the per-source `maspsx_flags` (`--expand-div`) and `small_data` (gp base plus symbol list) options in `tools/gcc_match.py` reproduce the original divide trap checks and `$gp`-relative small-data accesses. `shared_actor_state_dispatch` matches all 516 bytes from high-level C under its per-function `-fno-cse-follow-jumps` profile. Its gameplay semantics remain provisional; the host-testable models cover the opening key/mask clear, mode-1 numeric update, and mode-0 clamp, while its target-only C candidate models the observed mode-2/3 callback gate and temporary progress behavior.

When aiming for a progress threshold, calculate required **exact bytes** from the current report denominator and prioritize functions with clear boundaries and tractable control flow. A large function is useful only if it can be finished and byte verified; its fuzzy percentage does not advance exact progress.

### Converting Existing Byte Matches to C

The readable-C conversion pass has started with `func_8002E7D8`,
`func_8002E7FC`, and `func_80053A08`: 180 bytes previously supplied by assembly
transcriptions now compile byte-exactly from plain C. The first two require
`-fno-omit-frame-pointer` and a mapped gp-relative small-data byte. The vector
wrapper matches using natural register allocation after removing the old
volatile pointer and register pins. All three have native behavior tests.

The complete 3,228-byte grounded handler at `0x8004B35C` now also matches from
readable C in `bubsy_grounded_state.c`, replacing its assembly transcription.
Its profile requires `-fno-cse-follow-jumps`, gp-relative counter/event globals,
two argument register pins and an empty compiler barrier. All 132 bytes of
its compiler-generated switch table also match at `0x800139A8`. The separate
native execution harness tests helper call order and state mutations; the
older prelude and case models have been corrected where instruction review
exposed discrepancies.

The 1,380-byte movement preparer `func_80025A98` also matches from plain C.
It needs only the existing compiler flags and the mapped gp-relative global
transform pointer, with no register pins, barriers, or instruction snippets.
The native harness verifies table transitions, fixed-origin vectors, helper
failures, transform thresholds, and notification-before-enqueue order.

The newly recovered grounded stride producer `func_80035ACC` matches all
1,304 bytes from readable C. Its profile uses `-fno-cse-follow-jumps`,
three pointer register constraints, one empty memory barrier, and an
independently sampled final cap-mode read; there are no instruction snippets.
The native harness checks 9,216 boundary combinations, helper order, wrapping
delta updates, assertion continuation, and pointer/mode/stride mutations.

The complementary deceleration producer `func_80038844` matches all 968 bytes
from plain C under the standard profile, with natural register allocation and
no assembly, barriers, or register pins. Its native harness checks 1,310,720
boundary combinations and helper mutations, including the independent stopped-
mode sample, unconditional nonnegative-increment flag reset, saved state bit,
wrapped decrement and negation, pending X/Z gates, and prospective stride bounds.

The next conversion batch replaces nine assembly-backed matches (1,852 bytes):

| Function | Bytes | Target C constraints |
|---|---:|---|
| `game_enqueue_move_request` | 112 | One register pin and empty barrier |
| `func_800250CC` | 160 | None |
| `func_8004FF50` | 156 | None |
| `func_8004FFEC` | 164 | None |
| `func_8004EC20` | 220 | Two scalar pins, empty barriers, `-fno-cse-follow-jumps` |
| `func_8004ECFC` | 124 | None |
| `func_8004ED78` | 116 | None |
| `func_80037B30` | 484 | Three short-lived pointer pins |
| `bubsy_process_actor_event` | 316 | Register pins and empty barriers |

Instruction snippets have also been removed from `func_80024F2C`,
`vector_range_scale_q12`, `func_80053FA4`, `func_800540BC`, `func_800722C8`,
`func_80062E50`, and `func_80022944`, preserving all 1,692 bytes of those seven
existing C matches.
The basis shortcut uses four declared scalar aliases of the global matrix
words to prevent address CSE, rather than literal `lui`/load instructions.
The signed halfword comparison discards volatile qualification only for that
single original-width sample; it does not combine reads across helper calls.
The native harnesses cover queue callback mutations, list failure/removal,
camera slot bounds, case-37 object replacement, overlapping shortcut vectors,
matrix-copy call order, and 262,144 wrapping-distance combinations.
The movement-step harness executes the actual `func_80022944` C implementation
over 73,695 boundary combinations, including signed limits, step halving,
clamp bypasses, all three direction gates, transformed-vector accumulation,
and mutations during assertion, special-state and multiply callbacks.
Its shared layouts are declared in `src/game/move_step.h`; the arithmetic
uses explicit wrapping intermediates and contains no instruction snippets.

The allowlist now contains 126 C-based function matches (25,348 bytes) and
27 assembly-backed matches (17,340 bytes), including repeated module copies.
Of those C-based matches, one still contains real instruction snippets:
`func_8001007C` (36 bytes). The other 125 matches (25,312 bytes) contain no
instruction transcriptions; some require
register constraints, symbol aliases, or empty compiler barriers, so they are
not all unconstrained ISO C.
The overall exact total remains 42,688 bytes across 153 matches; converting
an already matched function to C does not increase that total.

The remaining assembly-backed implementations have all been inventoried:

| Implementation | Bytes per copy | Copies | Remaining work |
|---|---:|---:|---|
| `bubsy_handle_death_state` | 1,920 | 1 | Large inline-assembly blocks |
| `func_800347EC` | 1,804 | 1 | C candidate has matching size; two instruction-order groups still differ |
| `func_80015A54` | 2,080 | 1 | Full move-request caller reconstruction |
| `func_8003CC78` | 3,168 | 1 | Full move-request caller reconstruction |
| `func_80041218` | 2,920 | 1 | Full move-request caller reconstruction |
| `func_800100A0` | 136 | 1 | Signed multiply-high/low register ordering |
| `func_80010128` | 248 | 20 | Signed multiply-high/low register ordering |
| `func_80077298` | 352 | 1 | GTE coprocessor transfers and commands |

An ordinary 64-bit C multiplication candidate for `func_8001007C` emitted
44 bytes with register barriers, or 36 bytes with different registers and
instruction order without them. It was not promoted. A portable arithmetic
model is not proof of an exact C match, and the GTE routine cannot simply be
replaced with scalar math while claiming its original COP2 bytes are retained.

The uncredited `src/player_model/bubsy_actor_347ec.c` candidate now emits
1,804 bytes. Its remaining differences are the entry saved-register store
ordering and the final transition-index store ordering. Its state-bit branches,
fallback flag clear, and mode-zero state writes have been corrected against
the original instructions, but equal size is not an exact match.
The registered assembly transcription remains unchanged.

For each remaining conversion, retain the assembly-backed profile until the
C build passes the complete linked-byte comparison. Then switch the profile,
remove the superseded assembly source, and update its behavior tests and
documentation. The larger actor handlers, move-request callers,
and repeated vector-helper transcriptions remain conversion work,
not completed readable-C decompilation.

## Safe Repository Hygiene

Original executables, disc assets, downloaded toolchains, generated disassembly, objects, linked ELFs, and proof JSON belong under ignored `original/` or `build/` locations. Do not force-add them. Before committing, inspect `git status`, check the full diff, run `git diff --check`, rebuild and validate the report, and stage only source/config/docs/tests plus `config/code-report.json`. Push only after the report workflow's validation passes.
