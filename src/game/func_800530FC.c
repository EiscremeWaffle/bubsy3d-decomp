#include "sequence_playback.h"

extern void func_800556D0(
    int32_t failed, const char *condition, const char *source, uint32_t line
);

SequencePlaybackNode *func_800530FC(void *owner) {
    SequencePlaybackNode *node = g_sequence_playback_free;
    SequencePlaybackNode *active;
    SequencePlaybackNode *next_free;
    uint16_t count;

    if (node == 0) {
        func_800556D0(1, g_sequence_playback_free_condition, g_sequence_playback_source_path, 0xC68);
    }

    active = g_sequence_playback_active;
    count = g_sequence_playback_count;
    next_free = sequence_playback_address(node)->next_08;
    g_sequence_playback_active = node;
    g_sequence_playback_free = next_free;
    sequence_playback_address(next_free)->previous_04 = 0;
    sequence_playback_address(node)->owner_00 = owner;
    sequence_playback_address(node)->next_08 = active;
    sequence_playback_address(node)->previous_04 = 0;
    sequence_playback_address(active)->previous_04 = node;
    g_sequence_playback_count = (uint16_t)(count + 1U);
    return node;
}
