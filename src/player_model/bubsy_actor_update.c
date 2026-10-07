#include "bubsy_actor_update.h"

BubsyActorUpdateEntryResult bubsy_actor_update_entry_gate(
    BubsyActorUpdateActorView *actor,
    uint8_t update_mode,
    BubsyActorUpdateEnvironment *environment,
    uint8_t *local_state_24
) {
    if (environment->update_gate_6454 != 0) {
        *local_state_24 = 0;
        actor->update_state_10 = 0;
    } else {
        *local_state_24 = update_mode;
        if (update_mode != 0) {
            environment->update_mode_647A = 0;
            environment->sequence_counter_38C = 0;
        }
    }

    if (actor->update_state_10 != 0) {
        return BUBSY_ACTOR_UPDATE_SKIP;
    }

    environment->actor_event_guard_6460 = 0;
    return BUBSY_ACTOR_UPDATE_CONTINUE;
}

int32_t bubsy_actor_update_should_select_309(
    const BubsyActorUpdateActorView *actor,
    int32_t current_sequence_key,
    uint8_t runtime_mode
) {
    return (actor->flags_04 & 0x80u) != 0 &&
        runtime_mode == 0 &&
        current_sequence_key != 0x309 &&
        (actor->update_state_10 & 0x00800000u) == 0;
}

uint8_t bubsy_actor_update_local_state_for_sequence(
    const BubsyActorUpdateActorView *actor,
    int32_t current_sequence_key,
    uint8_t runtime_mode,
    uint8_t local_state_24
) {
    if ((actor->flags_04 & 0x100u) != 0) {
        return runtime_mode == 0 ? 0 : local_state_24;
    }

    if (current_sequence_key == 0x32E && runtime_mode == 0) {
        return 1;
    }

    return local_state_24;
}

int32_t bubsy_actor_update_should_check_mode1_sequence_boundary(
    int32_t current_sequence_key,
    uint8_t runtime_mode
) {
    return runtime_mode == 1 &&
        (current_sequence_key == 0x9A || current_sequence_key == 0x54);
}

int32_t bubsy_actor_update_should_reset_sequence_counter(
    const BubsyActorUpdateActorView *actor,
    int32_t sequence_counter_after_increment,
    uint8_t runtime_mode,
    int16_t level_id
) {
    return (runtime_mode == 1 && sequence_counter_after_increment >= 12) ||
        level_id == 0x14 ||
        (actor->flags_04 & 0x40u) != 0;
}
