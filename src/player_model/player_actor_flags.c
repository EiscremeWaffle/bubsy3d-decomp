#include "player_model.h"

int32_t player_actor_set_flag_04(PlayerActorFlagByte *actor) {
    actor->flags_04 |= 0x04;
    return 0;
}

int32_t player_actor_clear_flag_04(PlayerActorFlagByte *actor) {
    actor->flags_04 &= (uint8_t)~0x04;
    return 0;
}

int32_t player_actor_sequence_boundary_reached(const PlayerActorSequenceView *actor, uint8_t *result) {
    if ((actor->flags_04 & 0x04) == 0) {
        *result = actor->threshold_0E >= actor->cursor_08 / 100;
        return 0;
    }

    if (actor->state_05 == -1) {
        *result = 0;
    }

    const int16_t next_cursor = actor->cursor_08 + (actor->advance_cursor_07 != 0 ? 1 : -1);
    const int32_t next_entry = actor->sequence_18->entries[next_cursor];
    if (next_entry < -4 || next_entry > -1) {
        *result = 0;
        return 0;
    }

    const int32_t current_entry = actor->sequence_18->entries[actor->cursor_08];
    *result = actor->threshold_0E >= current_entry / 100;
    return 0;
}

int32_t player_actor_read_sequence_remainder(const PlayerActorSequenceView *actor, int32_t *remainder_out) {
    if ((actor->sequence_18->flags_2C & 0x04) != 0) {
        *remainder_out = 0;
        return 0;
    }

    const int32_t entry = actor->sequence_18->entries[actor->cursor_08];
    *remainder_out = entry % 100;
    return 0;
}

int32_t player_actor_read_cursor_delta(const PlayerActorSequenceView *actor, int32_t *delta_out) {
    *delta_out = actor->cursor_08 - actor->previous_cursor_0C;
    return 0;
}
