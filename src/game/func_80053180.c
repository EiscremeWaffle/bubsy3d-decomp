#include "sequence_playback.h"

int32_t func_80053180(SequencePlaybackNode *node) {
    SequencePlaybackNode *previous = sequence_playback_address(node)->previous_04;
    SequencePlaybackNode *next = sequence_playback_address(node)->next_08;
    SequencePlaybackNode *free_head;
    int32_t count;

    if (previous != 0) {
        previous->next_08 = next;
    } else {
        g_sequence_playback_active = next;
    }
    if (next != 0) {
        next->previous_04 = sequence_playback_address(node)->previous_04;
    }

    free_head = g_sequence_playback_free;
    count = (int32_t)g_sequence_playback_count - 1;
    g_sequence_playback_free = node;
    sequence_playback_address(free_head)->previous_04 = node;
    sequence_playback_address(node)->previous_04 = 0;
    sequence_playback_address(node)->next_08 = free_head;
    g_sequence_playback_count = (uint16_t)count;
    return count;
}
