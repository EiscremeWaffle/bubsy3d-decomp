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

The validated report currently credits 18,404 exact code bytes across 93
functions, or `0.613030%` of mapped code. The total includes two Clang-built
getters, eight original GCC C functions, the assembly-backed death and
grounded-state handlers, 20 assembly-backed copies of `func_80010128`, the L0
assembly-backed Q20.12 dot product `func_800100A0`, 22 C copies of
`func_80072F4C`, and 38 repeated copies of two actor-flag/cursor helpers. The
actor-event dispatcher remains fuzzy; the target-only C draft for
`shared_actor_state_dispatch` is also not exact and earns no progress credit.
The progress site changes only after this snapshot is committed, pushed, and
published by GitHub Actions.

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
for the zero-gate, nonzero-actor-state, and forced-clear branches. It represents
only this entry slice; the rest of `bubsy_update_actor_state` remains incomplete
and the slice is not byte-matched.

Actor byte `+0x04` bits `0x80` and `0x100` steer distinct sequence paths. On
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
For level `0x14`, a later path is entered only in runtime mode `0`, with a
positive sequence counter, actor flag `0x40` clear, and update gate
`0x80186454` zero. The predicate is translated and tested in
`bubsy_actor_update_should_enter_level14_counter_path`; the calls it gates are
still unidentified.

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
callers and takes an actor pointer, a pointer to a halfword state ID, and a
mode; observed callers use modes `0`, `1`, and `2`, so it is not Bubsy-only.
It derives a key from component halfword `+0x0C`, tests actor mask `0x1100`,
updates component fields `+0x30/+0x34`, and calls shared engine helpers. Its
gameplay role remains unresolved; the complete function is not translated or
byte-matched. Its initial branch is translated in
`player_actor_shared_state_clear_progress`: key `0x2A7` or any overlap with
actor mask `0x1100` clears component words `+0x30` and `+0x34`.
In mode `1`, actor state mask `0x8000` bypasses the numeric update; otherwise
mask `0x10000` sets `+0x30` to `-0x34`. When that mask is clear, `+0x34` is
temporarily set to zero if `+0x30 < -0x11`, or to `-0x198C` otherwise; the
callback path then restores its saved value. The scalar update is translated in
`player_actor_shared_state_apply_mode1_numeric_update`; the callback effects and
remaining modes are not yet translated. The
mode-0 numeric stage clamps `+0x30` to `5..0x11` unless actor state mask
`0x18000` bypasses it. Before the callback, `+0x34` is temporarily set to
`0x198C` below the upper cap or cleared at the cap, then restored by the shared
epilogue. The surviving clamp is translated in
`player_actor_shared_state_clamp_mode0_progress`; its callback side effects and
other mode processing remain incomplete. Modes `2` and `3` skip their
callback path when actor mask `0x10000` is set; otherwise they prepare callback
flags `-0x400`/`0x400` and temporary progress `0xE`, `6`, or `8`. The bounded
argument-selection logic is captured by `player_actor_shared_state_make_mode23_plan`;
the callbacks and their gameplay meaning remain unresolved.

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

The default-state block at `0x80036874` can route to `0x8004B35C` when the
runtime state byte `0x80186461` is `5`, the actor `+0x10` mask `0x10000` is
clear, lock byte `0x80186479` and gate halfword `0x80186454` are zero, and
actor flag `+0x04` bit `0x100` is clear. Descriptor state `0x25` takes this
route directly; state `0x26` takes it only when descriptor word `+0x04` is not
`-2`. Runtime mode zero selects `0x8004B35C`; nonzero mode selects
`0x8004BFF8`. `bubsy_actor_state_route_default` models and tests these routing
conditions.

The grounded handler starts with a `0x68`-byte frame and contains a 33-entry
computed state switch at `0x8004B4D0`, backed by the table at `0x800139A8`.
Every table target lies inside the handler body, but the L0 discovery map stops
at the indirect jump, so this handler is not yet a verified bounded function.
The inspected stores update actor state/flags, a companion object field, and
global counters; no direct world-position or velocity write has been identified
yet. Treat this as the grounded actor-state route, not a proven locomotion
integrator.
Before the table dispatch, descriptor lookup value `3` enables special gates
for states `14` and `15`. State `14` skips the switch when actor flags overlap
`0x1100`, or when actor state bit `0x1000` is set; in the latter case it first
sets descriptor halfword `+0x02` to `1` if that field was zero. State `15`
skips and clears descriptor halfword `+0x02` when actor state bit `0x1000` is
set. Other inputs continue to the computed switch. This prelude gate is
modeled and tested.
The extracted host dispatch map preserves all 33 targets for IDs `6..38`;
out-of-range IDs take the shared epilogue at `0x8004BFD0`. The table is tested
against the original executable's entries. Several IDs alias the same block,
and IDs `18`, `19`, and `22..28` target the epilogue directly.
The handler-entry gate and each unique non-epilogue switch path now have
tested host-side control-flow/state models in `bubsy_actor_update.c` and
`test_player_model.c`. These are not yet a single reconstructed C body or a
byte-matched function; called engine helpers remain outside these models.
An instruction-store audit found no direct actor/descriptor coordinate or
velocity writes in this handler. Its grounded-state dispatch is therefore
mapped more completely, but grounded locomotion itself is not established.
The bounded handler is now also an exact match through the explicitly
assembly-backed mnemonic transcription in `bubsy_grounded_state_match.c`,
verified at `0x8004B35C..0x8004BFF8` with pinned GCC 2.6.3/MASPSX and LLD. This
earns byte-match credit only; it does not upgrade the control-flow models into
recovered high-level C or establish a position/velocity integrator.

Switch case `38` targets `0x8004B6C0`. Its descriptor byte `+0x11` changes from
`2` to `4`, or from `1` to `0`; only the latter path calls `0x8002262C` with
argument `0x11`. All three outcomes then call `0x80039158` with the actor and
third argument. The byte transition and conditional argument are modeled and
tested by `bubsy_grounded_case38_update_descriptor`; the called helpers'
effects and the state's gameplay meaning remain unknown.

Case `37` at `0x8004B66C` does nothing further when actor flags `+0x04` overlap
`0x1100`. Otherwise descriptor byte `+0x11` value `0` changes to `1` and calls
`0x80037B30(0)`; value `5` skips the remaining work; other values call
`0x80039158(actor, third_argument)`. Cases `29` and `30` directly set
descriptor byte `+0x10` to `1` and `0`, respectively. These observed branch
decisions and writes are modeled and tested; helper effects remain unresolved.
Case `13` sets actor byte `+0x01` to `2`. Cases `34` and `36` set that same
actor byte to `2` and clear descriptor halfword `+0x00`. These direct writes
are also modeled and covered by host tests.

Case `21` (`0x8004B578`) computes a dot product of two normalized vectors. It
then clears descriptor halfword `+0x02` on every path. The path calling
`0x8002E7D8(actor)` and `0x80022650(actor, 8)` is selected only when actor state
`+0x10` bit `0x1000` is set, actor flags `+0x04` bit `0x40` is clear, the
companion's `+0x28` word is zero or the companion pointer is null, the signed
dot result is less than `0xCCC`, and descriptor state `+0x02` is not `5`.
Otherwise it calls `0x8002262C(actor, 0xC)`. This predicate and write are
modeled and tested. The threshold path also reaches `0x800347EC(actor,
third_argument, 1)`; helper effects remain unresolved.

Case `12` (`0x8004B70C`) proceeds only when actor flags `+0x04` have a nonzero
overlap with `0x818`, no overlap with `0x1104`, and actor state `+0x10` has no
overlap with `0xA0084`. It writes `5` to descriptor halfword `+0x02`. Actor
state bit `0x1000` additionally requests `0x8002262C(actor, 0xC)`; if actor
byte `+0x01` is not already `1`, it sets it to `1` and calls
`0x800347EC(actor, third_argument, 1)`. These gates, writes, and call requests
are modeled and tested; helper semantics remain unknown.

Case `17` (`0x8004B7F8`) clears descriptor byte `+0x14`, sets actor byte
`+0x01` to `2`, and calls `0x8001C700(4, 3)`. If actor state `+0x10` overlaps
`0x44`, it also clears actor byte `+0x00`, calls `0x80022650(actor, 2)`, and
then calls `0x800347EC(actor, third_argument, 1)`. These writes and call gates
are represented by the tested action model; callee effects are unresolved.
Case `16` (`0x8004B77C`) initializes descriptor byte `+0x14` to `1` when it
is zero and actor flags `+0x04` bit `0x02` is clear. It then requests
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
nonzero key additionally selects index `0`, calls `0x80052E18`, and calls
`0x80022650(actor, 0x17)`; both zero and nonzero keys set linked word `+0x30`
to `-0x11` and call `0x80022944(actor)`. Counter gates and followups are
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
Case `32` (`0x8004BB68`) returns without writes when actor state `+0x10` has
bit `0x20000` or actor flags `+0x04` have bit `0x40`. State bit `0x1000`
clears descriptor halfword `+0x02`. Otherwise state mask `0x44` calls
`0x8002262C(actor, 0x17)`, clears descriptor byte `+0x4C`, and sets byte
`+0x68` to `1`. With those gates clear, descriptor counter `+0x4D` below `11`
and actor-state mask `0x01002000` clear selects calls
`0x80022650(actor, 0x10)` and `0x800347EC(actor, third_argument, 1)`, clears
linked word `+0x30`, sets actor byte `+0x01` to `2`, and clears the counter.
Otherwise it calls `0x8002262C(actor, 0x17)` and `0x8003539C(actor, 1)`, then
clears the counter. This whole switch block is modeled and tested; helper
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
tested; helper meanings remain unresolved.
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
modeled and tested; the shared callees remain opaque.
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
three-component Q20.12 vectors. Its C host model computes the signed dot
product; its target profile is assembly-backed and byte-exact for the single
L0 copy. This confirms transform math on the grounded path, but does not by
itself establish how the resulting value changes Bubsy's world position or
velocity.

The transform wrapper also calls `func_80053D68`, a vector range-conditioning
helper that selects a shift from the largest component magnitude. Its C model
and threshold tests are in `player_vector_range.c`; the current 36.9% objdiff
similarity is diagnostic only and earns no exact credit.

The grounded handler calls `func_8005395C`, which range-conditions a 3-vector
with `func_80053D68`, obtains one scalar from `func_8005385C`, and scales each
conditioned component by that scalar through `func_80053350`. The bounded
`func_8005385C` body squares each nonzero Q20.12 component with
`func_8001007C`, adds the three results, and passes the sum to the
GTE-backed `func_8005359C` conversion. This is consistent with a fixed-point
vector-length calculation, but its output's gameplay role is unknown. The C
models and host tests are in `player_vector_transform.c` and
`test_player_model.c`; neither helper is byte-matched or earns exact credit.
This processing has not been shown to update Bubsy's world position or
velocity.

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
original two-argument ABI and follows the observed branches, but still differs
from the original 316-byte body. It remains a fuzzy candidate until compiled C
matches every byte. Gameplay meanings and engine callback implementations are
still unknown.

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
shared game infrastructure; no direct call from `bubsy_update_actor_state` has
been established, so it is not yet labeled Bubsy-specific movement behavior.