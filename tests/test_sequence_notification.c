#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sequence_notification.h"

extern int32_t func_8008E044(int32_t character_index, int32_t state_index);

volatile uint8_t g_sequence_notification_count;
volatile uint16_t g_sequence_notification_index;
SequenceNotificationRecord g_sequence_notification_records[256];
volatile uintptr_t g_character_state_table[8];
_Alignas(uint32_t) static uint8_t character_state[1024];
_Alignas(uint32_t) static uint8_t replacement_state[1024];
static unsigned int calls[256];
static unsigned int call_count;
static int mutate;

void func_80091194(int32_t mode) {
    unsigned int index = g_sequence_notification_index;

    assert(mode == 0);
    assert(call_count < 256);
    calls[call_count++] = index;
    if (mutate == 1 && index == 0) {
        g_sequence_notification_count = 4;
        g_sequence_notification_records[2].selection = 10;
    } else if (mutate == 1 && index == 2) {
        g_sequence_notification_count = 3;
    } else if (mutate == 2) {
        assert(index == 0);
        g_character_state_table[4] = (uintptr_t)replacement_state;
        *(uint32_t *)(character_state + 3u * 172u + 0x7C) = 0x12345678u;
    }
}

int main(void) {
    unsigned int index;

    assert(sizeof(SequenceNotificationRecord) == 52);
    assert(offsetof(SequenceNotificationRecord, selection) == 14);
    assert(offsetof(SequenceNotificationRecord, value_1b) == 27);
    g_sequence_notification_count = 4;
    g_sequence_notification_index = 0xBEEF;
    g_sequence_notification_records[0].selection = -2;
    g_sequence_notification_records[1].selection = 7;
    g_sequence_notification_records[2].selection = -2;
    g_sequence_notification_records[3].selection = -2;
    assert(func_80094108(0x1FFFE) == 4);
    assert(call_count == 3);
    assert(calls[0] == 0 && calls[1] == 2 && calls[2] == 3);
    assert(g_sequence_notification_index == 3);

    call_count = 0;
    g_sequence_notification_count = 0;
    g_sequence_notification_index = 0xBEEF;
    assert(func_80094108(0x1FFFE) == 0);
    assert(call_count == 0);
    assert(g_sequence_notification_index == 0xBEEF);

    mutate = 1;
    g_sequence_notification_count = 2;
    g_sequence_notification_records[0].selection = 10;
    g_sequence_notification_records[1].selection = 11;
    g_sequence_notification_records[2].selection = 12;
    g_sequence_notification_records[3].selection = 10;
    assert(func_80094108(0x1000A) == 3);
    assert(call_count == 2 && calls[0] == 0 && calls[1] == 2);
    assert(g_sequence_notification_index == 2);

    mutate = 0;
    call_count = 0;
    g_sequence_notification_count = 255;
    for (index = 0; index < 255; ++index) {
        g_sequence_notification_records[index].selection = 1;
    }
    assert(func_80094108(1) == 255);
    assert(call_count == 255);
    assert(calls[0] == 0 && calls[254] == 254);
    assert(g_sequence_notification_index == 254);

    memset(character_state, 0xA5, sizeof(character_state));
    memset(replacement_state, 0x5A, sizeof(replacement_state));
    g_character_state_table[4] = (uintptr_t)character_state;
    g_sequence_notification_count = 1;
    g_sequence_notification_records[0].selection = 0x304;
    mutate = 2;
    call_count = 0;
    assert(func_8008E044(4, 3) == 0x7F);
    assert(call_count == 1 && calls[0] == 0);
    assert(*(uint32_t *)(character_state + 3u * 172u + 0x88) == 0x12345678u);
    for (index = 0; index < 16; ++index) {
        assert(character_state[3u * 172u + 0x2C + index] == index);
    }
    for (index = 0; index < sizeof(replacement_state); ++index) {
        assert(replacement_state[index] == 0x5A);
    }

    mutate = 0;
    g_sequence_notification_count = 0;
    for (index = 0; index < 256; ++index) {
        uint32_t flags = 0xA5A50000u | index;

        memset(character_state, 0xA5, sizeof(character_state));
        g_character_state_table[4] = (uintptr_t)character_state;
        *(uint32_t *)(character_state + 3u * 172u + 0x90) = flags;
        assert(func_8008E044(0x10004, 0x10003) == 0x7F);
        assert(*(uint32_t *)(character_state + 3u * 172u + 0x90) ==
               ((flags & ~0x0Bu) | 0x04u));
    }
    return 0;
}
