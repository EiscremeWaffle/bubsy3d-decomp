#ifndef BUBSY3D_SEQUENCE_LIMIT_H
#define BUBSY3D_SEQUENCE_LIMIT_H

#include <stdint.h>

typedef struct SequenceLimitEntry {
    int16_t key;
    int16_t limit;
    uint8_t padding[68];
} SequenceLimitEntry;

extern uint8_t g_sequence_limit_scan_enabled;
extern int16_t g_sequence_limit_count;
extern SequenceLimitEntry g_sequence_limit_entries[];

/* A failed scan also inspects the entry at the stopping index. */
int32_t func_8005D5A8(int32_t requested_key, int32_t value);
int32_t func_8005E708(int32_t first_value, int32_t second_value);
int32_t func_8005E760(int32_t first_value, int32_t second_value);
int32_t func_8005E7B8(int32_t first_value, int32_t second_value);

#endif
