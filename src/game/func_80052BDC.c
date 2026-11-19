#include "input_events.h"
#include "player_model.h"
#include "sequence_playback.h"

extern void func_800556D0(
    int32_t failed, const char *condition, const char *source, uint32_t line
);
int32_t func_80052BDC(void *component, int32_t mode) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register PlayerActorSequenceView *actor __asm__("$16") = component;
    register const PlayerActorSequenceData *sequence __asm__("$19");
    register int32_t selected __asm__("$20");
    register int32_t selected_index __asm__("$17");
    register int32_t no_slot __asm__("$18");
    register int32_t index __asm__("$5");
    register int32_t slot_offset __asm__("$6");
    register int32_t oldest __asm__("$7");
    register int32_t count __asm__("$3");
    register int32_t loop_count __asm__("$8");
    register int32_t scan_key __asm__("$9");
    register SequencePlaybackSlot *scan_slots __asm__("$10");
    register SequencePlaybackSlot *slots __asm__("$4");
    register SequencePlaybackNode *active_node __asm__("$17");
    register int32_t slot_times_three __asm__("$3");
    register int32_t shifted_selected __asm__("$2");
    register int32_t shifted_flags __asm__("$2");
    register int32_t has_slots __asm__("$2");
    register int32_t next_index __asm__("$2");
#else
    PlayerActorSequenceView *actor = component;
    const PlayerActorSequenceData *sequence;
    int32_t selected;
    int32_t selected_index;
    int32_t no_slot;
    int32_t index;
    int32_t slot_offset;
    int32_t oldest;
    int32_t count;
    int32_t loop_count;
    int32_t scan_key;
    SequencePlaybackSlot *scan_slots;
    SequencePlaybackSlot *slots;
    SequencePlaybackNode *active_node;
    int32_t shifted_selected;
    int32_t shifted_flags;
    int32_t has_slots;
    int32_t next_index;
#endif
    int32_t key;
    uint32_t sample;

    if (actor == 0) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : : : "memory");
#endif
        return 1;
    }
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(actor) : "0"(actor) : "memory");
#endif

    sequence = actor->sequence_18;
    actor->flags_04 = (uint8_t)((actor->flags_04 & 0xFCu) + (uint32_t)mode);
    if (mode == 1) {
        if (actor->playback_node_00 == 0) {
            return 0;
        }
        func_80053180(actor->playback_node_00);
        actor->playback_node_00 = 0;
        if (actor->playback_slot_06 >= 0) {
            func_80050128(&sample);
            sequence->playback_slots[actor->playback_slot_06].active_08 = 0;
            sequence->playback_slots[actor->playback_slot_06].timestamp_0C = (int32_t)sample;
            actor->playback_slot_06 = -1;
        }
        return 0;
    }

    goto allocate;

matching_slot: {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t success __asm__("$2") = 0;
    register uint32_t active_value __asm__("$4");
    register SequencePlaybackSlot *match_slot __asm__("$3");
    __asm__ volatile("" : "=r"(success) : "0"(success));
#else
    int32_t success = 0;
    uint32_t active_value;
    SequencePlaybackSlot *match_slot;
#endif
    actor->playback_slot_06 = (int8_t)index;
    match_slot = sequence->playback_slots;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(match_slot) : "0"(match_slot) : "memory");
#endif
    active_value = 1;
    match_slot = (SequencePlaybackSlot *)((uintptr_t)slot_offset + (uintptr_t)match_slot);
    match_slot->active_08 = (uint8_t)active_value;
    return success;
}

allocate:
    active_node = actor->playback_node_00;
    if (active_node != 0) {
        return 0;
    }
    actor->playback_node_00 = func_800530FC(actor);
    player_actor_read_sequence_remainder(actor, &key);
    slots = sequence->playback_slots;
    if (slots == 0) {
        return 0;
    }

    selected = -1;
    oldest = 0x7FFFFFFF;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(oldest) : "0"(oldest));
#endif
    shifted_flags = (int32_t)sequence->flags_2C;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(shifted_flags) : "0"(shifted_flags));
#endif
    count = (shifted_flags >> 3) & 31;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(active_node) : "0"(active_node));
#endif
    /* active_node is zero here; the target reuses it as the initial index. */
    has_slots = (int32_t)(uintptr_t)active_node < count;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(has_slots) : "0"(has_slots));
#endif
    index = 0;
    if (has_slots) {
        scan_slots = slots;
        scan_key = key;
        loop_count = count;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(scan_slots), "=r"(scan_key), "=r"(loop_count)
                         : "0"(scan_slots), "1"(scan_key), "2"(loop_count));
#endif
        do {
            SequencePlaybackSlot *slot;
            uint8_t active;
            int32_t timestamp;
            /* A target slot is addressed as (index * 3) * 8. */
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            slot_times_three = (int16_t)index * 3;
            __asm__ volatile("" : "=r"(slot_times_three) : "0"(slot_times_three));
            slot_offset = slot_times_three * 8;
#else
            slot_offset = (int16_t)index * (int32_t)sizeof(*scan_slots);
#endif
            slot = (SequencePlaybackSlot *)((uintptr_t)slot_offset + (uintptr_t)scan_slots);
            /* The target samples the timestamp even for an active slot. */
            active = *(const volatile uint8_t *)&slot->active_08;
            timestamp = *(const volatile int32_t *)&slot->timestamp_0C;
            if (active == 0) {
                if (slot->key_10 == scan_key) {
                    goto matching_slot;
                }
                if (timestamp < oldest) {
                    selected = index;
                    oldest = timestamp;
                }
            }
            next_index = index + 1;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            __asm__ volatile("" : "=r"(next_index) : "0"(next_index));
#endif
            index = next_index;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            __asm__ volatile("" : "=r"(next_index), "=r"(index)
                             : "0"(next_index), "1"(index));
#endif
        } while ((int16_t)next_index < loop_count);
    }

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(selected) : "0"(selected));
#endif
    shifted_selected = (int32_t)((uint32_t)selected << 16);
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(shifted_selected) : "0"(shifted_selected));
#endif
    selected_index = shifted_selected >> 16;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(selected_index) : "0"(selected_index));
#endif
    no_slot = (uint32_t)selected_index > 0x7FFFFFFFu;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(no_slot) : "0"(no_slot));
#endif
    if (no_slot) {
        func_800556D0(0, g_sequence_playback_slot_condition, g_sequence_playback_source_path, 0xB3F);
    }
    actor->playback_slot_06 = (int8_t)selected;
    if (!no_slot) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(no_slot) : "0"(no_slot));
#endif
        sequence->playback_slots[selected_index].active_08 = 1;
        sequence->playback_slots[selected_index].value_14 = 0;
        func_80052E54(actor, key);
    }
    return 0;
}
