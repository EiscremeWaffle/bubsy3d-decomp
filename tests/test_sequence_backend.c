#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sequence_notification.h"

extern int32_t func_8008E044(int32_t character_index, int32_t state_index);

volatile uint8_t g_sequence_notification_count;
volatile uint16_t g_sequence_notification_index;
SequenceNotificationRecord g_sequence_notification_records[256];
volatile uint16_t g_sequence_set_mask_low;
volatile uint16_t g_sequence_set_mask_high;
volatile uint16_t g_sequence_clear_mask_low;
volatile uint16_t g_sequence_clear_mask_high;
volatile uintptr_t g_character_state_table[8];
_Alignas(uint32_t) static uint8_t character_state[1024];

int main(void) {
    unsigned int index;

    assert(sizeof(SequenceNotificationRecord) == 52);
    assert(offsetof(SequenceNotificationRecord, selection) == 14);
    assert(offsetof(SequenceNotificationRecord, value_1b) == 27);

    for (index = 0; index < 256; ++index) {
        SequenceNotificationRecord expected;
        uint32_t low_bit = index < 16 ? 1u << index : 0;
        uint32_t high_bit = index >= 16 ? 1u << ((index - 16) & 31) : 0;
        uint16_t set_low = (uint16_t)(0xA140u | low_bit);
        uint16_t set_high = (uint16_t)(0x8021u | high_bit);

        memset(&g_sequence_notification_records[index], 0xA5, sizeof(expected));
        expected = g_sequence_notification_records[index];
        expected.value_00 = 0;
        expected.value_04 = 0;
        expected.value_1b = 0;
        g_sequence_notification_index = (uint16_t)index;
        g_sequence_set_mask_low = 0xA140;
        g_sequence_set_mask_high = 0x8021;
        g_sequence_clear_mask_low = 0xF5AF;
        g_sequence_clear_mask_high = 0x7FAA;
        func_80091194((int32_t)(index % 3) - 1);
        assert(memcmp(&g_sequence_notification_records[index], &expected, sizeof(expected)) == 0);
        assert(g_sequence_set_mask_low == set_low);
        assert(g_sequence_set_mask_high == set_high);
        assert(g_sequence_clear_mask_low == (uint16_t)(0xF5AFu & ~set_low));
        assert(g_sequence_clear_mask_high == (uint16_t)(0x7FAAu & ~set_high));
    }

    memset(g_sequence_notification_records, 0xA5, 3 * sizeof(SequenceNotificationRecord));
    memset(character_state, 0xA5, sizeof(character_state));
    g_character_state_table[4] = (uintptr_t)character_state;
    g_sequence_notification_records[0].selection = 0x304;
    g_sequence_notification_records[1].selection = 9;
    g_sequence_notification_records[2].selection = 0x304;
    g_sequence_notification_count = 3;
    g_sequence_set_mask_low = 0x40;
    g_sequence_set_mask_high = 0x03;
    g_sequence_clear_mask_low = 0xFF;
    g_sequence_clear_mask_high = 0xF5;
    assert(func_8008E044(4, 3) == 0x7F);
    assert(g_sequence_notification_index == 2);
    assert(g_sequence_set_mask_low == 0x45);
    assert(g_sequence_set_mask_high == 0x03);
    assert(g_sequence_clear_mask_low == 0xBA);
    assert(g_sequence_clear_mask_high == 0xF4);
    for (index = 0; index < 3; index += 2) {
        assert(g_sequence_notification_records[index].value_00 == 0);
        assert(g_sequence_notification_records[index].value_04 == 0);
        assert(g_sequence_notification_records[index].value_1b == 0);
    }
    assert(g_sequence_notification_records[1].value_00 == 0xA5A5);
    for (index = 0; index < 16; ++index) {
        assert(character_state[3 * 172 + 0x2C + index] == index);
    }

    memset(g_sequence_notification_records, 0xA5, sizeof(g_sequence_notification_records));
    for (index = 0; index < 255; ++index) {
        g_sequence_notification_records[index].selection = 1;
    }
    g_sequence_notification_count = 255;
    g_sequence_set_mask_low = 0;
    g_sequence_set_mask_high = 0;
    g_sequence_clear_mask_low = 0xFFFF;
    g_sequence_clear_mask_high = 0xFFFF;
    assert(func_80094108(1) == 255);
    assert(g_sequence_notification_index == 254);
    assert(g_sequence_set_mask_low == 0xFFFF);
    assert(g_sequence_set_mask_high == 0xFFFF);
    assert(g_sequence_clear_mask_low == 0);
    assert(g_sequence_clear_mask_high == 0);
    for (index = 0; index < 255; ++index) {
        assert(g_sequence_notification_records[index].value_00 == 0);
        assert(g_sequence_notification_records[index].value_04 == 0);
        assert(g_sequence_notification_records[index].value_1b == 0);
    }
    assert(g_sequence_notification_records[255].value_00 == 0xA5A5);
    return 0;
}
