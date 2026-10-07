#ifndef BUBSY3D_BUBSY_ACTOR_UPDATE_H
#define BUBSY3D_BUBSY_ACTOR_UPDATE_H

#include <stddef.h>
#include <stdint.h>

typedef struct BubsyActorUpdateActorView {
    uint8_t unknown_00_to_03[4];
    uint32_t flags_04;
    uint8_t unknown_08_to_0F[8];
    uint32_t update_state_10;
} BubsyActorUpdateActorView;

_Static_assert(offsetof(BubsyActorUpdateActorView, flags_04) == 0x04, "actor flags offset must match the executable");
_Static_assert(offsetof(BubsyActorUpdateActorView, update_state_10) == 0x10, "actor update state offset must match the executable");

typedef struct BubsyActorUpdateEnvironment {
    int16_t update_gate_6454;
    uint8_t update_mode_647A;
    uint8_t actor_event_guard_6460;
    int32_t sequence_counter_38C;
} BubsyActorUpdateEnvironment;

typedef enum BubsyActorUpdateEntryResult {
    BUBSY_ACTOR_UPDATE_SKIP = 0,
    BUBSY_ACTOR_UPDATE_CONTINUE = 1,
} BubsyActorUpdateEntryResult;

BubsyActorUpdateEntryResult bubsy_actor_update_entry_gate(
    BubsyActorUpdateActorView *actor,
    uint8_t update_mode,
    BubsyActorUpdateEnvironment *environment,
    uint8_t *local_state_24
);
int32_t bubsy_actor_update_should_select_309(
    const BubsyActorUpdateActorView *actor,
    int32_t current_sequence_key,
    uint8_t runtime_mode
);
uint8_t bubsy_actor_update_local_state_for_sequence(
    const BubsyActorUpdateActorView *actor,
    int32_t current_sequence_key,
    uint8_t runtime_mode,
    uint8_t local_state_24
);
int32_t bubsy_actor_update_should_check_mode1_sequence_boundary(
    int32_t current_sequence_key,
    uint8_t runtime_mode
);
int32_t bubsy_actor_update_should_reset_sequence_counter(
    const BubsyActorUpdateActorView *actor,
    int32_t sequence_counter_after_increment,
    uint8_t runtime_mode,
    int16_t level_id
);
int32_t bubsy_actor_update_should_enter_level14_counter_path(
    const BubsyActorUpdateActorView *actor,
    int32_t sequence_counter,
    uint8_t runtime_mode,
    int16_t level_id,
    int16_t update_gate
);
int32_t bubsy_actor_update_sequence_index_for_local_state(
    const BubsyActorUpdateActorView *actor,
    uint8_t local_state_24,
    uint8_t runtime_mode
);
int32_t bubsy_actor_update_get_sequence_mode_override(
    uint8_t local_state_24,
    uint8_t runtime_mode,
    int32_t *override_out
);

#endif
