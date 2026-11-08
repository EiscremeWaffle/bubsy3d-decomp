#include "sequence_queries.h"

extern int32_t func_8005E5C0(int32_t sequence, int32_t state, int32_t value);

int32_t func_8001C6BC(int32_t index, int32_t state, int32_t value) {
    return func_8005E5C0(g_sequence_values[(int16_t)index],
        (int16_t)state, (int16_t)value) == 0;
}
