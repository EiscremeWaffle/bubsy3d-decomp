# Bubsy Player Code: First Pass

This is a byte-grounded first decompilation pass, not a finished matching C
implementation. The USA executables are still the authority for behavior.

## Model Selection

The shared L0 level routine at `0x800296B4` is identified by four embedded model
paths and its callers. Its normal/swim branch is:

| Character selector | Config byte `+0x2A2A`, bit `0x10` | Model path |
| --- | --- | --- |
| nonzero | clear | `BUB.TZP` |
| nonzero | set | `BUBSWIM.TZP` |
| zero | clear | `PLISKIN.TZP` |
| zero | set | `PLISWIM.TZP` |

The character selector is read by `0x800283E0` from `0x801D89F0`; the nonzero
branch selects Bubsy-named resources. The active config pointer comes from
`0x800BE698`, and the flag is read at offset `0x2A2A`. The `0x10` flag is named
as a swimming/model-form flag because it switches to the `*SWIM.TZP` assets;
the config structure and flag's broader semantics remain unknown.

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

The getter has a separate [C implementation](../src/player_model/player_model_global.c).
With `BUBSY3D_MATCH_ORIGINAL_L0`, Zig 0.14.1's bundled Clang compiled that 16-byte
function to the original L0 bytes exactly. This uses an experimental MIPS-II
scheduling profile to place the store in the `jr` delay slot; the emitted
instructions are all MIPS-I. It does not identify the original Bubsy compiler.
Only this getter currently receives objdiff source-match credit; the selector
above remains unmatched.

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
recovered original symbol.

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
percentage remains zero until source-built code is compared and matched.