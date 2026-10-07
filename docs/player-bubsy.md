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

Objdiff credits 32 bytes across these two functions. The four-way model selector
above remains unmatched, as do the Bubsy actor-update and death-state routines.
The whole-game source percentage therefore still rounds to `0.00%` on decomp.dev.

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
set and clear bit `0x04` in actor byte offset `0x04`. Their straightforward C
translations live in `player_actor_flags.c` and pass native state tests, but are
not byte-matched: the available compiler chooses different registers and
delay-slot instructions. They are kept out of the getter base object so fuzzy
similarity cannot be mistaken for verified matching progress.

The same update calls `0x8005290C` with the actor's component pointer and an
output byte. Its bounded code reads actor offsets `0x04`, `0x05`, `0x07`, `0x08`,
and `0x0E`, plus sequence data through a pointer at `0x18`. When flag bit `0x04`
is set, it steps a cursor forward or backward according to byte `0x07`; table
values `-4` through `-1` take a threshold path using signed division by 100.
Otherwise it compares the cursor-derived value directly to the field at `0x0E`.
The C translation `player_actor_sequence_boundary_reached` has host tests for
these branches, but the underlying concept may be animation timing, movement
sequencing, or another actor-sequence protocol; that meaning is not established.

## Actor Event Dispatch

The actor update calls `0x80037214` with mode `0` and the actor pointer. The
helper reads the current event through the actor component and consults a
runtime mode plus L0 state byte `0x80186463`. Verified branch outcomes include:

- Mode `0`, runtime mode `0`, event `0x3E0`, and state not `3`: select property
	`0x35`, set actor flag `0x04`, and dispatch with flags `0`.
- Runtime mode `1` and event `0x157` (when the special branch is reached):
	select property `0x54`, set actor flag `0x04`, and dispatch with flags `0`.
- Other supported mode `0/1` paths select property `0x3E0` or `0x157` according
	to runtime mode, set flag `0x04`, and dispatch with flag `0x40000000`.
- Other update modes return without dispatch.

The C reconstruction in `bubsy_actor_events.c` keeps event/property IDs and
engine operations explicit through callbacks. Their gameplay meanings and the
engine callback implementations are still unknown; this function is not yet a
byte match.

## Death-State Routine

`0x8003737C..0x80037AF8` is rooted as `bubsy_handle_death_state`. The debug
assertion at `0x8003767C` names `gBubsyInfo.deathType >= 0` and reports
`../f/bubsy.c`, line `0x912`. The routine initializes a large temporary state
frame, reads and normalizes the death-type global at `0x80186458`, dispatches
through numeric state values, calls engine animation/event helpers, and returns
through the matching `0x158`-byte stack-frame epilogue at `0x80037AF4`.

The observed state values and timing arithmetic are not yet mapped to named
death animations or gameplay rules. The routine is in the code treemap, but its
source behavior still needs instruction-by-instruction reconstruction.

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