#include "sequence_queries.h"

extern const char g_sequence_query_condition[];
extern const char g_transition_source_path[];
extern int32_t func_8005E9B8(int32_t sequence, int32_t state, uint8_t *output);
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

uint8_t func_8001C7C4(int32_t index, int32_t state) {
    uint8_t result = 0;

    if (func_8005E9B8(g_sequence_values[(int16_t)index], (int16_t)state, &result) != 0) {
        func_800556D0(0, g_sequence_query_condition, g_transition_source_path, 0x5CB);
    }
    return result;
}
