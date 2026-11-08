# Bubsy Player Code: First Pass

This is a byte-grounded first decompilation pass, not a finished matching C
implementation. The USA executables are still the authority for behavior.

## Model Selection

The shared L0 level routine at `0x800296B4` is identified by four embedded model
paths and its callers. It combines a runtime model-mode byte with a config flag:

| Mode byte at `0x801D89F0` | Config byte `+0x2A2A`, bit `0x10` | Model path |
| --- | --- | --- |
| zero | clear | `BUB.TZP` |
| nonzero | clear | `BUBSWIM.TZP` |
| zero | set | `PLISKIN.TZP` |
| nonzero | set | `PLISWIM.TZP` |

The model-mode getter at `0x800283E0` reads byte `0x801D89F0`; nonzero selects
the `*SWIM.TZP` resources. This getter is byte-matched in C. The active config
pointer comes from `0x800BE698`, and bit `0x10` at offset `0x2A2A` selects
Pliskin-named resources when set and Bubsy-named resources when clear. The
config structure and broader meaning of the mode byte remain unknown.

The selected path is passed to the resource loader at `0x8001BA10`, together
with the output slot at `0x801D8A08` and current size/value at `0x801D8A0C`.
The returned value replaces `0x801D8A0C`. A zero result triggers the engine
assertion helper at `0x800556D0` with source `../f/level.c`, line `0x495`.
The getter at `0x80029790` copies the asset pointer from `0x801D8A08`.

The initial C reconstruction is [player_model.c](../src/player_model/player_model.c).
Its boundary uses an injectable state and loader interface so the observed
selection logic can be tested without pretending the surrounding game globals
or resource system have been fully reconstructed. The native harness covers
all four filename branches and the failed-load assertion path.

Both getters have C implementations in
[player_model_global.c](../src/player_model/player_model_global.c). With
`BUBSY3D_MATCH_ORIGINAL_L0`, Zig 0.14.1's bundled Clang compiled each 16-byte
function to its original L0 bytes exactly. The model-pointer getter uses an
experimental MIPS-II scheduling profile to place the store in the `jr` delay
slot; the emitted instructions are MIPS-I. This profile is specific to these
two small getters and does not identify the original Bubsy compiler.

The validated report currently credits 63,972 exact code bytes across 263
functions, or `2.084216%` of mapped code. The total includes two Clang-built
getters, the assembly-backed death handler,
the readable-C grounded-state handler, the readable-C shared transition
`func_800347EC`, the movement consumer
`game_apply_queued_move_request`, and the queue insertion and drain routines
`game_enqueue_move_request` and `func_800250CC`, the event-record helper
`func_8004FF50` and its remover `func_8004FFEC`, the exact case-37 initializer
`func_80037B30`, the exact case-17 helpers `func_8001C700` and `func_8005E7B8`,
plus the component-transform helpers
`func_80054B20` and `func_80054C7C`, plus the grounded movement-vector producer
`func_80022944`, the readable-C stride producers `func_80035ACC` and `func_80038844`,
the readable-C movement preparer `func_80025A98`, its shared initializer
`func_8002611C`, setup wrapper `func_80025FFC`, move-info allocator
`func_800222EC`, its component-basis allocator `func_80054624`, the
record/sequence/runtime helpers and conditional input-to-turn path detailed
below, and the five grounded-path helpers `func_800556D0`,
`func_8001B558`, `func_80034EF8`, `func_8003539C`, and `func_80039158`,
and the three bounded
move-request caller transcriptions `func_80015A54`, `func_8003CC78`, and `func_80041218`,
20 assembly-assisted C copies of
`func_80010128`, the L0 assembly-assisted Q20.12 dot product `func_800100A0`, and
the assembly-assisted Q20.12 scalar multiply `func_8001007C` and C distance approximation
`func_80062E50`, the exact C squared-difference nonzero test
`move_vector_has_nonzero_squared_length` (linker label `func_80053904`), the
C-built forwarding helper `func_800539E4`, and the C-built grounded-case
global-byte helpers `func_8002E7D8` and `func_8002E7FC`, and the
C-built 104-byte vector-scale wrapper
`func_80053A08`,
the C-built pending-vector conversion `func_800536F4` and grounded-handler
forwarding wrappers `func_8002262C` and `func_80022650`,
their C-built bit-query/set/clear helpers `func_80056678`, `func_8005668C`,
 and `func_800566E8`, the actor `+0x04` clear-bit wrapper `func_8002269C`,
the fixed-pointer initializer `func_8003AA1C`, and the GP-relative accessors
`func_8001DCE0` and `func_800516F8`,
and the C-built component-basis matrix helpers `func_80054414` and
`func_80054488`, plus basis normalization helpers `func_800546F4` and
`func_8005476C`, basis reconstruction `func_800542A4`, the temporary-rotation helpers `func_800547C8` and
`func_800549E8`, column scaling `func_8005541C`, and the inverse-basis wrappers `func_8005406C` and
`func_80054120`,
the C-built `vector_length_q12`, `vector_range_scale_q12`, and
 `vector_range_condition_q12` functions (their exact-match linker labels remain
`func_8005385C`, `func_8005395C`, and `func_80053D68`),
22 C copies of
`func_80072F4C`, and 38 repeated copies of two actor-flag/cursor helpers.
`bubsy_process_actor_event` matches all 316 bytes from readable C in
`src/player_model/bubsy_actor_events.c`, using register constraints and empty
barriers but no instruction snippets. `shared_actor_state_2_handler` is now
byte-matched from high-level C in `src/player_model/shared_actor_state_2.c`.
The progress site changes only after this snapshot is committed, pushed, and
 published by GitHub Actions.

### Shared Actor Helper `func_800347EC`

`func_800347EC` is a bounded 1,804-byte routine at `0x800347EC..0x80034EF8`
with four direct callers. Its entry returns immediately when actor flag
`+0x04` bit `0x04` is set or lock byte `0x80186479` is nonzero. Otherwise
 actor flags `+0x04` mask `0x1100` select property calls `2` and `6`; a clear
mask continues into the caller-selected mode path. This entry gate is modeled
and covered by host tests.

The full 1,804-byte range now matches from readable C in
`bubsy_actor_347ec.c`, built with pinned GCC 2.6.3/MASPSX and linked at its
verified address; the former assembly transcription was removed. Five register
pins and one empty entry barrier reproduce the original global register
allocation without emitting instructions. Two source-shape facts made the
remaining store ordering natural. First, the transition call reads the object ID
through the actor struct member `object_id_0C`. Second, the player globals at
`0x80186450` are one structure. GCC 2.6 then keeps those stores ordered relative
to actor-field accesses without memory barriers. The host-side planners and
tests cover the branch decisions and data changes. Engine-call effects and
gameplay meanings remain unresolved.

In nonzero mode, state mask `0x44` clear selects the regular path: it clears
actor byte `+0x00`, calls `0x8002269C(actor, 1)` and `0x80054B20`, optionally
 calls properties `3` and `4` for state mask `0x18`, then calls `0x80022C94`.
The selector arguments depend on masks `0x9000` and `0x10000` and `$gp+0x37C`;
the default first stack value is `0x29B`, changed to `0x294` when mask `0x18`
also causes property calls `3` and `4`;
the path then requests transition `0x8001B558(0x27, 4, ...)` and returns.
With mask `0x44` set, the alternate selector uses stack values `0x2EA/0x2EF`
for mask `0x9000`, `0x2FF/0x302` for mask `0x10000`, or `0x33A/0x2A7` by
 default. Its follow-up uses the saved component value: state mask `0x8000`
may dispatch with global `0x80186450`; mask `0x10000` may dispatch mode `1`
and clear `$gp+0x37D`; mask `0x1000` plus value `0x2EA` transforms companion
context and clears state/global values; mask `0x100` with signed counter
`0x801864BA >= 0xFE` writes linked word `+0x4C` to `-0x400` or `+0x400`,
clears the counter, and clears state bit `0x100`.

If that alternate path has actor state bit `0x04` clear, it enters a shared
 fallback. The fallback can take a special sequence-`0x2B0` path when state bit
`0x40` is set and actor byte `+0x00` equals `1`; otherwise it calls properties
`5`, `8`, and `0xA`, resets globals, and handles optional property `0xC`.
Actor flag `+0x04` bit `0x02` gates the unsigned object bound: clear skips the
bound check, a failed bound calls the assertion helper, and a successful bound
updates `$gp+0x380`, actor byte `+0x01`, transition `0x8001B558(0x27, 5, ...)`,
and actor flags. State mask `0x18000` selects properties `0xF` and `0x10`;
 otherwise globals `0x80186464/0x801864B8` are reset. `$gp+0x37C` controls the
final call to `0x8003539C(actor, 1)`.

In mode zero with state mask `0x44` clear, actor byte `+0x00` becomes `1`;
mask `0x18` additionally calls properties `3` and `4`. The path then calls
`0x80022C94`, requests transition `0x8001B558(0x27, 4, ...)`, and calls
`0x8002925C` and `0x80026A0C` with observed arguments. With mask `0x44` set
and bit `0x40` clear, it writes actor byte `+0x00 = 1` and returns. When bit
 `0x40` is set, selector result byte `1` returns without further writes; every
other result resets globals, chooses actor byte `+0x01` from `0x80186464`, and calls
`0x8002944C`. If actor flag bit `0x02` is set, an unsigned object-bound check
either updates `$gp+0x380` and requests transition mode `4` plus
`0x8002269C(actor, 1)`, or calls the assertion helper. These are observed
transitions, not recovered gameplay meanings.

## Bubsy Actor Routine

 `0x8003539C..0x800358C8` is separately rooted as `bubsy_update_actor_state`.
Its embedded assertions refer to `../f/bubsy.c` at lines `0x41A` and `0x495`,
and it takes an actor-like pointer in `$a0` and a second mode byte in `$a1`.
The caller at `0x800371D0` passes `$a1 = 1` and also writes `0x41` to actor
offset `0x2D`. The routine reads actor offsets `0x04`, `0x10`, `0x1C`, `0x2B`,
and `0x6D`; tests animation/property IDs including `0x309`, `0x32E`, `0x35`,
`0x54`, `0x9A`, and `0x3E0`; and uses shared globals including offsets `0x388`
and `0x38C` from `$gp`. These facts suggest actor interaction/state processing,
but they do not yet prove the meaning of those IDs or fields. The precise C
function name, field types, and full gameplay semantics remain unknown. The
`bubsy_update_actor_state` name is a searchable provisional label, not a
recovered original symbol. It calls helpers at `0x80052E18` and `0x80052E30` that
set and clear bit `0x04` in actor byte offset `0x04`. The C translations live in
`player_actor_flags.c` and pass native state tests. A compiler sweep found
checksum-pinned GCC 2.6.3 matches all seven helpers in that source file,
including the multiply-high register choices in the boundary and remainder
helpers. It also matches the C model-resource selector and the assembly-backed
death handler. GCC 2.7.2 differed on the remainder helper's multiply-high
register selection. The standard profiles now use 2.6.3 for these verified
functions. This is stronger byte-match evidence, not proof that the whole game
used that compiler or recovery of all gameplay meanings.

The update entry first reads signed halfword `0x80186454`. If nonzero, it
forces its local `+0x24` state byte to zero and clears actor `+0x10`. If zero,
nonzero update mode `a1` instead clears byte `0x8018647A` and `$gp+0x38C`. A
nonzero actor `+0x10` then takes the direct epilogue path. Otherwise the update
clears byte `0x80186460`, dispatches actor events in mode `0`, and computes the
current sequence key as actor-component halfword `+0x0C - 2`.

The entry gate is translated in `bubsy_actor_update.c` and covered by host tests
for the zero-gate, nonzero-actor-state, and forced-clear branches. The complete
[bubsy_actor_state_update.c](../src/player_model/bubsy_actor_state_update.c)
matches all 1,324 bytes with pinned GCC 2.6.3/MASPSX and LLD. It uses register
constraints and empty compiler barriers, not instruction transcription. Native
tests and 1,536 deterministic original-versus-C MIPS cases cover the observed
counter, sequence, phase, message, and helper-mutation paths; those sampled
cases are behavioral evidence, not proof for every possible input.

Actor flags at `+0x04`, bits `0x80` and `0x100`, steer distinct sequence paths. On
the `0x100` path, the local state byte `+0x24` is cleared when `$gp+0x7EC` is
zero. With that flag clear, sequence key `0x32E` sets local state to `1` when
the runtime mode is zero. This local-state predicate is translated and tested
in `bubsy_actor_update_local_state_for_sequence`.

In runtime mode `1`, the updater checks the sequence boundary for current keys
`0x9A` and `0x54`; this gate is translated and tested in
`bubsy_actor_update_should_check_mode1_sequence_boundary`.
After a successful boundary check, it increments `$gp+0x38C` and clears the
counter when mode `1` reaches `12`, the level ID is `0x14`, or actor flags
`+0x04` contain `0x40`. This reset predicate is tested in
`bubsy_actor_update_should_reset_sequence_counter`.
For levels other than `0x14`, the later phase-processing path is entered only
in runtime mode `0`, with a positive sequence counter and actor flag `0x40`
clear. If update gate `0x80186454` is zero, it begins the phase and submits
event `0x2C`; the initial gate is modeled and tested by
`bubsy_actor_update_should_start_counter_phase`. Gate values `3` and `2`
select the subsequent ready and message paths.

On the `0x80` path, it selects index `0x309` only when `$gp+0x7EC` is zero, the
current key is not `0x309`, and actor state `+0x10` does not contain mask
`0x00800000`. This predicate is translated and tested in
`bubsy_actor_update_should_select_309`; a nonzero result from
`player_actor_select_sequence_state` triggers the assertion at
`../f/bubsy.c:0x41A`. In the later
local-state path, local state `1` selects index `0x35` in runtime mode `0`; in
nonzero runtime modes it selects `0x9A` when actor mask `0x818` is set, else
`0x54`. This choice is translated and tested in
`bubsy_actor_update_sequence_index_for_local_state`. A nonzero result from the
subsequent state selection triggers `../f/bubsy.c:0x495`; the update then sets
actor flag `0x04` and calls the shared 48-caller sequence routine at
`0x80052BDC` with mode `0`. These branches and IDs are instruction-grounded,
but their gameplay labels (movement, animation, interaction, or scene state)
remain unresolved.
For local state `1` in any nonzero runtime mode, the updater also writes
`-0xF40` to `0x801864A0`; the value selection is translated and tested in
`bubsy_actor_update_get_sequence_mode_override`.

Immediately following that updater is the shared routine now mapped as
`shared_actor_state_dispatch` (`0x800358C8..0x80035ACC`). It has eight direct
callers and takes an actor pointer, a component-basis context, and a mode;
grounded cases use modes `0`, `1`, `2`, and `3`, so it is not Bubsy-only.
It queries the actor component's sequence key, tests actor mask `0x1100`,
updates stride/increment fields `+0x30/+0x34` in the captured actor
`shared_28` move-info pointer, and calls the movement-step producer.
These fields are not component fields. The complete C reconstruction in
`src/player_model/shared_actor_state_dispatch.c` is now byte-matched: the
pinned GCC 2.6.3 profile reproduces all 516 bytes when compiled with
`-fno-cse-follow-jumps` added to the usual flags. Under plain `-O2`, CSE
follows the taken `mode == 0` branch and rewrites the mode-zero progress
store to reuse the mode register instead of literal zero, which differs at
one instruction word; the extra flag is a reproduction profile, not a claim
about the original toolchain. Its initial branch is translated in
`player_actor_shared_state_clear_progress`: key `0x2A7` or any overlap with
actor mask `0x1100` clears move-info words `+0x30` and `+0x34`.
In mode `1`, actor state mask `0x8000` bypasses the numeric update; otherwise
mask `0x10000` sets `+0x30` to `-0x34`. When that mask is clear, `+0x34` is
temporarily set to zero if `+0x30 < -0x11`, or to `-0x198C` otherwise; the
callback path then restores its saved value. The scalar update is translated in
`player_actor_shared_state_apply_mode1_numeric_update`. The actual matched
dispatcher also executes the callback and the remaining modes. The
mode-0 numeric stage clamps `+0x30` to `5..0x11` unless actor state mask
`0x18000` bypasses it. Before the callback, `+0x34` is temporarily set to
`0x198C` below the upper cap or cleared at the cap, then restored by the shared
epilogue. The surviving clamp is translated in
`player_actor_shared_state_clamp_mode0_progress`. Modes `2` and `3` skip their
callback path when actor state mask `0x10000` is set; otherwise they snapshot
the context basis through `func_800546F4`, rotate it through `func_800547C8`
using angles `[0, -0x400/+0x400, 0]`, and call `func_80022944` with temporary
stride `14`, or `8`/`6` for state bit `0x8000` and model mode zero/nonzero.
State/model values are sampled after the rotation helpers. The dispatcher then
restores the basis through `func_800549E8`, restores the saved stride, and
restores the saved increment in its common epilogue. It retains the original
move-info pointer even if a helper replaces `actor->shared_28`.

The 216-byte `func_800547C8` and 132-byte `func_800549E8` now match from
plain C with natural register allocation and no inline assembly or barriers.
The rotation wrapper saves all 32 basis bytes, converts the low halfwords of
the input angles with wrapping X/Z negation, and writes back only nine matrix
halfwords. A null component returns `0x18`. Restoration sequentially copies
nine halfwords, negating odd indices; padding and translation are unchanged.
The [native harness](../tests/test_grounded_rotation.c) executes both helpers
and the actual matched dispatcher, checking 405 overlap cases, 343 angle
cases, all 65,536 halfword direction values, 1,872 dispatch boundaries, and
helper mutations. The 368-byte `func_800542A4` basis reconstruction now also
matches plain C without register constraints or barriers. It samples all three
signed input columns before any callback, conditions columns two and three,
rebuilds the first through their cross product, conditions it, rebuilds the
third through another cross product, and normalizes all three. It stores only
the resulting nine low halfwords, leaving padding and translation untouched.
The helper ignores every callee status, exactly as the original does.
The [basis harness](../tests/test_grounded_basis.c) checks 131,072 reconstruction
cases, including all halfword signs, in-place output, ignored error statuses,
and callback mutation of the component after initial sampling.

That harness also executes the already-matched 476-byte `func_800550A4`
orientation builder used by movement preparation. It verifies translation
snapshots, the zero-direction exit, the vertical-direction X/Z fallback,
preserved translation/padding, and propagation of subtraction/cross-product
failures in 120 cases. Its normalization/conditioning statuses are ignored;
failure before the final matrix copy preserves the component's basis except
for callback mutations. Rotation backends `func_80077C50` and `func_80076890`
and the cross-product/normalization internals are still not recovered in
instruction-free C; these wrapper matches do not establish scalar GTE recovery.

The preparation path's `func_8005541C` at `0x8005541C..0x80055674` now matches
all 600 bytes from plain C with natural register allocation. It visits matrix
elements in column order `0/3/6`, `1/4/7`, `2/5/8`, multiplying each signed
halfword by the corresponding 32-bit Q12 scale through `func_8001007C`.
Each callback's product is sampled into a separate local and clamped to
`-32767..32767` before the halfword store; `-32768` is deliberately excluded.
The scale is reread for every element, so callback mutations affect subsequent
samples. All multiply statuses are ignored; padding and translation are
untouched except for callee mutations. The same basis harness covers 589,824
input/scale combinations plus 24 forced-product clamp and mutation cases,
including signed Q12 rounding and low-32-bit product wrapping. The multiply
backend remains an instruction-assisted C match, not recovered plain C.

The shared movement initializer `func_8002611C` (`0x8002611C..0x80026398`)
now matches all 636 bytes from plain C without register constraints or inline
assembly. Its direct caller at `0x800260EC` is in `func_80025FFC`, which
allocates missing move-info and initializes the shared target transform before
calling this routine. This is shared movement infrastructure, not evidence of
a controller-input routine or a Bubsy-only action.

The initializer captures `actor->move_info_28` before querying the movement
table. It clears index/counter, sets mode byte `+0x1C` to one, snapshots the
first table payload, and adds its X/Y/Z to move-info origin `+0x58` with
32-bit wrapping. Selector one rereads the current index and sets stride to
twice that entry's selector byte. It computes separation from origin to target,
captures the actor component's transform after that callback, and calls target
translation, basis snapshot, orientation construction, and column scaling in
order. Table/separation errors use the original assertions and continue if
the assertion callback returns; later transform-helper failures return zero.

Flags one/four enable direction extraction. Flag one always keeps the vector;
flag four alone keeps it only outside inclusive Y range `-3275..3275`. The
keep path copies all 16 bytes into `+0x88`, then restores the saved basis.
The clear path zeros only X/Y/Z, preserving the fourth word. There is no
movement-step callback or queue insertion in this initializer. The original
direction extractor defines only X/Y/Z; the copied fourth word is scratch
padding, not a recovered semantic field. The
[native harness](../tests/test_move_initializer.c) checks 216 flag/direction/
selector combinations, exact helper order, six failure exits, assertion
continuation, wrapping target sums, and captured-pointer/helper mutations.

The 288-byte setup wrapper `func_80025FFC` (`0x80025FFC..0x8002611C`)
now also matches plain C without register constraints or inline assembly.
An existing `actor->move_info_28` returns one immediately, without querying
the table, allocating, creating a transform, or rerunning initialization.
With no move-info, it queries the movement table and returns one for zero
entries or a zero first-entry limit. A non-one table-query byte calls the
original assertion; execution continues if that callback returns. It then
calls `func_800222EC` with the actor's move-info pointer address and supplied
origin. A zero allocation-result byte returns zero; a zero actor context
sampled after allocation returns one without transform setup.

Otherwise a null shared transform calls `func_80054624`; nonzero status
returns zero. A ready transform bypasses creation. Finally the wrapper calls
`func_8002611C` and normalizes only its low result byte to zero/one, so
`256` and `-256` are false while `255` and `-1` are true. The
[setup harness](../tests/test_move_setup.c) covers 176 result/allocation/flag/
global combinations, repeat-call idempotence, all early exits, assertion
continuation, and allocation-time context/global mutations.

The move-info allocator `func_800222EC` (`0x800222EC..0x80022484`) now
matches all 408 bytes from plain C without inline assembly, barriers, or
register constraints. It requests `0xA8` bytes from `func_80082EEC` and
publishes the allocation through the output pointer before checking for null.
Null returns zero without reading the origin; success returns one.

It sets index to minus one and selectively clears counter, target XYZ,
separation, stride/increment, pending XYZ, rotation XYZ, direction XYZ, and
the auxiliary XYZ fields at `+0x78` and `+0x98`. After the initial clears,
it samples origin separately per axis and writes each sample to both
`+0x58` and `+0x68`. The output pointer is reread between field stores;
each origin sample captures that pointer for its two destination stores.
This ordering preserves the original behavior when origin overlaps the
allocation. It does not initialize the whole object: mode `+0x1C`, vector
padding, opaque bytes, and the final four-byte tail at `+0xA4` are untouched.
The named C layout covers `0xA4` bytes of the `0xA8` allocation.

The [allocation harness](../tests/test_move_allocation.c) compares the entire
allocation against an ordered byte-level model over 343 origin boundary
combinations and 1,792 overlap/fill cases, plus null allocation with a null
origin. Native and ASan/UBSan runs pass.

The 140-byte transform allocator `func_80054624` now matches plain C in
[func_80054624.c](../src/game/func_80054624.c). It asks `func_80082EEC` for
32 bytes, publishes the returned pointer even on failure, returns `3` for
null, and otherwise copies all 32 bytes of the live template at `0x800A42EC`.
The lower wrapper dispatches BIOS vector `A(33h)` (`malloc`); its adjacent
`func_80082EFC` wrapper dispatches `A(34h)` (`free`). The real setup, allocation,
and record-table lookup are exercised together in
[test_transform_allocation.c](../tests/test_transform_allocation.c). Notably,
transform-allocation failure leaves move-info installed: a repeat setup call
then returns success without retrying the transform allocation. This is original
behavior, not a proposed recovery strategy.

### Shared Lookups and Conditional Input-to-Turn Trace

The record-pointer global is `0x801D89A4`, and the signed record count is
`0x801D8994`. The three lookups now match plain C and share the target's
36-byte record layout in [level_records.h](../src/game/level_records.h):

- `func_800286F8` returns zero for a negative index; otherwise it writes the
  selector byte at `+0x04` and returns one. It does not check the upper bound.
- `func_80028764` returns null for a negative index, the record's `+0x1C`
  object for an in-range index, or the result of `func_80040FC0` otherwise.
- `func_80028C18` rejects negative and out-of-range indices, then writes the
  table pointer at `+0x14` before its count at `+0x10`. It rereads the global
  record pointer between these writes. This ordering was checked against the
  original with overlapping outputs; the volatile pointer declaration is
  intentional.

The other newly matched dependencies include the runtime sample getter
`func_8001AE88`, readiness-byte getter `func_800222E0`, and 19-entry update
loop `func_8002C488`. The sequence wrappers `func_8001C6BC` and `func_8001C7C4`
use the shared table at `0x80182F64`; the latter initializes an output byte,
reports backend failure through the assertion callback, and returns the byte
after that callback. Their native tests cover return values, signed narrowing,
callback mutation, and lookup boundaries.

The following controller-to-turn-event path is established for the default
tables and the indicated runtime gates:

1. The matched [func_8004FCBC](../src/game/func_8004FCBC.c) registers two
   eight-byte controller buffers at `0x8018CA48` and `0x8018CA50` with BIOS
   pad services and starts polling. The matched
   [func_8004FCF8](../src/game/func_8004FCF8.c) accepts a buffer only when
   its status is zero and its type is `0x41` (digital pad), complements its
   two active-low button bytes, and combines port 1 in the low halfword and
   port 2 in the high halfword. Other controller types are rejected.
2. The matched [func_8001A6E0](../src/game/func_8001A6E0.c) polls those
   buffers. In normal input mode zero it queues
   `func_8001B558(3, 0, 0xFFFE, 0xFFFE, &buttons, &port1, &port2, 0)`.
   Mode one can instead select a word from an indexed input stream; the
   original's stop, effect, sample, and index-reread ordering is preserved.
   The stream sample publisher [func_8004FF28](../src/game/func_8004FF28.c)
   also matches. The native
   [controller tests](../tests/test_controller_input.c) cover all 65,536
   button patterns, while the
   [poll integration tests](../tests/test_controller_poll.c) exercise both
   ports, stream boundaries, and callback mutations.
3. The matched [func_8002278C](../src/game/func_8002278C.c) publishes the
   default mode table at `0x8009EC8C`. Its **event-3** callback and payload
   initializer are `func_80023E94` and
   [func_80023A0C](../src/game/func_80023A0C.c) at `0x8009ECB4` and
   `0x8009ECB8`. These same addresses also alias static event-10 entries; the
   physical poller uses event 3 with `0xFFFE` flags and heading. The
   initializer copies the bitmap and digital-pad status bytes to event-record
   offsets `+0x0C`, `+0x10`, and `+0x11` in order.
4. When its gates allow the bitmap branch, `func_80023E94` uses tables at
   `0x8009EEF0` and `0x8009EFF0`. For the first port, the digital-pad
   protocol's directional right and left bits become bits 13 and 15 in this
   game's byte-swapped bitmap; see the
   [PSX controller bit layout](https://psx-spx.consoledev.net/controllersandmemorycards/#controllers-standard-digitalanalog-controllers).
   In the first four observed mapping rows, those active bits queue events 8
   and 10 respectively; their cleared-bit events are 9 and 11.
   Original-MIPS execution using synthetic BIOS buffers exercised all 16
   active/cleared combinations across those rows, from the real pad reader
   through the real event initializer and callback, without stubbing helpers.
   In that test the callback was invoked directly with the queued record.
   Its matched helpers include the runtime-byte getter
   [func_8001BCE4](../src/game/func_8001BCE4.c), input-gate getters
   [func_8001BCF0](../src/game/func_8001BCF0.c) and
   [func_8001BD00](../src/game/func_8001BD00.c), sample reader
   [func_80050128](../src/game/func_80050128.c), and interval gate
   [func_80023A30](../src/game/func_80023A30.c). The interval is narrowed to
   a signed halfword, added to the previous sample with unsigned wrapping, and
   the previous sample is updated only if the new sample meets the threshold.
   [Native gate tests](../tests/test_input_event_gates.c) cover byte values,
   signed interval boundaries, and counter wraparound.
   Separately, when the input-gate byte is nonzero, the callback adds `100`
   to its reference with 32-bit wrapping and compares the result to the
   `func_8001AE88` sample as signed words. If the threshold is strictly less
   and the current bitmap is nonzero, it stores `40` at `0x800BDEFE`, calls
   `func_8001C04C`, and leaves before bitmap mapping. Original-MIPS tests cover
   equality, signed boundaries, and wraparound in that comparison.
5. `func_8001AEC0` initializes the configuration at `0x801D891C`, and the
   matched [func_80027F08](../src/game/func_80027F08.c) publishes its heading
   table and copies parameters through
   [func_80044C34](../src/game/func_80044C34.c). With the default table,
   heading slot zero is `func_80036018`; the queue processor
   `func_8001B1E4` can dispatch the generated events through that slot.
   `func_80036018` looks up the actor by the event's `+0x08` field and, when
   its state gates allow it, calls the matched grounded handler. Cases 8 and
   10 reach `func_80034EF8`, which sets the turn-direction global from
   `state->id == 8` and can write a signed turn at move-info `+0x4C`.
   The matched movement consumer `func_80024F2C` subsequently passes a
   nonzero pending rotation at `+0x48` to the matched
   [func_800548A0](../src/game/func_800548A0.c), before clearing that vector.

Additional controller-path helpers include these exact matches:
[func_8001BC08](../src/game/func_8001BC08.c) sets the event-runtime byte to `1`,
and [func_80061A64](../src/game/func_80061A64.c) stores its argument at
`0x800BE400` and returns zero.
[func_8001BC28](../src/game/func_8001BC28.c) looks up the actor, clears the
event-runtime byte, and reads the current context. If context bytes `+0x6D`
and `+0x2B` are clear and the low 16 bits returned by `func_80045278` are
nonzero, it increments the word at `0x800BDE98` with 32-bit wrapping, calls
`func_80045094` and `func_80069758`, and dispatches event 4 with the numeric
actor address plus `0x0C`. This address is `0x0C` even when the lookup returns
null. The [native reset tests](../tests/test_event_runtime_reset.c) and
original-MIPS tests cover the gates, call ordering, counter wrap, and null actor.
The matched [func_80045278](../src/game/func_80045278.c) reads the unsigned
status byte at `0x800BE1C2`.
[func_80045094](../src/game/func_80045094.c) sets a reset byte, runs a setup
callback, and walks the live entry count. For each entry it passes the whole
word to `func_80044458`, then rereads its first signed little-endian halfword
for `func_8004232C`. It clears the configured number of words, clears a second
flag, invokes `func_800439EC` twice followed by `func_80044268` and
`func_80069758`, and finally clears the reset byte. Callbacks can change the
current entry and the remaining entry count. The target match uses register
constraints and an empty frame-reservation barrier, not instruction
transcriptions. [Reset-resource tests](../tests/test_reset_resources.c) cover
callback mutations and clearing boundaries.
Its matched [entry dispatcher](../src/game/func_80044458.c) looks up the
indexed object, calls `func_80052BDC` with mode 1, then calls
`func_80044B24` with the original index and the low byte of its mode; its
result is narrowed to a byte. The matched
[tracked-object sweep](../src/game/func_8004232C.c) captures the list head
before looking up the reference, narrows the requested tag to a signed
halfword, and calls `func_80042170` only for matching objects that do not
reference the captured object and lack flag `0x200000`. It captures each next
node before its callback but reads later nodes live. The
[entry tests](../tests/test_reset_entry_helpers.c) exercise every tag value,
callback mutations, and byte-narrowing behavior. The entry dispatcher uses
target register constraints and empty barriers; the tracked sweep is plain C.
The [real-entry integration test](../tests/test_reset_real_entries.c) also
connects both helpers, the object getters, and proximity refresh to the reset
backend, including a callback that changes an entry between its two reads.
Its 224-byte [group-dispatch backend](../src/game/func_80044B24.c) also
matches readable C. It captures the selected group before the actor getter,
reads its entry count and indices live, skips the actor's current object, and
optionally checks whether another active group contains each entry before
preparing it. Target register constraints and empty barriers preserve the
original allocation and scheduling without instruction transcriptions.
The [dispatch tests](../tests/test_reset_group_dispatch.c) cover mode and
predicate byte-narrowing, empty counts, and callback mutations. Its 156-byte
[membership check](../src/game/func_80044A88.c) now matches the original from
C using target register constraints and empty barriers without instruction
transcriptions. It skips the current group, then returns 1 if any other active
group with a positive signed entry count contains the requested object index.
The [native membership and integration test](../tests/test_reset_group_membership.c)
and [original-MIPS tests](../tests/test_original_controller_route.py) cover
empty and negative counts, duplicate active groups, exclusions, randomized
membership combinations, and callback changes to a later group.
[func_80069758](../src/game/func_80069758.c) refreshes a list of objects near
the current reference. It snapshots the reference's X and Z, walks the live
linked list, and retains objects whose wrapping sum of absolute coordinate
differences is signed-less-than their nonzero signed radius. It increments the
16-bit count and stores the object before calling the assertion backend when
the new signed count is at least 40; the assertion is not a capacity limit and
may return. An assertion callback can change the count, the current list slot,
or its next link. The target match uses register constraints and empty
scheduling/reload barriers, without transcribed instructions. The
[native tests](../tests/test_proximity_list.c) and original-MIPS tests cover
null references, distance and overflow boundaries, assertion thresholds, and
callback mutations. The [full timeout-reset integration test](../tests/test_timeout_full_integration.c)
links the real cleanup, tagged-slot, reset, and proximity C helpers together
and checks both proximity refreshes and callback-sensitive event dispatch.
The proximity backend's [reference getter](../src/game/func_8005327C.c) and
[list getter](../src/game/func_800529D0.c) also match in plain C. The latter
copies the live list head through its output pointer and returns zero; the
[getter integration test](../tests/test_proximity_getters.c) uses both real
getters with the refresh routine.
[func_8001C04C](../src/game/func_8001C04C.c) handles the input timeout: unless
the initial event ID is `0xFE` or `0x16`, it checks `func_80062890`, reports
a nonzero result through the assertion helper, and calls `func_80048488`.
It then clears the input gate and calls the reset helper unless the current ID
is `0xFE`. After these callbacks it rereads the ID, flags, and heading and
dispatches the event unless its ID is `0xFE` or `0xFF`. Callbacks may change
the ID or re-enable the gate. The [timeout tests](../tests/test_timeout_dispatch.c)
cover all 256 IDs and callback mutations; the
[integration test](../tests/test_timeout_reset_real.c) uses both matched C
helpers together.
[func_80062890](../src/game/func_80062890.c) returns immediately when its
active byte is zero. Otherwise it runs four shutdown callbacks, conditionally
frees four live resource pointers when the protection word is zero, invokes a
final state callback, clears the active byte, and returns zero. Each pointer
is reread after earlier callbacks and is cleared after its own free callback;
the protection word is tested once before the frees. The
[resource tests](../tests/test_timeout_resources.c) cover empty and populated
slots, protection, callback mutations, and ordering.
[func_80027F40](../src/game/func_80027F40.c) passes the state at `0x800BE4F8`
to `func_80062C54`, then reads and frees the current model-asset pointer;
the first helper can change that pointer.
[func_8002C9D4](../src/game/func_8002C9D4.c) tests the low byte of its input;
for zero it conditionally calls `func_8002CA88(5, -1)`, then checks the live
model mode and, for mode one, calls `func_8002BC9C` and
`func_8002CA88(15, -1)`. It writes the input byte after these callbacks.

The optional stream path uses three matched texture-font helpers:
[func_800483C0](../src/game/func_800483C0.c) asserts
when the loaded flag is not `1`, then scans all 50 tags and invokes
`func_80050534` for matches; [func_80048504](../src/game/func_80048504.c)
scans only when that flag is nonzero and invokes `func_80050650`; both report
nonzero backend results through the assertion helper. Finally,
[func_80048C48](../src/game/func_80048C48.c) marks matching tags `-1` and
subtracts their count with 32-bit wrapping when loaded. A requested tag of
`-1` also counts entries already marked `-1`. The
[native tests](../tests/test_controller_related_helpers.c) cover all flag
bytes, signed tag truncation, counter wrap, assertion order, and callback
mutations. Original-MIPS tests check representative flag and tag boundaries
and callback mutations against the executable. A
[stream integration test](../tests/test_controller_stream_real.c) uses the real
poller and these three C helpers together for activation and release; hardware,
backend operations, and the final event dispatcher are stubbed.

The matched [func_80048488](../src/game/func_80048488.c) walks those 50 slots;
for every tag other than `-1`, it calls `func_80048504` and `func_80048C48`
with the tag read before either callback. Later slots are read live, so callback
mutations affect subsequent iterations. Both
[isolated sweep tests](../tests/test_texture_font_sweep.c) and the real-helper
integration tests cover this behavior.

The [queue snapshot helper](../src/game/func_8001B8B8.c) `func_8001B8B8`
now matches all 272 bytes from readable C; the same bytes occur at relocated
addresses in all 20 level executables. It reloads the pending count for each
mode and scans at most 100 slots, copying occupied records in mode order `0..4`
and preserving their slot order within each mode. A scan stops after
seeing the current count of occupied records; records in other modes remain
pending when the count reflects the occupied slots. A copied record's event ID
is cleared and the pending count is decremented. Its
[native tests](../tests/test_transition_snapshot.c) cover empty and full
queues, inconsistent counts, priority, preserved data, and 8,192 randomized
separate-queue and same-queue cases. A sparse-queue fixture also checks the
original helper under MIPS emulation.

The snapshot calls [func_80010220](../src/game/func_80010220.c) to copy seven
aligned words per record. Its 36-byte readable C matches the identical helper
in all 22 executables. The general helper requires valid aligned arrays and a
nonzero word count; it copies forwards even when ranges overlap, so it is not
`memmove`. Its native test covers 15,000 lengths, offsets, and overlap cases.

The event processor also calls [func_80032D7C](../src/game/func_80032D7C.c),
whose 120 L0 bytes now match from plain C. When its byte flag is nonzero, it
gets the reference component, passes it and the destination global to
`func_80054A6C`, then calls `func_8003ABE8`; it always returns `1`. The
[native test](../tests/test_queue_update_gate.c) covers all flag values, call
order, pointer forwarding, and a flag mutation during the getter.

[func_80054A6C](../src/game/func_80054A6C.c) now matches all 92 L0 bytes from
plain C. It first calls the basis-copy helper, then copies the three translation
words, negating Y with 32-bit wrapping. Its translation reads occur after the
helper call, and it leaves the destination padding untouched.

[func_8003ABE8](../src/game/func_8003ABE8.c) matches all 120 L0 bytes from C
with register constraints and empty ordering barriers. When `$gp+0x36C` is
nonzero, it looks up actor zero and, if present, captures its component transform,
samples its translation, reads and clears the `$gp+0x378` word, subtracts it
from Y with 32-bit wrapping, and applies the adjusted translation. It
does not recheck the gate or replace the captured transform after callbacks;
skipped paths leave the offset unchanged. The
[queue-transform test](../tests/test_queue_transform.c) covers all gate bytes,
missing actors, wraparound, callback mutations, and the integrated call order.
The [real-helper integration test](../tests/test_queue_transform_real.c) runs the
queue adjustment with the actual C translation reader and writer across 6,400
gate/Y/offset combinations, including `INT32_MIN`. It also tests their
null, matching-basis, and different-basis branches; the transformation backends
are stubbed for the different-basis cases.
An [original-MIPS test](../tests/test_original_controller_route.py) also exercises
the real actor lookup and translation helpers together, including signed
boundary values and offset wraparound; it does not emulate controller hardware.

[func_80032C8C](../src/game/func_80032C8C.c) is now a readable-C exact match
for all 240 L0 bytes. It returns immediately when the runtime gate is zero;
otherwise it prepares and retrieves a component, conditionally enables it for
byte-mode zero when its flag is set, always clears the associated runtime
state, applies the component, conditionally finalizes it for byte-mode zero
when its flag is set, then always finishes.
Its [native test](../tests/test_queue_runtime_gate.c) covers every gate byte,
byte narrowing of the mode argument, forwarded pointers, and callback order.

The event processor [func_8001B1E4](../src/game/func_8001B1E4.c) now matches all
884 L0 bytes from readable C, using small inline MIPS load sequences and
delay-slot barriers without transcribing the control flow. It updates the queue
counters, snapshots pending records, routes static, mode-specific, and heading
handlers, and stops scanning when the live snapshot count reaches zero. Its
[native test](../tests/test_queue_processor.c) covers those dispatch paths,
heading parity, the empty-count exit, generated-event handling, and tail calls.

The optional [original-MIPS integration tests](../tests/test_original_controller_route.py)
also process the event queue twice with a synthetic actor, including the
original queue-update gate, callback, heading handler, grounded handler, turn
helper, movement consumer, rotation wrapper, rotation generator, composition
routine, conversion routine, and cross product. For first-port bits 13 and 15,
they observe respective turn inputs of `+11` and `-11` in mapping row zero and
verify that the pending vector is cleared. Starting from an identity basis,
the final nine halfwords are `(4103, 0, +/-69, 0, 4096, 0, -/+69, 0, 4103)`;
the wrapper preserves the remaining component bytes.

The fixture exercises the queue-update gate's disabled branch and explicitly
supplies the queued move request; separate original-MIPS tests check the
enabled branch and its original pointer getter with two downstream calls
stubbed. Other engine/BIOS and sequence functions are also stubbed. Unicorn
executes the original rotation software, but its GTE transfers are intercepted
by a narrow software model:
the `MVMVA` rotation command used by `func_8007699C` and the leading-sign
count registers used by the original square-root backend. The model follows
the [GTE register and saturation rules](https://psx-spx.consoledev.net/geometrytransformationenginegte/).
Tests cover sign counts, signed shifts and saturation, and the original
composition routine with noncommuting, extreme, and in-place matrices. This
does not validate physical controller hardware, GTE flags or timing, other GTE
commands, or an unstubbed game. The matched dot and cross products now express
their arithmetic in C, but their target path still uses a small three-instruction
MIPS HI/LO multiply intrinsic with register constraints and barriers; they are
assembly-assisted, not pure C. The scalar multiply also retains its target-only
HI/LO instructions, while its C shift and addition use defined unsigned
arithmetic. Its portable host path is tested against the original's low 32
result bits. The [native vector tests](../tests/test_fixed_vectors.c) cover
10,049 scalar and 80,000 signed-boundary, random, and overlapping-buffer cases
for the host dot and cross models. [Original-MIPS tests](../tests/test_original_controller_route.py)
also compare both routines against a fixed-point reference, including overlap.

The 216-byte `func_800548A0` snapshots the complete 32-byte basis, wraps the
three input angles to halfwords (negating the first and third), builds a
rotation, calls the GTE composition backend `func_8007699C` with the saved
basis first, converts the result via `func_800542A4`, and copies only the
nine basis halfwords back. It leaves the live component's padding and
translation untouched. The
[rotation harness](../tests/test_grounded_rotation.c) checks both rotation
wrappers across 686 angle combinations, callback order, snapshot timing,
null input, and preservation of padding/translation.

The paths above depend on configuration, queue, actor, and state gates. The
input callback `func_80023E94`, GTE matrix composition backend, and portions
of the final orientation/rendering path are not yet readable matched C. This
is not a complete unmodified controller-to-heading execution proof.

The separate state-ID switch appears in the mapped `func_80036270` range. Its
dispatch instruction is at `0x800363E4`; it subtracts `2` from the state
halfword and bounds-checks the resulting index against `49` before loading a
target from the table at `0x80012B00`. The C data symbol
`player_actor_state_targets` reproduces that 49-pointer, 196-byte table exactly
and is independently byte-checked in the progress build. Most entries share a default handler.
The code range has no direct JAL callers and its containing function ownership
is still under review, so I’m not attributing the switch to the preceding
`shared_actor_state_dispatch` helper. State `22` calls
`bubsy_update_actor_state` in mode `0` when actor `+0x10` is zero; state `27`
calls `bubsy_handle_death_state`.

State `2` calls a separate `shared_actor_state_2_handler` at `0x8003697C`,
which returns at `0x80037214`. It has its own `0xE8`-byte frame and no other
direct callers were found. It reads player model-mode/configuration data and
calls shared actor/model helpers; its gameplay purpose is still unresolved.
The bounded 2,200-byte range is byte-matched from high-level C in
`src/player_model/shared_actor_state_2.c` (GCC 2.6.3 PSX, MASPSX
`--expand-div`, with runtime mode, player-ready, and the mode-one handle as
gp-relative small-data externs). One register pin and an empty asm barrier
keep `&g_actor_handles` in `s4`; gameplay meanings remain provisional.

State `23` reaches `shared_actor_state_23_handler` at `0x80037D14`, which
returns at `0x80038364` after the `jr` delay slot. It has one direct caller.
Its first path compares the supplied sequence value with `0x29A`, checks actor
`+0x10` mask `0x2000` and mode byte `0x80186463`, and can call `0x80038C0C`,
`0x800226C0`, and emit event `0x17`. Other paths inspect component bytes
`+0x2C`, `+0x00`, and `+0x0C` and mutate shared transition state. Its full
interaction meaning remains unresolved; it is mapped but not fully translated
or matched. The event-`0x17` path is gated by sequence `0x29A`, actor mask
`0x2000` clear, and mode byte other than `1`, `2`, or `4`; this condition is
translated and tested in `player_actor_state23_should_dispatch_event_17`.

The non-default jump-table destinations are:

| State IDs | Handler block | Verified behavior |
| --- | --- | --- |
| `2` | `0x8003653C` | Calls `shared_actor_state_2_handler`; handler semantics unresolved. |
| `18` | `0x8003680C` | Calls shared handler `0x8004D26C`. |
| `19` | `0x80036824` | Calls shared handler `0x8004D994`. |
| `22` | `0x8003677C` | Calls Bubsy updater in mode `0` when actor `+0x10` is zero. |
| `23` | `0x800367E0` | If actor `+0x10` mask `0x01000000` is clear, calls `shared_actor_state_23_handler`. |
| `27` | `0x80036768` | Calls the death-state routine. |
| `39` | `0x8003685C` | Calls shared handler `0x800347EC`. |
| `40` | `0x8003662C` | State-specific behavior unresolved. |
| `42` | `0x800366AC` | State-specific behavior unresolved. |
| `43` | `0x80036554` | State-specific behavior unresolved. |
| `46` | `0x80036750` | Calls shared handler `0x80039F34`. |
| `48` | `0x80036524` | State-specific behavior unresolved. |
| `49` | `0x8003640C` | State-specific behavior unresolved. |
| `50` | `0x800364A0` | State-specific behavior unresolved. |

All other IDs in `2..50` use the default destination `0x80036874`. The IDs
remain state-machine values only; their names and player-facing meanings have
not been recovered.

### Grounded/Swim Routing

The default-state block at `0x80036874` can route to `0x8004B35C` when actor
`+0x10` mask `0x100000` is clear, lock byte `0x80186479` and gate halfword
`0x80186454` are zero, and actor flag `+0x04` bit `0x100` is clear. If runtime
state byte `0x80186461` is `5`, it additionally requires descriptor state
`0x25`, or state `0x26` with descriptor word `+0x04` not equal to `0xFFFE`.
Other runtime-state values do not impose that descriptor-state restriction.
Runtime mode zero selects `0x8004B35C`; nonzero mode selects
`0x8004BFF8`. `bubsy_actor_state_route_default` models and tests these routing
conditions. Original-MIPS tests cover 96 state/auxiliary/mode combinations
and each actor, lock, and gate bit independently; native tests cover all 256
runtime-state bytes. The `0xFFFE` sentinel is zero-extended, not signed `-2`.

The grounded handler starts with a `0x68`-byte frame and contains a 33-entry
computed state switch at `0x8004B4D0`, backed by the table at `0x800139A8`.
Every table target lies inside the handler body. Automatic L0 discovery stops
at the indirect jump, but a manual root bounds the complete handler at
`0x8004B35C..0x8004BFF8`, and its complete readable C reconstruction is byte-exact. The
inspected stores update actor state/flags, a companion object field, and global
counters; no direct world-position or velocity write has been identified yet.
Treat this as the grounded actor-state route, not a proven locomotion integrator.
Before the table dispatch, descriptor lookup value `3` enables special gates
for states `14` and `15`. State `14` skips the switch when actor flags overlap
`0x1100`, or when actor state bit `0x1000` is set; in the latter case it first
sets descriptor halfword `+0x02` to `1` if that field was zero. State `15`
skips and clears descriptor halfword `+0x02` when actor state bit `0x1000` is
set. Lookup values `0`, `1`, and `2` instead apply the state-14-style gate to
state `6`; other state IDs continue to the switch. Any other lookup value,
including negative values, skips dispatch. This prelude gate is modeled and
tested.
The extracted host dispatch map preserves all 33 targets for IDs `6..38`;
out-of-range IDs take the shared epilogue at `0x8004BFD0`. The table is tested
against the original executable's entries. Several IDs alias the same block,
and IDs `18`, `19`, and `22..28` target the epilogue directly.
The handler-entry gate and each unique non-epilogue switch path have tested
host-side models in `bubsy_actor_update.c` and `test_player_model.c`. The
unified `bubsy_grounded_handler_build_model_plan` composes the prelude gate,
target table, direct state writes, and per-case action plans. It does not
execute the engine calls encoded by those action masks, so it is a unified
control-flow model, not the target C body. Opaque helper effects remain outside
the model. The separate `bubsy_grounded_state.c` executes all original branches,
helper calls, assertions, event lifecycle operations, and direct state writes.
Its native mock-engine harness checks call order, helper-driven mutations,
prelude gates, counter boundaries, and regressions against the corrected case
models.
An instruction-store audit found no direct actor/descriptor coordinate or
velocity writes in this handler. Its grounded-state dispatch is therefore
mapped more completely, but grounded locomotion itself is not established.
All 3,228 function bytes from `bubsy_grounded_state.c` match
`0x8004B35C..0x8004BFF8` with pinned GCC 2.6.3/MASPSX and LLD. Its profile uses
`-fno-cse-follow-jumps`, gp-relative counter/event globals, two argument register
pins, and an empty compiler barrier; no instruction transcription remains.
The compiler-generated 132-byte switch table is linked at `0x800139A8` and
independently compared with the fingerprinted executable. The superseded
assembly source has been removed. This completes the bounded handler's
readable-C reconstruction, but does not establish a position/velocity integrator
or recover the gameplay semantics of every called helper.

Switch case `38` targets `0x8004B6C0`. Its descriptor byte `+0x11` changes from
`2` to `4`, or from `1` to `0`; only the latter path calls `0x8002262C` with
argument `0x11`. All three outcomes then call `0x80039158` with the actor and
third argument. The byte transition and conditional argument are modeled and
tested by `bubsy_grounded_case38_update_descriptor`; the state's full gameplay
meaning remains unknown. The
[interaction helper](../src/player_model/bubsy_grounded_interaction.c) matches
all 1,168 bytes with pinned GCC 2.6.3/MASPSX and LLD; register constraints and
empty barriers preserve the original scheduling without instruction
transcription. Native tests and 772 deterministic original-versus-C MIPS
cases cover its observed branches and helper mutations. Its phase-two path uses
the first lookup's target for `func_80038C0C` and a second lookup's target for
the later heading and used-byte write; an already-used second target still
dispatches event `0x25`. If the second poll fails, the current phase is
re-read and processed, including changes made by the poll helper.

Case `37` at `0x8004B66C` does nothing further when actor flags `+0x04` overlap
`0x1100`. Otherwise descriptor byte `+0x11` value `0` changes to `1` and calls
`0x80037B30(0)`; value `5` skips the remaining work; other values call
`0x80039158(actor, third_argument)`. Cases `29` and `30` directly set
descriptor byte `+0x10` to `1` and `0`, respectively. These observed branch
decisions and writes are modeled and tested. The exact 484-byte C reconstruction
of `func_80037B30` gates on runtime mode, a shared counter, actor state, and
actor flags; its eligible branch reads both component transforms, calls
shared actor-state processing, updates actor and descriptor fields, adjusts
the counter, and requests transition `(0x17, 5)`. The instruction flow is
exact, but its helper semantics and gameplay meaning remain provisional. A
readable C implementation is in `src/game/func_80037B30.c`; it preserves
the observed offsets, calls, and live global-pointer reloads. Its native tests
also cover replacement of the selected object during a helper call.
Case `13` sets actor byte `+0x01` to `2`. Cases `34` and `36` set that same
actor byte to `2` and clear descriptor halfword `+0x00`. These direct writes
are also modeled and covered by host tests.

Case `21` (`0x8004B578`) computes a dot product of two normalized vectors. It
then clears descriptor halfword `+0x02` on every path. The path calling
`0x8002E7D8()` and `0x80022650(actor, 8)` is selected only when actor state
`+0x10` bit `0x1000` is set, actor flags `+0x04` bit `0x40` is clear, the
companion's `+0x28` word is zero or the companion pointer is null, the signed
dot result is less than `0xCCC`, and descriptor state `+0x02` is not `5`.
`func_8002E7D8` clears the global byte at `$gp+0x2C4`; its adjacent helper
`func_8002E7FC` sets that byte to one. Both now match from plain C in
`src/game/func_8002E7D8.c`, with `g_grounded_case_event` mapped to `0x800BE020`.
The profile keeps the original leaf-function frame pointers and declares
the byte as gp-relative small data; the source needs no inline assembly.
Otherwise case 21 calls
`0x8002262C(actor, 0xC)`. The predicate and descriptor write are modeled and
tested. The threshold path also reaches `0x800347EC(actor, third_argument, 1)`;
that helper's effects remain unresolved.

Case `12` (`0x8004B70C`) proceeds only when actor flags `+0x04` have a nonzero
overlap with `0x818`, no overlap with `0x1104`, and actor state `+0x10` has no
overlap with `0xA0084`. Only when actor state bit `0x1000` is set does it call
`0x8002262C(actor, 0xC)` and then write `5` to descriptor halfword `+0x02`; if actor
byte `+0x01` is not already `1`, it sets it to `1` and calls
`0x800347EC(actor, third_argument, 1)`. These gates, writes, and call requests
are modeled and tested; helper semantics remain unknown.

Case `17` (`0x8004B7F8`) clears descriptor byte `+0x14`, sets actor byte
`+0x01` to `2`, and calls `0x8001C700(4, 3)`. If actor state `+0x10` overlaps
`0x44`, it also clears actor byte `+0x00`, calls `0x80022650(actor, 2)`, and
then calls `0x800347EC(actor, third_argument, 1)`. These writes and call gates
are represented by the tested action model. The 60-byte C implementation of
`func_8001C700` indexes the signed halfword at `0x80182F64` with the signed
first argument, passes that value and the narrowed second argument to
`func_8005E7B8`, and returns one when that helper returns zero. The 88-byte
C wrapper narrows both inputs to signed halfwords, calls `func_8005D5A8`, and
preserves its nonzero result; when it returns zero, the wrapper calls
`func_8008E1DC` and returns zero. `func_8005D5A8` bypasses its table lookup with
result `0x1B` when `$gp+0x624` is zero; otherwise it scans up to the signed
count at `$gp+0x62C` in the key/limit table at `0x801CCE14`, using a 72-byte
stride. It also checks the entry at the stopping index when the count is
exhausted or nonpositive. An unmatched key returns `1`; a match returns zero
only when the signed value is nonnegative and below the signed row limit.
The [selector](../src/game/func_8005D5A8.c) matches all 204 original bytes
using target register constraints and empty barriers without instruction
transcriptions. Its native and original-MIPS tests cover all flag values,
signed boundaries, duplicate keys, and stopping-index entries.
`func_8008E1DC` sign-extends both inputs to halfwords and delegates to
`func_8008E044`; its current C source also remains non-exact. That initializer
selects a 172-byte state row from the character table at `0x801D7D44`. It
clears flag bits `1`, `2`, and `8`, then sets bit `4`, rereading the character
table pointer for each update. It captures the first row for the later reset,
calls `func_80094108`, resets and copies that row's state fields, writes indices
`0..15` to its 16 per-entry slots, and returns `0x7F`. The notifier scans a
byte-counted table of 52-byte records at `0x801CFE80`, comparing signed
halfword keys at record offset `0x0E` with the packed selection. For every
match it stores the index at `0x801D5F26`, calls `func_80091194(0)`, and
rereads the count before continuing. That backend ignores its argument and
clears the selected record's fields at offsets `0x1B`, `0x04`, and `0x00`.
It sets the indexed bit in the low or high halfword at `0x801D8AD8` or
`0x801D8AE0`, then clears every bit present in those resulting halfwords from
the corresponding halfwords at `0x801D043C` and `0x801D0444`. For indices
above 15, the original variable shift wraps its count modulo 32 before the
result is stored as a halfword. The notifier remains non-exact. The
[backend](../src/game/func_80091194.c) now matches all 220 bytes using readable
C with four target-specific MIPS instructions, register constraints, and the
per-source ASPSX 2.21 expansion profile. The profile is a verified reproducer;
it does not assert that version 2.21 was the original assembler.
Original-MIPS tests cover callback mutation, changes to the character-table
pointer between flag updates, all 256 byte-sized backend indices, overlapping
record/global addresses, and the real initializer-to-backend call chain,
including 255 matching notifications. A separate
[native alias test](../tests/test_sequence_backend_alias.c) preserves the
original overlapping addresses at index `691`.
These observations do not change case 17's return convention:
`func_8005E7B8` returns zero after invoking the initializer.
Case `16` (`0x8004B77C`) exits when descriptor byte `+0x14` is zero and actor
flags `+0x04` bit `0x02` is set. Otherwise it writes `1` to the descriptor byte,
including when its previous value was another nonzero byte. It then requests
`0x800347EC(actor, third_argument, 0)` only when actor flags do not overlap
`0x1104`, actor state `+0x10` does not overlap `0xA9180`, and actor byte `+0x01`
is not `1`. The gate and initialization are modeled and tested.
Case `14` (`0x8004B838`) enters its sequence-selection path when actor state
`+0x10` has no overlap with `0x010000C4`, actor flags `+0x04` overlap `0x818`
but not `0x1104`, and descriptor byte `+0x11` is not `2`. That path clears
actor state `+0x10` and linked-object word `+0x30`, then selects sequence
`0x2BE`, asserts on a nonzero selection result, and calls `0x80052E18`.
Eligibility and direct writes are modeled and tested; callback effects remain
unknown.
Case `31` (`0x8004BA34`) first rejects actor flag mask `0x1104`, actor-state
mask `0xA0080`, or nonzero descriptor byte `+0x4C`. Actor flag `0x40` or
actor-state mask `0x1044` selects a separate path. Otherwise it increments
descriptor byte `+0x4D` modulo 256; the old value selects the below-11 branch
or the sequence-key path. On the alternate path, actor state bit `0x1000`
sets descriptor halfword `+0x02` to `2` only when that field is zero; otherwise
state mask `0x44` calls `0x800358C8(actor, third_argument, 1)` when bit
`0x10000` and actor flag `0x40` are clear. On the sequence-key path, a
nonzero key additionally selects index `0` and calls `0x80052E18`; both zero
and nonzero keys call `0x80022650(actor, 0x17)`, set linked word `+0x30`
to `-0x11`, and call `0x80022944(actor)`. Counter gates and followups are
modeled and tested.
Cases `8` and `10` share `0x8004BC48`. They return early for actor flag bit
`0x04`, actor-state mask `0x82000`, or actor-flag mask `0x1100`. With actor
state bit `0x1000` set, descriptor halfword `+0x02` changes from zero to `3`
for case 8 or `4` for case 10 unless actor byte `+0x01` is `1`. Otherwise
state mask `0x8104 == 4` dispatches shared helper `0x800358C8` in mode `3`
(case 8) or `2` (case 10); bit `0x80` exits; remaining inputs continue into
the counter path. That path exits when descriptor byte `+0x11` is `5`; otherwise
it calls `0x8004FFEC` when `$gp+0x540` is nonzero, clears `$gp+0x540`, resets
`$gp+0x53C` and writes the current case ID to `$gp+0x544` when that ID changes,
then increments the signed halfword counter and caps values at `0x28`. It
finally calls `0x80034EF8`. The selector and counter path are modeled and tested.
Case `32` (`0x8004BB68`) uses a restricted path when actor state `+0x10` has
bit `0x20000` or actor flags `+0x04` have bit `0x40`: it clears descriptor
halfword `+0x02` only when `(actor_state & 0x21000) == 0x1000`, then returns.
On the unrestricted path, state bit `0x1000` clears the same halfword.
Otherwise state mask `0x44` calls
`0x8002262C(actor, 0x17)` and sets descriptor byte
`+0x68` to `1`. With those gates clear, descriptor counter `+0x4D` below `11`
and actor-state mask `0x01002000` clear selects calls
`0x80022650(actor, 0x10)` and `0x800347EC(actor, third_argument, 1)`, clears
linked word `+0x30`, sets actor byte `+0x01` to `2`, and clears the counter.
Otherwise it calls `0x8002262C(actor, 0x17)` and `0x8003539C(actor, 1)`, then
clears the counter. Descriptor byte `+0x4C` is preserved on every path.
This whole switch block is modeled and tested; helper
semantics remain unknown.
Case `7` (`0x8004BE60`) sets actor byte `+0x01` to `2` and descriptor
halfword `+0x02` to zero when actor state bit `0x1000` is set. Otherwise it
calls `0x8002269C(actor, 0x0F)`, clears descriptor byte `+0x4C`, and sets
descriptor byte `+0x68` to `1`.
Cases `9` and `11` share the remainder of that block. State bit `0x1000` takes
the same actor-byte `+0x01 = 2` and descriptor-halfword `+0x02 = 0` path.
Otherwise sequence key `0x2BB` for case 9 or
`0x2B8` for case 11 selects index `0x2B5` and calls `0x80052E18`. The path
then clears actor state bits `0x100` and `0x400`. If the prior state had no
overlap with `0x220`, or global halfword `$gp+0x53C` is zero, it also clears
actor word `+0x14`, clears state bits `0x20` and `0x200`, and calls
`0x8003539C(actor, 1)`. Otherwise it calls `0x80034EF8`; it first calls
`0x8004FF50` only when `$gp+0x540` is zero. These branches are modeled and
tested. `func_8004FF50` rejects null actor/value inputs with result `1`, sets
`$gp+0x860` busy, and allocates a 16-byte record through `func_80082EEC`.
Null or `-1` allocation clears busy and returns `3`; success stores actor,
value, value, and old list head, installs the new head at `$gp+0x85C`, writes
the output pointer, clears busy, and returns zero. All 156 bytes now match
plain C in `src/game/func_8004FF50.c`.
The adjacent `func_8004FFEC` searches that list, unlinks the matching record,
frees it through `func_80082EFC`, and returns zero; null or not-found records
return one after clearing busy where applicable. All 164 bytes now match
plain C in `src/game/func_8004FFEC.c`. Native tests cover allocation failure,
head/middle/tail removal, missing records, and helper mutations.
The [event callback](../src/player_model/bubsy_grounded_event_callback.c)
at `0x8004B2A4` subtracts two from the counter with halfword wrapping, clamps
negative results to zero, and otherwise returns while the counter is nonzero.
At zero it selects the first level-record actor when the signed record count is
positive, or calls `func_80040FC0(0)` otherwise. It releases the event even
when the pointer is null, clears the event pointer and actor word `+0x14`,
clears actor-state bits `0x200` and `0x20`, and calls `func_8003539C(actor, 1)`.
The actor is captured before event release; mutations during either helper are
preserved in the original order. All 184 bytes match plain C without register
constraints or assembly. Native tests cover all 65,536 counter inputs, and
original-MIPS tests cover signed count boundaries, event-pointer and actor
mutations, and a helper changing the counter after the zero check.
The complete 1,188-byte `func_80034EF8` now matches readable
[C](../src/player_model/bubsy_grounded_turn.c), with register constraints and
empty compiler barriers but no instruction transcription. It samples the
direction and runtime mode, performs the observed state/sequence calls, and
writes the signed turn to linked offset `+0x4C` when nonzero. Its native
tests and 768 deterministic original-versus-C MIPS cases cover the direction,
counter, mode, state-bit, and helper-mutation paths; gameplay labels remain
provisional. The called assertion dispatcher `func_800556D0` and transition
dispatcher `func_8001B558` are also readable C byte matches of 176 and 864
bytes, respectively.
Case `15` (`0x8004B8D8`) reads the current sequence key. Key `0x2BE` selects
index `0x2C3`, calls `0x80052E18`, and submits the move request with arguments
`(0xF, 4, -2, actor+0x0C)`. For other keys, a true result from
`0x8005290C` calls `0x8002262C(actor, 7)` and `0x8003539C(actor, 1)`; a false
result submits the same move request. This branch plan is modeled and tested;
the request's gameplay purpose is not established.
Case `6` (`0x8004B990`) proceeds only when actor flags `+0x04` do not overlap
`0x1144` and actor state `+0x10` does not overlap `0x82000`. It sets descriptor
byte `+0x4C` to `1`; actor-state mask `0x44` selects
`0x800358C8(actor, third_argument, 0)`, otherwise a clear mask `0x01020080`
selects `0x80035ACC(actor, third_argument)`. The gate and action selection are
modeled and tested; both shared callees are byte-matched readable C. Their
lower-level engine effects are not all recovered.
Case `20` (`0x8004B4D8`) returns early if descriptor byte `+0x0D` or actor
state `+0x10` is nonzero, or actor flags `+0x04` overlap `0x1100`. Otherwise
it calls `0x800516F8`, `0x80054AF0`, and `0x80022650(actor, 0xC)`. It selects
sequence index `0x349` only when actor flag `0x80` is clear and the current
sequence key is not already `0x349`. The gates and action plan are modeled and
tested; the helper purposes are unknown.
Cases `33` and `35` share `0x8004BDA4`. The successful path requires actor
flags `+0x04` to have neither bit `0x04` nor any overlap with `0x1140`, actor
state `+0x10` to equal zero, and actor byte `+0x01` not to equal `1` (the
additional `0x01080000` mask is also clear). It writes descriptor halfword
`+0x00` to `3` for case 33 or `2` for case 35 and clears linked word `+0x30`,
then requests calls `0x80022650(actor, 0xF)`, `0x80022650(actor, 8)`, and
`0x800347EC(actor, third_argument, 1)`. The gate and direct writes are tested.

The grounded handler at `0x8004B35C` calls `func_800100A0` while combining two
three-component Q20.12 vectors. Its C implementation computes the signed dot
product using a target-only HI/LO multiply intrinsic and is byte-exact for the
single L0 copy. The actual persistent transform write is in the generic move-request
consumer `game_apply_queued_move_request` at `0x80024F2C`: it composes pending
moveInfo vectors and applies the resulting translation through
`func_80054C7C` to the object's component transform, then archives and clears the
pending vectors. Bubsy-side actor callbacks enqueue their live object pointers
through `game_enqueue_move_request`; this confirms an actor component-transform
translation update, but does not establish world-space coordinates or a
separate velocity field. The 160-byte routine at `0x800250CC` gates queue
draining when the component sequence key returned by `func_80052ABC` is `0x31F`.
That path skips applying entries but still clears the queue count. For other
keys, it repeatedly applies queued object pointers through `func_80024F2C` and
rereads the signed count after each application, so entries appended during
draining are included in that pass; it clears the count after the loop.
All 160 bytes match plain C in `src/game/func_800250CC.c`, using a signed
halfword loop index and a live count reload across each helper call.

The consumer's pending-vector conversion `func_800536F4` is now a 128-byte
C match. Each component takes an arithmetic right shift by 12; negative
components first undergo wrapping negation and are negated again after the
shift. This truncates ordinary negative inputs toward zero, but `INT32_MIN`
produces positive `524288` through the observed wrapping sequence. Native
tests cover that exception, fractional boundaries, in-place conversion, and
the three-word output extent.

The grounded handler also directly calls the 36-byte wrappers
`func_8002262C` and `func_80022650`, now exact C matches. Both pass actor
offset `+0x10` and an eight-bit selector to `func_800566E8` and
`func_8005668C`, respectively, preserving the helper return value. Native
tests now exercise these wrappers against the real downstream helpers rather
than forwarding mocks.

Those helpers are exact C matches too: the 20-byte `func_80056678` queries
one selected bit, the 92-byte `func_8005668C` sets it when absent, and the
92-byte `func_800566E8` clears it when present. The mutators conditionally
XOR the mask, leaving every other bit unchanged. Consequently,
`func_80022650(actor, selector)` sets a bit in actor word `+0x10`, while
`func_8002262C(actor, selector)` clears it. Individual bit gameplay names
remain provisional.

The observed return is not a uniform success code: a set no-op returns `1`,
a clear no-op returns `0`, and a changed path returns the updated word's
register bits. Selectors are reduced to their low five bits by MIPS variable
shifts; the host C paths mask them explicitly, while the matching target
profile relies on that hardware behavior. Native tests cover all 256 byte
selectors over mixed bit patterns, repeated calls, large query selectors,
returned register bits, and the wrappers' actor-word offset.

The grounded-handler prelude also calls `func_8002269C`, a byte-exact C
wrapper that clears the selected bit in actor word `+0x04`. The 20-byte
`func_8003AA1C` helper stores pointer `0x80186450` through its output argument
and returns `1`; its downstream purpose is not established. Native tests cover
the extra actor-word offset and exact pointer value.

Two more grounded-handler helpers read small-data slots through `$gp`.
`func_8001DCE0` returns the word at `$gp + 0x170`; `func_800516F8` copies the
word at `$gp + 0x888` to its output pointer and returns zero. L0 initializes
`$gp` to `0x800BDD5C`, so the observed slots are `0x800BDECC` and
`0x800BE5E4`, respectively. The host tests model these slots as variables;
their gameplay meanings remain unknown.

The `vector_range_scale_q12` wrapper also calls `vector_range_condition_q12`, a
vector range-conditioning helper that selects a shift from the largest component magnitude. Its C implementation
and threshold tests are in `player_vector_range.c`; its 312-byte range now
matches exactly under the pinned GCC profile.

The grounded handler calls `vector_range_scale_q12`, which range-conditions a 3-vector
with `vector_range_condition_q12`, obtains one scalar from `vector_length_q12`, and scales each
conditioned component by that scalar through `func_80053350`. The bounded
`vector_length_q12` squares each nonzero Q20.12 component with
`func_8001007C`, a 36-byte helper now byte-matched from C with narrow inline
HI/LO extraction and C-side recombination. It adds the three results and passes the sum to the
GTE-backed `func_8005359C` conversion. This is consistent with a fixed-point
vector-length calculation, but its output's gameplay role is unknown. The C
models and host tests are in `player_vector_transform.c` and
`test_player_model.c`. Both functions match their bounded L0 bytes; the wrapper
uses target-specific return-register and zero-store constraints. This
processing has not been shown to update Bubsy's world position or velocity.
The subsequent `func_80053A08` wrapper applies `func_8001007C` to each of three
components using the same scale and adjacent output slots, then returns zero.
Its 104-byte range now matches from plain C in `src/game/func_80053A08.c`,
with natural GCC register allocation and no inline assembly or register pins.
The calls read and write components sequentially, preserving the original
behavior for in-place and partially overlapping buffers. Native tests cover
zero, negative, and fractional scales, call count, and output bounds.

The same update calls `0x8005290C` with the actor's component pointer and an
output byte. Its bounded code reads actor offsets `0x04`, `0x05`, `0x07`, `0x08`,
and `0x0E`, plus sequence data through a pointer at `0x18`. When flag bit `0x04`
is set, it steps a cursor forward or backward according to byte `0x07`; table
values `-4` through `-1` take a threshold path using signed division by 1,000.
Other table values produce false. When the actor flag is clear, the result is
always true, without a threshold comparison. The next cursor is computed in
32 bits, without truncation back to a signed halfword.
The C translation `player_actor_sequence_boundary_reached` has host tests for
these branches, but the underlying concept may be animation timing, movement
sequencing, or another actor-sequence protocol; that meaning is not established.
Two additional called helpers make the sequence data handling clearer: the
140-byte helper at `0x800529F8` returns `entries[cursor] % 1000` unless sequence
data flag `+0x2C` bit `0x04` is set, in which case it returns the signed cursor
itself. The 28-byte helper at `0x80052A84`
returns `cursor_08 - previous_cursor_0C`. C versions are covered by signed,
flagged-cursor, and cursor-delta tests. All three are now matching C in the exact allowlist. Their
higher-level animation meaning is still uncertain.

The 24-byte helper at `0x80052ABC` has 31 direct callers, including the Bubsy
actor update. It writes the sign-extended halfword at actor offset `0x0C` minus
2 through its output pointer. The C translation has signed-boundary tests and
matches in C under the pinned GCC profile and is credited in the validated report.

The 136-byte helper at `0x80052AEC` is called by the actor update with sequence
index `0x309`. It compares the signed index against the signed table length at
sequence-data offset `0x0C`. An index at or past the length returns `0x11`; a
nonnegative table entry returns `0x12`; a negative entry selects a state by
setting actor `+0x05` to the entry's low byte, both cursor fields `+0x08/+0x0C`
to `index + 2`, field `+0x0A` to the next table entry's low halfword, and field
`+0x0E` to `1`, then returns `0`. The caller asserts on either nonzero result
using `../f/bubsy.c:0x41A`. The host C translation tests all three outcomes; the
target C implementation preserves the original repeated pointer loads and store
order and matches all 136 bytes exactly without MIPS opcode strings.

## Actor Event Dispatch

The actor update calls `0x80037214` with mode `0` and the actor pointer. The
helper reads the current event through the actor component and consults a
runtime mode plus L0 state byte `0x80186463`. Verified branch outcomes include:

- Mode `0`, runtime mode `0`, event `0x3E0`, and state byte not `3`: select `0x35`, set actor flag `0x04`, and dispatch with flags `0`.
- Mode `0`, runtime mode `1`, and event `0x157`: select `0x54`, set actor flag `0x04`, and dispatch with flags `0`.
- Other mode-`0` events: no selection, flag change, or dispatch; return `0x157`.
- Mode `1`: select `0x3E0` for runtime mode `0`, else `0x157`, set actor flag `0x04`, and dispatch with flag `0x40000000`.
- Other modes: no dispatch; return `1`.

The portable C model in `bubsy_actor_events.c` keeps event/property IDs and
engine operations explicit through callbacks. The target C version uses the
original two-argument ABI and now matches all 316 original bytes. Its register
constraints and empty barriers emit no instructions themselves; the superseded
assembly transcription has been removed. Gameplay
meanings and engine callback implementations are still unknown.

## Death-State Routine

`0x8003737C..0x80037AFC` is rooted as `bubsy_handle_death_state`. The debug
assertion at `0x8003767C` names `gBubsyInfo.deathType >= 0` and reports
`../f/bubsy.c`, line `0x912`. The routine initializes a large temporary state
frame, reads and normalizes the death-type global at `0x80186458`, dispatches
through numeric state values, calls engine animation/event helpers, and returns
through the matching `0x158`-byte stack-frame epilogue at `0x80037AF4`.
At entry, event `0x1B` with property `4` is dispatched when `$gp+0x3A4` is
`-2` and `func_800222E0()` returns a zero low byte. Level `0x13` then takes a
separate cleanup-and-return branch. These gates are translated and tested; the
cleanup and event helpers themselves remain unidentified. Otherwise, nonzero
byte `0x80186462` skips the main body but still reaches the final
`func_8001A670` call at `0x80037AC8` before the epilogue. The main-body predicate
is translated in `bubsy_death_state_should_run_main_loop`.
The following table-driven block is reached only when `$gp+0x3A4` is `-2` or
`-1`; this range check is translated in
`bubsy_death_state_uses_counter_entry_list`. The counter's meaning remains
unknown. At the block's end, `-2` becomes `-1`; otherwise a helper low byte of
`1` changes the counter to `0`, and other values are preserved. This transition
is translated and tested in `bubsy_death_state_next_global_counter`. Within the
table path, event `0x1B` with property `5` is dispatched only when the list is
nonempty and its selected entry has byte `+0x20` clear; this branch is modeled
by `bubsy_death_state_should_dispatch_counter_entry_event`. These predicates
remain narrow, tested semantic models. The report-only handler candidate now
includes the observed table path, its conditional property-5 event, helper-call
sequence, counter updates, and property-0 return event. The engine helpers'
broader behavior is still unidentified.

The observed state values and timing arithmetic are not yet mapped to named
death animations or gameplay rules. The routine is in the code treemap, but its
source behavior still needs instruction-by-instruction reconstruction.

The scalar state progression observed inside this routine is now translated in
`bubsy_death_state.c`, without claiming the whole handler. A negative value
calls the assertion callback with condition `gBubsyInfo.deathType >= 0`, source
`../f/bubsy.c`, line `0x912`, then is set to `1` by
`bubsy_death_state_normalize_initial`. For state `1`, the result
of `func_80082ECC() % 3` keeps state `1` at remainder `0`, sets `7` at remainder
`1`, and sets `9` at remainder `2`. Other remainders initially preserve state
`1`; all these paths then reach the final parity gate. State `2` initially
stays `2` unless the counter remainder is `1`, when it becomes `10`. State `4`
maps to `6` for level IDs `5`, `7`, `9`, or `18`, and to `11` for IDs `4`, `6`,
or `8`; other level IDs preserve state `4`. The final gate at `0x800377D0`
uses `beq`, so state
`11` skips the parity transition and remains `11`. Other states reaching this
gate become `12` on even parity or `13` on odd parity on level IDs `4`, `6`,
or `8`. These are observed scalar transitions,
not recovered animation names. The standalone C helper is tested but not
byte-matched. The target-only `bubsy_handle_death_state` implementation models the verified entry
gates, counter-list path, death-type scalar progression, resource/event setup,
runtime reset, main helper sequence, and terminal writes. Several animation
and engine-helper semantics remain unresolved. The GCC 2.6.3/MASPSX probe now
produces all 480 original instructions. Independently linking it with LLD at
`0x8003737C` and resolving helper symbols to their observed addresses gives
1,920 identical bytes, with no differences. Objdiff also reports 100% for a
relocation-resolved verification object derived from that linked compiler
output, not from the original executable. The matched function's SHA-256 is
`eb2e2bdbb080e0ffa3e8adf93e0843a710dd3ffab3a2d7fd684c8143cccd01d1`.
This build is now reproduced by `python -m tools.progress build` using the
tracked `tools/gcc_match.py` implementation and matching profile in
`config/base-matches.json`. It downloads checksum-pinned GCC/MASPSX inputs,
compiles the tracked source and headers on a temporary Linux filesystem, and
resolves all helper symbols with LLD. Only the linked compiler output is
packaged into the combined base object, after comparison with the hash-verified
original executable. Build and source hashes are recorded under
`build/base/l0/functions/matching/bubsy_handle_death_state/`.

This is an assembly-backed byte match, not fully recovered high-level C or a
complete game rebuild. The new validated snapshot reports this function at
100% and includes its 1,920 bytes in exact credit. Clang still builds the
getters and the data table; the completed GCC sources are excluded from that bundle.
This also removes unused death-model readonly helper tables that interfered
with objdiff's aggregate data-section matching. All 196 original table bytes
are still checked independently and credited by objdiff. The validator has not
been relaxed and no report measures are filled in manually.
After the main path begins, the handler clears actor fields `+0x04/+0x10` and
globals `0x80186460`, `0x80186461`, `0x80186452`, `0x80186479`, `0x8018649D`,
`0x801864B8`, and `0x801864C0`. It sets `0x80186463` from the returned object's
byte `+0x0C`; this reset block is translated and tested in
`bubsy_death_state_reset_runtime`.

The main path continues after this reset through several animation, event, and
actor-update helpers. Its terminal block at `0x80037A74..0x80037ACC` sets
`0x80186462` to `1`, clears actor byte `+0x2C`, sets `$gp+0x3A4` to `-2`, and
zeros the word at the actor component pointer `+0x20` plus nested-object words
at offsets `0x30`, `0x34`, `0x38`, `0x3C`, `0x40`, `0x78`, `0x7C`, `0x80`,
`0x98`, `0x9C`, and `0xA0`; it then clears `0x80186464` and calls
`func_8001A670`. These terminal effects are modeled and tested in
`bubsy_death_state_finalize_runtime`. The report candidate also emits the
observed main-path helper sequence and terminal call. These target instructions
are included in the verified local GCC match; the high-level meanings of the
engine helpers remain unresolved.

`func_80082ECC` is a BIOS-vector stub: it sets `$t2` to `0xA0`, jumps there,
and sets `$t1` to `0x2F` in the delay slot. The meaning of its returned value
is not yet established, so the C API accepts that value as input rather than
claiming it represents elapsed time.

Other `../f/bubsy.c` line-string references, including
`gBubsyInfo.deathType >= 0`, identify additional routines to investigate next.
They should not be translated by guessing from the assertion text alone.

## Reproduce

From the repository root:

```powershell
python -m tools.code_analysis --functions --write-map
python -m tools.progress build
python -m unittest discover -s tests -v
```

The rebuilt code map includes the manually evidenced roots from
[manual-code-roots.json](../config/manual-code-roots.json). The decomp.dev code
percentage counts only the two byte-verified getters until further source code
is matched.

## Movement Request Queue

The shared L0 routine at `0x80024EBC` is tagged by an assertion as
`../f/game.c:0xB01`. It checks the signed request count against 30, calls the
assertion helper on overflow, then increments the 16-bit count and stores the
request pointer in the array based at `0x80185A6C`. Its return value is the
request-slot byte offset (`old_count * 4`). Six direct callers exist in L0.

This queue is now represented by `game_enqueue_move_request` in
`src/game/move_requests.c`, with tests for count 29 and overflow at 30. It is
shared game infrastructure. Player actor callbacks enqueue their live object
pointers through this API.

The 160-byte routine at `0x800250CC` drains the request array after a sequence-
key query. When the key is `0x31F`, it skips applying entries but still clears
the count. Otherwise it applies each queued pointer through `0x80024F2C`,
checks the live count again, and clears the count after the loop. The complete
160 bytes now match plain C in `src/game/func_800250CC.c`; the halfword loop
index reproduces the original sign-extension and indexed addressing.
The enqueue primitive also matches all 112 bytes from readable C in
`src/game/func_80024EBC.c`, including assertion continuation and a count reload
after the assertion callback.

The six L0 call sites group into four caller candidates: `0x80015EA0` in the
`0x80015A54..0x80016270` frame is gated by actor word `+0x10` bit `0x04`;
`0x80025FC4` in `0x80025A98..0x80025FFC` is gated by pending-vector words
`+0x38/+0x3C/+0x40`; `0x8003D5D8` and `0x8003D718` are separate transition
paths in `0x8003CC78..0x8003D8D8`; `0x8004193C` and `0x80041B08` occur in
different sequence paths in `0x80041218..0x80041D80`. The three larger callers
contain indirect dispatches, and none of these call sites is in the current
reachable-code map, so their extents and state meanings remain provisional.

The first caller also invokes `func_80027E24`, which calls the actor-state flag
lookup `func_8004B264` at `0x80027EA0`. All 64 bytes of that lookup match plain C
in `src/player_model/bubsy_grounded_state_flag.c`. It resolves a context table
through `func_80026648`, then reads the actor's current byte at `+0x01` and
returns whether table byte `+0x40 + state` is zero. The state byte is read
after the resolver returns. The real resolver ignores both arguments and
returns the fixed address `0x801D5F70`; its 16 bytes now match plain C in
`src/game/func_80026648.c`. The meanings of the table and flag remain
unverified. Native tests cover all 65,536 state/flag combinations, and
original-MIPS tests cover all 256 state indices with zero and nonzero flags.
Both test suites also use a synthetic resolver that changes the state to verify
the read order; the real resolver itself does not mutate it.

The 228-byte `func_80027E24` list search matches readable C in
`src/game/func_80027E24.c`. It obtains the input's record ID from `+0x0C`,
looks up the associated list, skips entries equal to the current input ID, and
uses the first object whose byte `+0x00` is `0xF5` for the state-flag test. If
none qualifies, it calls the assertion helper with `EGSBoolFalse`,
`../f/game.c`, line `0x12CA`, and returns one if that helper returns. The
target build uses a fixed register and empty compiler barrier to forward the
object getter's live argument register; the real context getter ignores that
argument. The caller must supply a valid record ID and nonnull selected
objects: the original ignores a failed record lookup and dereferences the
selected object without a null check. Native and original-MIPS mutation tests
verify that changes to the list count or input record ID during object lookup
affect the remaining search, and that only the low result byte is tested.

The 80-byte `func_80028C84` record-to-list-ID lookup matches plain C; invalid
indices leave its output untouched. The following one-based list getter,
`func_80028CD4`, also matches all 100 bytes from plain C in
`src/game/func_80028CD4.c`, with no assembly, register pins, or barriers. It
returns null and writes a zero count for an invalid list ID; otherwise it
writes the signed halfword count and returns the list pointer. Its global list
pointer is loaded again after the count write. Original-MIPS tests exercise
that reload by making the output alias the pointer slot. The original scaled
the byte offset inside the second range test and then reused the offset
register as the entry address, so the source mirrors that shape.
The host integration test links the matched search and getters; the
original-MIPS integration executes the real unmodified list helper.

The full bounded control flow of `func_80025A98` is reconstructed in
`src/game/func_80025A98.c`, with target-facing layouts in `move_preparation.h`.
All 1,380 bytes at `0x80025A98..0x80025FFC` match the fingerprinted executable
under pinned GCC 2.6.3/MASPSX. The only special mapping is the global transform
pointer at `$gp+0x194` (`0x800BDEF0`); the source uses natural register allocation
and contains no inline assembly or register pins. The superseded assembly
transcription has been removed.

The separate host model `game_prepare_move_request` in
`src/game/move_requests.c` covers descriptor and
move-info guards, the control-table counter/index loop, component-vector updates,
transform gates, and the final conditional enqueue. Opaque engine calls are
injected through `GameMovePreparationOps.invoke`; this preserves call order and
data flow without claiming their internal effects are understood. The smaller
`game_enqueue_move_request_if_translation_pending` helper isolates and tests
the final zero/nonzero vector gate. This model is host-side and not byte-matched.

The preparer computes each target component at move-info `+0x08/+0x0C/+0x10`
from the fixed origin at `+0x58/+0x5C/+0x60` plus the current 16-byte table
payload, with 32-bit wrapping. It does not accumulate payloads into the prior
target; the older host model did, and has been corrected. When a zero-separation
payload is tagged `0x0A`, it toggles the payload counter and retries. A different
tag asserts at line `0xDAD`; if the assertion returns and the tag remains
different, it exits the loop rather than toggling.

Its transform path saves the component basis, composes it with the global
transform, and applies the supplied basis input. Flag mask `0x01` forces the
four-word transform-vector copy and basis restoration. Flag mask `0x04` alone does
so only when the vector's signed Y lies outside `[-3275, 3275]`; otherwise it
clears the first three saved transform words, preserving the fourth. Helper
failures return zero without notification or enqueue, but retain writes already
performed. The final `func_80022944` call runs before the live pending-vector
enqueue check, so its mutations affect whether the actor is queued.
The native `test_move_preparation.c` harness exercises table rewind/advance and
completion, fixed-origin retry behavior, wrapping arithmetic, transform bounds,
helper failures, assertion continuation, and post-notification enqueue decisions.

The enqueue guard at `0x80015EA0` is modeled by
`game_enqueue_move_request_if_actor_state_bit_2` in the same module: it reads
actor word `+0x10` and enqueues only when mask `0x04` is set. That helper covers
only the final predicate. The wider switch-state-12 plan below also models the
alternate bit-clear path, which calls `0x80052AEC`, `0x80052E18`, and
`0x80022C94(actor, 0x380, ...)`, then enqueues through a local success flag.

The enqueue-related control flow is modeled in `src/game/move_requests.c`.
`game_process_move_request_state_12` executes both sides of the grounded
state-12 bit-`0x04` gate, rereads state after `0x80022C94`, and performs
transition `(0x0C, 4)` after enqueue. Runners `game_run_move_request_path_3d718`,
`game_run_move_request_path_4193c`, and `game_run_move_request_path_41b08`
execute their enqueue paths, callback order, and post-enqueue transitions.
`game_run_move_request_path_3d5d8` models the positive-result gate, actor-byte
`+0x01 == 2`, inequality against the saved comparison byte, observed
actor/global stores, transitions, enqueue, and trailing helper calls. The
preceding positive-check calculation remains an input because its math/helper
behavior is unresolved. Native tests cover each modeled path.
This models the six enqueue paths, not the complete four enclosing state-machine
bodies; their indirect dispatches and other helper effects remain unresolved.

The queue consumer is the 416-byte routine `game_apply_queued_move_request` at
`0x80024F2C`. Its high-level C source in `src/game/func_80024F2C.c` byte-matches
the L0 routine with pinned GCC 2.6.3 PSX. It reads pending movement vectors
from the object's `moveInfo`, composes the translation, and calls
`func_80054B20` to read the current component translation, adds the queued
translation with `func_80010038`, and calls `func_80054C7C` to store the result;
it then archives and clears the pending values. This establishes a read-add-
write update of the component translation. Both accessors receive a null basis,
so this path updates the component's global-basis translation directly; the
pending vector at `+0x38` is added directly without a Q12-to-integer shift.
The separate auxiliary vector at `+0x98` does pass through the observed shift
conversion before joining the pending vector. No separate velocity field is
updated here.

Grounded case handling calls the 536-byte routine `func_80022944` at
`0x80022944`. Its C source in `src/game/func_80022944.c` byte-matches L0 with
the pinned GCC 2.6.3 PSX profile. It accumulates signed Q12 increments from
move-info `+0x34` into the integer per-update stride at `+0x30`, then applies
the observed actor-state and clamp gates. It scales configured Q12 direction
components by that stride into pending Q12 values at `+0x38/+0x3C/+0x40`.
When those direction fields are zero, it extracts and normalizes the component
transform's third basis vector, then accumulates the scaled result. The request
preparation path initializes `+0x30` to twice a table selector in its selector
mode. This grounds the stride-to-translation dataflow without identifying the
stride as physical velocity.

The matched source now contains no instruction snippets: the clamped-step
self-subtraction is expressed in C, with register constraints and empty
barriers retained for matching. Wrapping arithmetic is explicit, including
the original signed-minimum delta behavior. A native harness executes this
same implementation over 73,695 boundary combinations and callback mutations;
it also checks transformed pending-vector accumulation and the live Y gate.

### Grounded Stride Producer and Upstream Boundary

The complete 1,304-byte routine `func_80035ACC` at
`0x80035ACC..0x80035FE4` is now byte-matched readable C in
[bubsy_grounded_stride.c](../src/player_model/bubsy_grounded_stride.c).
Grounded case 6 calls it after the already recovered flag/state gates; the
other direct caller is at `0x8004C758`. It clears byte `0x8018647A`, obtains
the current component sequence, and updates move-info stride `+0x30` and
signed Q12 increment `+0x34` before calling `func_80022944` exactly once.

The runtime byte at `gp+0x7EC` (`0x800BE548`) is **not established pad input**.
The verified state-2 initializer calls `level_get_player_model_mode` at
`0x800369DC`, then stores its result at `0x800369EC`. That exact getter reads
`0x801D89F0`, the mode also used to choose ordinary versus `*SWIM.TZP`
resources. No controller-button interpretation is needed for this producer.

With that mode stable, its three main ranges are:

| Model mode | Starting stride | Sequence handling | Stride/increment action |
| --- | --- | --- | --- |
| `0` | below `17` | Select `19` if needed; restart and emit transition `26`, property `4` | Set stride `17`, increment `0x147`; set actor state bit `3`, clear bit `4` |
| `0` | `17..34` | Select `19` and restart if needed, without the transition | Add `0x147` to the increment only if component byte `+4` has bit `0x20` |
| `0` | at least `35` | Clear bit `3`, set bit `4`; select `36` if needed, restart and emit the transition | Clear increment and set stride `35` |
| `1` | below `12` | Select `0` and restart if needed, without the transition | Set stride `12`, increment `0x147`; clear actor state bit `4` |
| `1` | at least `12` | Clear bit `3`, set bit `4`; select `0` and restart if needed | Clear increment and set stride `12` |
| other | any | No range-specific sequence action | Leave stride and increment unchanged before the override |

The `0x147` value is accumulated in the signed Q12 increment, not directly
added to integer stride on every call. The downstream producer performs the
truncation-toward-zero conversion. Increment overflow wraps like the original
32-bit MIPS addition.

The final override byte at `0x80186460` aliases grounded descriptor `+0x10`
(the descriptor base returned by `func_8003AA1C` is `0x80186450`).
Grounded cases **29/30** write this byte to **1/0**, respectively:

- Override `1` forces stride `35` in model mode `0`, otherwise `26`.
- Override `0` applies upper caps `35`/`12` in model modes `0`/`1`.
- Other override values skip this final cap. These are upper caps, not
  lower clamps, and do not clear the increment.

All helper order and subsequent mode/component rereads are preserved.
Nonzero sequence-selection errors assert using the original source-line
constants, then continue with the sequence restart; they do not return early.
The transition passes value `14` and pointers to the three translation
coordinates. The original component flag is sampled after any sequence change.

The matching profile uses three pointer register constraints, one empty memory
barrier, and an independently sampled cap-mode read. There is no instruction
transcription. The dedicated
[native harness](../tests/test_bubsy_grounded_stride.c) checks 9,216 boundary
combinations plus helper mutations, including both final cap paths, unknown
modes, all override classes, signed increment wrap, saved move-info pointers,
and component replacement before the animation-flag read.

This establishes **state/animation -> stride/increment -> pending translation**,
not the complete controller -> heading path. The complementary deceleration
producer below completes another bounded stride path. Its gate has writers in
`func_800347EC`, including `0x80034CBC` and `0x80034E28`. Those state
transitions, the input callback, and remaining heading writers need readable
matches. The
shared dispatcher's temporary basis-rotation/stride path and its column
reconstruction are now recovered and native-tested, but the GTE backends are
not. The model-mode byte is not an
identified controller input. The already-matched `func_80054AF0` is a
direction extractor, not an angle decoder: it reads signed basis halfwords at
`+0x04`, `+0x0A`, and `+0x10`, negates the middle sample after sign extension,
and writes three 32-bit direction components. Thus an input Y halfword
`-32768` produces `+32768`, not a wrapped halfword. The preparation routine
copies this vector into move-info `+0x88` when its transform gates pass.
This proves a component-basis -> direction path, not the upstream
controller -> basis construction.

### Complementary Grounded Deceleration Producer

The complete 968-byte `func_80038844` (`0x80038844..0x80038C0C`) now
byte-matches from plain C in
[bubsy_grounded_deceleration.c](../src/player_model/bubsy_grounded_deceleration.c).
It is called at `0x8003619C` when byte `0x801864B8` is nonzero.
The standard pinned GCC 2.6.3 PSX/MASPSX profile suffices: natural register
allocation, no inline assembly, no register pins, and no compiler barriers.
Its analyst-confirmed root added these 968 previously unclassified bytes to the
discovered map, bringing the denominator to 3,040,136 bytes at that time. This is newly
mapped and matched code, not a reclassification of a previous exact match.
The first stopped-mode read is independent of the later mode-1 test, matching
the original two global samples.

For starting stride `+0x30 <= 0`, it clears both stride and increment `+0x34`,
then clears actor state bit `11`. It then:

- In model mode `0`, calls `func_8003539C(actor, 1)` only if state mask `0x44`
  is clear; this path does **not** clear actor state bit `6`.
- In model mode `1`, clears state bit `6` and calls that helper only if byte
  `0x80186464` is zero.
- In all cases clears active byte `0x801864B8` and returns without producing
  pending movement. The clear is after any reset helper mutation.

For positive starting stride, a nonnegative increment first captures state
bit `3` (`0x08`). In model mode `0`, if state bit `9` is clear and state mask
`0x18` is nonzero, it selects sequence `0x318` and restarts the component.
If state bit `4` (`0x10`) was set, this is preceded by reading the component's
`+0x14` transform translation and emitting transition `26`, property `4`,
with value `14` and pointers to its three coordinates. Sequence-selection
failure asserts at source line `0xBC5`, then still restarts.

For **every nonnegative increment**, even when that sequence-change gate is
false, it clears actor state bits `3` and `4`, sets bit `11`, and zeroes the
increment. Negative starting increments skip the entire sequence/flag-reset
block and do not capture bit `3`.

The following decrement uses state and mode reread after those helpers:

| Condition, in priority order | Increment update |
| --- | --- |
| Saved pre-reset state bit `3` was set | Replace with signed `0xFFFF0088` (`-65,400`) |
| Model mode `0` and either state bit `2` or byte `0x80186464` nonzero | If nonnegative, first set to `-0x199`; then subtract `0x332` when word `0x801864C0` is nonzero, otherwise subtract `0x199` |
| Otherwise state bit `9` set | Subtract `0x51` |
| Otherwise | Subtract `0x51C` |

These are wrapping 32-bit increment operations. The large saved-bit value is
not `-0x88`, and a nonnegative increment is normally zeroed before this table.
Helper changes to component, mode, state, pending vector, or stop byte are
visible where the original rereads them; the move-info pointer itself is saved.

It next caps current stride at `35`. Nonzero byte `0x8018649C` clears the
active marker and returns. Otherwise nonzero pending **X or Z**
(`+0x38/+0x40`) skips the prospective-stride calculation and vector producer,
without clearing the active marker. Pending Y alone does not block movement.

When X/Z are zero, it predicts the result of the downstream Q12 step. A result
above `35` rewrites starting stride so the next step lands on `35`; a result
below zero similarly lands on zero, but only if current stride is nonnegative.
It then calls `func_80022944` exactly once. The conversion retains the MIPS
wrapping negation before arithmetic right shift: `INT_MIN` produces a positive
`524,288` step here, unlike mathematical signed division. The C uses unsigned
wrapping arithmetic to avoid undefined signed overflow; this is preservation
of the target behavior, not a new physics rule.

The [native harness](../tests/test_bubsy_grounded_deceleration.c) executes
1,310,720 boundary combinations with an independent arithmetic/call-order
oracle, plus helper mutations. Coverage includes ordinary and unknown model
modes, all five relevant starting state bits, stopped paths, increment sign
and wrap boundaries, all pending components, stop-before-pending priority,
assertion continuation, saved-bit behavior after state changes, component
replacement, saved move-info pointers, both prospective bounds, and preservation
of producer mutations. Address/undefined-behavior sanitizer runs cover the
same matrix.

The vector producer `func_80022944` calls the 416-byte helper `func_800227A4` in
its special-state path at
`0x800227A4`; `src/game/func_800227A4.c` now byte-matches that L0 routine in C.
It reads the component translation, passes it with the move-info vector to
`func_80062E50`, and uses the resulting `+0x18` scalar and step `+0x30` for
threshold decisions. The selected paths either clear the remaining scalar and
add a translation delta to `+0x38`, or derive and scale a vector into the same
pending fields. This connects the special-state adjustment to the position
consumer, but the scalar's gameplay meaning remains unknown.

`func_800227A4` calls the 60-byte helper `func_80062D4C`, whose C source in
`src/game/func_80062D4C.c` is also byte-matched. It subtracts the component and
move-info vectors through `func_80010000`, then passes the difference and an
output pointer to `func_80053904`. This 88-byte helper is high-level C and
byte-matches L0. It sums the low 32 bits of the three component squares with
wraparound, then passes the squared length to `func_800535CC`. That wrapper
uses the GTE leading-zero count to index a 64-entry table whose values range
from 4096 to 5769, then extracts bit 12. Zero input returns zero; every
nonzero normalized table value returns one. Thus `func_80053904` tests whether
the component-to-move-info difference is nonzero; it does not compute distance
magnitude. The separate `func_80062E50` call computes the remaining-distance
approximation used by the movement threshold.

`func_800227A4` also calls the 148-byte `func_80062E50`; its C source in
`src/game/func_80062E50.c` byte-matches L0. It takes absolute differences on
three components, orders the largest against the other two, and returns the
largest plus one quarter of the other two summed. This is the approximation
used for the remaining-distance threshold; its higher-level gameplay meaning
is still unproven.

A readable host-side orchestration model for this routine is in
`game_apply_queued_move_request` in `src/game/move_requests.c`. Its injected
callbacks preserve the observed helper order and pointer ranges without
guessing the opaque helpers' internal math; native tests cover optional paths,
translation application, archival, clearing, and the missing-info assertion.

The full bounded caller `func_80025A98` now has a byte-matched readable C
implementation in `src/game/func_80025A98.c`, alongside the host-side control-flow
model `game_prepare_move_request` in `src/game/move_requests.c`. The model covers the sentinel-state return,
control-table cursor/repeat updates, component-entry loop, transform stages,
and final pending-translation enqueue gate. Component translation reads and
global-transform writes use the direct null-basis models of `func_80054B20`
and `func_80054C7C`; calls whose engine effects are not recovered remain
injected through `GameMovePreparationOps`. Host guards for invalid views are
not claims about target behavior. The action-plan functions below
cover only their named enqueue slices, not complete translations of the other
three enclosing routines. Sequence-directory reads are modeled directly by
`game_move_request_lookup_table` and `game_move_request_read_selector`, with
native tests for their signed-index bounds and output writes. The model also
directly reads the component's signed matrix vector and computes the
three-component difference and wrapped squared sum before delegating scalar
conversion through `GameComponentVectorMagnitudeOps`. These helpers have
focused native tests; scalar conversion and surrounding engine transforms
remain opaque callbacks rather than guessed implementations.

The 48-byte helper `func_80054AF0` at `0x80054AF0..0x80054B20` reads the signed
matrix elements at `+0x04/+0x0A/+0x10`, negates Y, and returns zero. Its
high-level C source in `src/game/func_80054AF0.c` byte-matches the original with
the pinned GCC 2.6.3 PSX profile.

The shared 348-byte helper `func_80054B20` at `0x80054B20..0x80054C7C` has 137
direct L0 JAL call sites, including its recursive call. Its high-level C source
in `src/game/func_80054B20.c` is now byte-matched with the pinned GCC 2.6.3 PSX
profile. The null-basis and global-basis paths copy the component translation
at `+0x14/+0x18/+0x1C` to the output vector and negate Y; a differing basis is
normalized and transformed through the observed helper calls.

The shared 360-byte helper `func_80054C7C` at `0x80054C7C..0x80054DE4` has 92
direct L0 JAL call sites, including its recursive call. Its high-level C source
in `src/game/func_80054C7C.c` byte-matches with pinned GCC 2.6.3 PSX. A null
component pointer returns `0x18`; otherwise the null-basis path writes the three
translation words at component offsets `+0x14/+0x18/+0x1C`, negating Y.
Neighboring helper `func_80054414` transposes the nine matrix halfwords at
`+0x00..+0x10` and copies the translation words at `+0x14..+0x1C`;
`func_80054488` copies the nine matrix halfwords without transposing. Both are
byte-matched C helpers; the former leaves the padding halfword at `+0x12`
untouched. Together with the component setters, these operations establish the
matrix layout and basis-conversion boundary. The 120-byte C helper
`func_800546F4` copies the matrix, negates the four halfwords at `+0x02`,
`+0x06`, `+0x0A`, and `+0x0E`, and zeros translation. The 92-byte C wrapper
`func_8005476C` then restores the input translation while negating Y. In the
3x3 matrix this negates the four coefficients coupling the Y axis to X and Z;
it is a coordinate handedness/sign conversion, not evidence by itself that the
result is world-space. The inverse-basis wrapper `func_8005406C` transposes
the supplied matrix with `func_80054414`, then delegates the point operation
to `func_80053FA4`; this 80-byte wrapper is an exact C match. That routine has
an identity-basis path that copies its four input words unchanged, and a
non-identity path that calls the GTE routine at `0x80077298`. Its 352-byte
boundary and raw instruction transcription are byte-exact. A separate
high-level C behavioral model covers the routine's observable three-word
result; the exact source still uses COP2 instructions, so its byte-exact source
remains the assembly transcription. The routine loads the 3x3 matrix into GTE registers, performs fixed-point vector work, and stores three output words.
The observed MVMVA commands select no translation vector; basis translation
is handled separately by `func_80010038`. In the
forward direction, the `func_80064528` path
uses `func_80053FA4`, adds the adjusted basis translation with `func_80010038`,
then performs the common final Y flip and stores. The 116-byte inverse helper
`func_80054120` subtracts basis translation with `func_80010000`, then calls
the exact transposing wrapper `func_8005406C`; its C source matches all bytes.
The 100-byte forward helper `func_800540BC` follows the corresponding
transform-then-add sequence and is a byte-exact high-level C match. The host model
`game_component_basis_transform_point_q12` in `component_transform.c` models
the bounded GTE routine's two-pass MVMVA: each signed input is split into high
and low 15-bit parts, sign-extended through the GTE's 16-bit IR registers,
transformed with SF=0 and SF=1, then recombined as `high * 8 + low`. An identity
basis copies all four input words. Native tests cover identity, a quarter-turn
matrix, negative fractional shifts, overflow-sensitive inputs, signed 15-bit
split boundaries, and the untouched fourth output word. The C model emulates
the observable outputs rather than GTE control/flag side effects.
The 352-byte body is bounded in
the code map as `func_80077298_gte_basis_transform_point`; all three signed
conversion paths and its return are accounted for. The 200-byte
`func_80053FA4` identity-basis fast path and GTE dispatch is also a byte-exact
high-level C match. The host
model `game_component_transform_set_translation` captures the normalization
and branches in `src/game/component_transform.c`; its callback represents that
complete transform result, including basis translation. The adjacent L0 helper
`func_80055280` is now a high-level C exact match for its 236-byte body. It reads
the component translation in the optional basis, adds the requested delta,
writes the translated origin through that basis into a temporary identity
transform, then rebuilds the component orientation with `func_800550A4`. This
476-byte helper is also high-level C and byte-exact: it derives a direction
from the difference between the component and reference origins, conditions
and normalizes it, then forms two cross-product axes against a vertical
reference and copies the resulting matrix into the component. This is a
direction-based orientation update, not a parent-transform link. All nine
direct L0 callers of `func_80055280` pass null; its internal reference at
`0x80013CC0` is the 3x3 Q12 identity matrix. The observed update paths therefore
provide no evidence of parent-relative composition and operate through the
identity/global basis. This narrows the grounded component path toward
engine-global coordinates. The global basis template at `0x800A42EC` is the
Q12 identity matrix with zero translation, and `func_80054624` copies this
template into allocated basis objects. The getter/setter contract therefore
establishes that component translation fields are world/global-space values,
not parent-local coordinates; a non-null basis is a conversion frame used at
the API boundary. Rendering may subsequently convert those world coordinates
into camera/view space, which is a separate stage.

For the grounded movement queue specifically, the exact consumer
`func_80024F2C` passes a null basis (`0`) to both `func_80054B20` and
`func_80054C7C`. Both therefore take their direct component-translation paths;
the non-identity GTE basis conversion described above is not used by this
consumer. The queued vector is added to the stored component translation, with
Y negated on the read/write boundary. The paired basis paths establish the
coordinate contract: writing through a non-null basis transforms the point by
its matrix and adds its translation; reading into a basis subtracts that
translation and applies the transposed matrix, after the observed Y-sign
normalization. Null basis is treated as the identity global basis, so the
grounded queue delta is expressed in the engine-global basis rather than in a
caller-supplied local basis. Thus the grounded queue updates world/global
translation directly; the Y sign conversion is separate from the basis frame.

The renderer camera path uses a global view transform at `0x801D34EC`. The
three-slot camera records contain a 3D position and three rotation bytes;
`func_8004EC20` updates a record and calls `func_80072798` to rebuild that
global matrix. `func_800722C8` copies the view transform, composes its rotation
with an object transform while saving/restoring GTE control state, and loads
the result as the GTE light matrix. The exact C helper `func_80072298` loads an
RT rotation matrix and translation into the GTE. This confirms that the
component global/world frame feeds renderer GTE setup; the full packet-building
and RTPT/RTPS submission path remains to be recovered as high-level C.

The three camera-slot routines now match all 460 bytes from readable C:
`func_8004EC20` (220), `func_8004ECFC` (124), and `func_8004ED78` (116).
The clear and rotation-only setters need no register constraints or barriers.
The full setter samples the mode byte after its transform callback and writes
the slot status afterward. Native tests verify invalid indices, unaffected
neighboring slots, and callback mutations. Instruction snippets have also been
removed from the exact C basis shortcut `func_80053FA4`, forward wrapper
`func_800540BC`, view-matrix copy `func_800722C8`, approximate distance helper
`func_80062E50`, vector zero-result stores, and the move consumer assertion.
The distance harness covers 262,144 signed/wrapping boundary combinations.