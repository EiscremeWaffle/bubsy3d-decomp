#ifndef BUBSY3D_SEQUENCE_NOTIFICATION_H
#define BUBSY3D_SEQUENCE_NOTIFICATION_H

#include <stdint.h>

typedef struct SequenceNotificationRecord {
    uint16_t value_00;
    uint8_t reserved_02[2];
    uint16_t value_04;
    uint8_t reserved_06[8];
    int16_t selection;
    uint8_t reserved_10[11];
    uint8_t value_1b;
    uint8_t reserved_1c[24];
} SequenceNotificationRecord;

extern volatile uint8_t g_sequence_notification_count;
extern volatile uint16_t g_sequence_notification_index;
extern SequenceNotificationRecord g_sequence_notification_records[];
extern volatile uint16_t g_sequence_set_mask_low;
extern volatile uint16_t g_sequence_set_mask_high;
extern volatile uint16_t g_sequence_clear_mask_low;
extern volatile uint16_t g_sequence_clear_mask_high;

int32_t func_80094108(uint32_t selection);
void func_80091194(int32_t mode);

#endif
