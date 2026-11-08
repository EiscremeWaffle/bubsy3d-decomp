#include "sequence_notification.h"

/* Behavioral candidate; not yet byte-matched. */
int32_t func_80094108(uint32_t selection) {
    uint32_t index = 0;
    uint8_t count = g_sequence_notification_count;

    if (count != 0) {
        int16_t key = (int16_t)selection;
        do {
            if (g_sequence_notification_records[(uint8_t)index].selection == key) {
                g_sequence_notification_index = (uint8_t)index;
                func_80091194(0);
            }
            ++index;
        } while ((uint8_t)index < g_sequence_notification_count);
    }
    return (uint8_t)index;
}
