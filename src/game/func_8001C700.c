#include "sequence_queries.h"

extern int32_t func_8005E7B8(int32_t doubled_value, int32_t signed_value);

int32_t func_8001C700(int32_t index, int32_t signed_value) {
    const int32_t value = g_sequence_values[(int16_t)index];
    const int32_t narrowed_signed_value = (int16_t)signed_value;

    return func_8005E7B8(value, narrowed_signed_value) == 0;
}