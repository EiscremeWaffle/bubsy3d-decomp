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
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register const PlayerActorSequenceView *actor_value __asm__("$6") = actor;
    register uint8_t *output __asm__("$5") = result;
    register uint32_t flags __asm__("$2");
    register int32_t next_cursor __asm__("$4");
    register const PlayerActorSequenceData *sequence __asm__("$7");
    register const int32_t *entries __asm__("$7");
    register int32_t current_entry __asm__("$4");
    register int32_t quotient __asm__("$3");
    register uintptr_t next_address __asm__("$2");
    uint32_t next_entry;

    __asm__ volatile("" : "=r"(output) : "0"(output));
    flags = actor_value->flags_04;
    __asm__ volatile("" : "=r"(flags) : "0"(flags));
    next_cursor = actor_value->cursor_08;
    sequence = actor_value->sequence_18;
    __asm__ volatile("" : "=r"(next_cursor), "=r"(sequence) : "0"(next_cursor), "1"(sequence));
    if ((flags & 4) == 0) {
        goto boundary_true;
    }
    if (actor_value->state_05 == -1) {
        *output = 0;
        __asm__ volatile("" : : : "memory");
    }
    if (actor_value->advance_cursor_07 != 0) {
        next_cursor++;
    } else {
        next_cursor--;
    }
    entries = sequence->entries;
    next_address = (uint32_t)next_cursor * sizeof(int32_t) + (uintptr_t)entries;
    __asm__ volatile("" : "=r"(next_address) : "0"(next_address));
    next_entry = (uint32_t)*(const int32_t *)next_address + 4;
    if (next_entry >= 4) {
        goto boundary_false;
    }
    current_entry = entries[actor_value->cursor_08];
    quotient = current_entry / 1000;
    if (actor_value->threshold_0E < quotient) {
        goto boundary_false;
    }
boundary_true:
    *output = 1;
    goto boundary_done;
boundary_false:
    *output = 0;
boundary_done:
    return 0;
#else
    int32_t next_cursor;
    int32_t next_entry;
    int32_t current_entry;

    if ((actor->flags_04 & 0x04) == 0) {
        *result = 1;
        return 0;
    }

    if (actor->state_05 == -1) {
        *result = 0;
    }

    next_cursor = actor->cursor_08 + (actor->advance_cursor_07 != 0 ? 1 : -1);
    next_entry = actor->sequence_18->entries[next_cursor];
    if (next_entry < -4 || next_entry > -1) {
        *result = 0;
        return 0;
    }

    current_entry = actor->sequence_18->entries[actor->cursor_08];
    *result = actor->threshold_0E >= current_entry / 1000;
    return 0;
#endif
}

int32_t player_actor_read_sequence_remainder(const PlayerActorSequenceView *actor, int32_t *remainder_out) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register const PlayerActorSequenceView *actor_value __asm__("$3") = actor;
    register int32_t *output __asm__("$6");
    register const PlayerActorSequenceData *sequence __asm__("$5");
    register int32_t entry __asm__("$5");
    register int32_t quotient __asm__("$4");

    __asm__ volatile("" : "=r"(actor_value) : "0"(actor_value) : "memory");
    output = remainder_out;
    __asm__ volatile("" : "=r"(output) : "0"(output) : "memory");
    sequence = actor_value->sequence_18;
    if ((sequence->flags_2C & 0x04) != 0) {
        *output = actor_value->cursor_08;
        return 0;
    }
    entry = sequence->entries[actor_value->cursor_08];
    quotient = entry / 1000;
    *output = entry - quotient * 1000;
    return 0;
#else
    int32_t entry;

    if ((actor->sequence_18->flags_2C & 0x04) != 0) {
        *remainder_out = actor->cursor_08;
        return 0;
    }

    entry = actor->sequence_18->entries[actor->cursor_08];
    *remainder_out = entry % 1000;
    return 0;
#endif
}

int32_t player_actor_read_cursor_delta(const PlayerActorSequenceView *actor, int32_t *delta_out) {
    *delta_out = actor->cursor_08 - actor->previous_cursor_0C;
    return 0;
}

int32_t player_actor_write_previous_cursor_minus_two(const PlayerActorSequenceView *actor, int32_t *value_out) {
    *value_out = actor->previous_cursor_0C - 2;
    return 0;
}

int32_t player_actor_select_sequence_state(PlayerActorSequenceView *actor, int32_t index) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register volatile PlayerActorSequenceView *actor_value __asm__("$6") = actor;
    register const volatile PlayerActorSequenceData *sequence __asm__("$4") = actor_value->sequence_18;
    register int32_t index_value __asm__("$5") = index;
    register uint32_t entry_offset __asm__("$7") = (uint32_t)index * sizeof(int32_t);
    register int32_t next_cursor __asm__("$3");
    register uintptr_t state_address __asm__("$3");
    register uint16_t next_entry __asm__("$2");
    register uint8_t state_byte __asm__("$4");

    if (index_value >= ((const PlayerActorSequenceData *)sequence)->entry_count) {
        return PLAYER_SEQUENCE_INDEX_PAST_TABLE;
    }
    if (*(const int32_t *)(entry_offset + (uintptr_t)sequence->entries) >= 0) {
        return PLAYER_SEQUENCE_ENTRY_NOT_NEGATIVE;
    }

    next_cursor = index_value + 2;
    actor_value->cursor_08 = next_cursor;
    next_entry = *(const volatile uint16_t *)(entry_offset + (uintptr_t)sequence->entries + 4);
    actor_value->previous_cursor_0C = next_cursor;
    actor_value->unknown_0A = next_entry;
    __asm__ volatile("" : : : "memory");
    state_address = (uintptr_t)sequence->entries;
    __asm__ volatile("" : "=r"(state_address) : "0"(state_address));
    state_address = entry_offset + state_address;
    state_byte = *(const volatile uint8_t *)state_address;
    actor_value->threshold_0E = 1;
    actor_value->state_05 = state_byte;
    return PLAYER_SEQUENCE_STATE_SELECTED;
#else
    const PlayerActorSequenceData *sequence;
    int32_t entry;
    int16_t next_cursor;

    sequence = actor->sequence_18;
    if (index >= sequence->entry_count) {
        return PLAYER_SEQUENCE_INDEX_PAST_TABLE;
    }

    entry = sequence->entries[index];
    if (entry >= 0) {
        return PLAYER_SEQUENCE_ENTRY_NOT_NEGATIVE;
    }

    next_cursor = (int16_t)(index + 2);
    actor->cursor_08 = next_cursor;
    actor->previous_cursor_0C = next_cursor;
    actor->unknown_0A = (uint16_t)sequence->entries[index + 1];
    actor->threshold_0E = 1;
    actor->state_05 = (int8_t)entry;
    return PLAYER_SEQUENCE_STATE_SELECTED;
#endif
}

int32_t player_actor_shared_state_clear_progress(
    uint32_t actor_flags,
    int32_t current_sequence_key,
    PlayerActorSharedStateView *shared_state
) {
    if (current_sequence_key != 0x2A7 && (actor_flags & 0x1100u) == 0) {
        return 0;
    }

    shared_state->progress_30 = 0;
    shared_state->progress_34 = 0;
    return 1;
}

void player_actor_shared_state_apply_mode1_numeric_update(
    uint32_t actor_state_10,
    PlayerActorSharedStateView *shared_state
) {
    if ((actor_state_10 & 0x8000u) != 0) {
        return;
    }

    if ((actor_state_10 & 0x10000u) != 0) {
        shared_state->progress_30 = -0x34;
    }
}

void player_actor_shared_state_clamp_mode0_progress(
    uint32_t actor_state_10,
    PlayerActorSharedStateView *shared_state
) {
    if ((actor_state_10 & 0x18000u) != 0) {
        return;
    }

    if (shared_state->progress_30 < 5) {
        shared_state->progress_30 = 5;
    }

    if (shared_state->progress_30 >= 0x11) {
        shared_state->progress_30 = 0x11;
    }
}

int32_t player_actor_shared_state_make_mode23_plan(
    uint8_t state_mode,
    uint32_t actor_state_10,
    uint8_t runtime_mode,
    PlayerActorSharedStateMode23Plan *plan_out
) {
    if ((state_mode != 2 && state_mode != 3) || (actor_state_10 & 0x10000u) != 0) {
        return 0;
    }

    plan_out->callback_flags = state_mode == 3 ? 0x400 : -0x400;
    if ((actor_state_10 & 0x8000u) == 0) {
        plan_out->temporary_progress = 0xE;
    } else {
        plan_out->temporary_progress = runtime_mode != 0 ? 6 : 8;
    }
    return 1;
}

int32_t player_actor_state23_should_dispatch_event_17(
    uint32_t actor_flags,
    uint8_t state_6463,
    int32_t sequence_value
) {
    return sequence_value == 0x29A &&
        (actor_flags & 0x2000u) == 0 &&
        state_6463 != 1 &&
        state_6463 != 2 &&
        state_6463 != 4;
}
