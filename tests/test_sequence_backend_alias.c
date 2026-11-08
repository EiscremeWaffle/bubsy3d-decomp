#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sequence_notification.h"

SequenceNotificationRecord g_sequence_notification_records[692];

/* Preserve the original offsets where these globals overlap record storage. */
__asm__(
    ".globl g_sequence_notification_index\n"
    ".set g_sequence_notification_index, g_sequence_notification_records+0x60A6\n"
    ".globl g_sequence_set_mask_low\n"
    ".set g_sequence_set_mask_low, g_sequence_notification_records+0x8C58\n"
    ".globl g_sequence_set_mask_high\n"
    ".set g_sequence_set_mask_high, g_sequence_notification_records+0x8C60\n"
    ".globl g_sequence_clear_mask_low\n"
    ".set g_sequence_clear_mask_low, g_sequence_notification_records+0x05BC\n"
    ".globl g_sequence_clear_mask_high\n"
    ".set g_sequence_clear_mask_high, g_sequence_notification_records+0x05C4\n"
);

int main(void) {
    volatile SequenceNotificationRecord *row = &g_sequence_notification_records[691];

    assert(sizeof(SequenceNotificationRecord) == 52);
    assert(offsetof(SequenceNotificationRecord, value_04) == 4);
    memset(g_sequence_notification_records, 0xA5, sizeof(g_sequence_notification_records));
    g_sequence_notification_index = 691;
    g_sequence_set_mask_low = 0x8001;
    g_sequence_set_mask_high = 0x7777;
    g_sequence_clear_mask_low = 0xFFFF;
    g_sequence_clear_mask_high = 0xFFFF;
    assert(row->value_04 == 0x7777);

    func_80091194(77);

    assert(row->value_00 == 0);
    assert(row->value_04 == 8);
    assert(row->value_1b == 0);
    assert(row->reserved_02[0] == 0xA5);
    assert(g_sequence_set_mask_low == 0x8001);
    assert(g_sequence_set_mask_high == 8);
    assert(g_sequence_clear_mask_low == 0x7FFE);
    assert(g_sequence_clear_mask_high == 0xFFF7);
    return 0;
}
