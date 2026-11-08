#include "sequence_notification.h"

#if defined(__mips__) || defined(BUBSY3D_MATCH_ORIGINAL_L0)

typedef struct {
    uint8_t value;
    uint8_t padding[51];
} SequenceByteField;

typedef struct {
    uint16_t value;
    uint8_t padding[50];
} SequenceHalfwordField;

extern volatile SequenceByteField g_sequence_record_byte[];
extern volatile SequenceHalfwordField g_sequence_record_half_a[];
extern volatile SequenceHalfwordField g_sequence_record_half_b[];

void func_80091194(int32_t mode) {
    register uint32_t index __asm__("$4") = g_sequence_notification_index;
    register uint32_t slot __asm__("$3");
    register uint32_t low_bit __asm__("$7");
    register uint32_t high_bit __asm__("$6");
    register uint32_t one __asm__("$2");
    register uint32_t set_low __asm__("$4");
    register uint32_t set_high __asm__("$5");
    register uint32_t clear_low __asm__("$2");
    register uint32_t clear_high __asm__("$3");

    (void)mode;
    __asm__ volatile("" : "=r"(index) : "r"(index));
    __asm__ volatile("andi %0,%1,0xFFFF" : "=r"(slot) : "r"(index));
    if (slot < 16) {
        one = 1;
        low_bit = one << slot;
        high_bit = 0;
    } else {
        one = 1;
        low_bit = 0;
        slot -= 16;
        __asm__ volatile("sllv %0,%1,%2" : "=r"(high_bit) : "r"(one), "r"(slot));
        __asm__ volatile("andi %0,%1,0xFFFF" : "=r"(slot) : "r"(index));
    }

    __asm__ volatile("" : "=r"(slot) : "0"(slot), "r"(low_bit), "r"(high_bit));
    g_sequence_record_byte[slot].value = 0;
    g_sequence_record_half_a[slot].value = 0;
    g_sequence_record_half_b[slot].value = 0;

    set_low = g_sequence_set_mask_low;
    set_high = g_sequence_set_mask_high;
    clear_low = g_sequence_clear_mask_low;
    clear_high = g_sequence_clear_mask_high;
    __asm__ volatile("or %0,%1,%2" : "=r"(set_low) : "r"(low_bit), "r"(set_low));
    g_sequence_set_mask_low = set_low;
    __asm__ volatile("" : "=r"(set_high) : "0"(set_high) : "memory");
    set_high = high_bit | set_high;
    g_sequence_set_mask_high = set_high;
    __asm__ volatile("" : "=r"(clear_low) : "0"(clear_low) : "memory");
    clear_low = clear_low & ~set_low;
    __asm__ volatile("" : "=r"(clear_low) : "0"(clear_low));
    g_sequence_clear_mask_low = clear_low;
    __asm__ volatile("" : "=r"(clear_high) : "0"(clear_high) : "memory");
    clear_high = clear_high & ~set_high;
    __asm__ volatile("" : "=r"(clear_high) : "0"(clear_high));
    g_sequence_clear_mask_high = clear_high;
}

#else

/* Behavioral model retained for host-side tests. */
void func_80091194(int32_t mode) {
    uint32_t index = g_sequence_notification_index;
    volatile SequenceNotificationRecord *record = &g_sequence_notification_records[index];
    uint32_t low_bit;
    uint32_t high_bit;
    uint32_t set_low;
    uint32_t set_high;
    uint32_t clear_low;
    uint32_t clear_high;

    (void)mode;
    if (index < 16) {
        low_bit = 1u << index;
        high_bit = 0;
    } else {
        low_bit = 0;
        /* MIPS variable shifts use only the low five bits of the shift count. */
        high_bit = 1u << ((index - 16) & 31);
    }

    record->value_1b = 0;
    record->value_04 = 0;
    record->value_00 = 0;

    set_low = g_sequence_set_mask_low | low_bit;
    set_high = g_sequence_set_mask_high | high_bit;
    clear_low = g_sequence_clear_mask_low;
    clear_high = g_sequence_clear_mask_high;
    g_sequence_set_mask_low = set_low;
    g_sequence_set_mask_high = set_high;
    g_sequence_clear_mask_low = clear_low & ~set_low;
    g_sequence_clear_mask_high = clear_high & ~set_high;
}

#endif
