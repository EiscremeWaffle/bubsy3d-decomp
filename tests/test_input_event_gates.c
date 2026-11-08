#include "input_events.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

uint8_t g_event_runtime_byte;
uint8_t g_event_input_gate;
uint32_t g_event_input_gate_reference;
uint32_t g_input_sample_counter;
uint32_t g_input_gate_last_sample;
uint8_t g_unknown_800BE1C2;

int main(void) {
    static const uint32_t samples[] = {0, 1, 0x7FFFFFFFu, 0x80000000u, UINT32_MAX};
    static const int32_t intervals[] = {
        INT32_MIN, -65537, -32769, -32768, -1, 0, 1, 32767, 32768, 65535, INT32_MAX
    };
    uint32_t output;
    unsigned int byte;
    unsigned int i;
    unsigned int j;
    unsigned int k;

    for (byte = 0; byte < 256; ++byte) {
        g_event_runtime_byte = (uint8_t)byte;
        g_event_input_gate = (uint8_t)byte;
        g_unknown_800BE1C2 = (uint8_t)byte;
        assert(func_8001BCE4() == (uint8_t)byte);
        assert(func_8001BCF0() == (uint8_t)byte);
        assert(func_80045278() == (int32_t)byte);
    }
    for (i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        g_event_input_gate_reference = samples[i];
        g_input_sample_counter = samples[i];
        output = 0;
        assert(func_8001BD00() == samples[i]);
        assert(func_80050128(&output) == 0 && output == samples[i]);
        assert(func_80050128(&g_input_sample_counter) == 0);
        assert(g_input_sample_counter == samples[i]);
    }

    for (i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        for (j = 0; j < sizeof(samples) / sizeof(samples[0]); ++j) {
            for (k = 0; k < sizeof(intervals) / sizeof(intervals[0]); ++k) {
                uint32_t threshold = samples[i] + (uint32_t)(int32_t)(int16_t)intervals[k];
                uint8_t expected = samples[j] >= threshold;
                g_input_gate_last_sample = samples[i];
                g_input_sample_counter = samples[j];
                assert(func_80023A30(intervals[k]) == expected);
                assert(g_input_gate_last_sample == (expected ? samples[j] : samples[i]));
            }
        }
    }
    puts("Input event gates: all byte values, counters, signed intervals and wrapping thresholds passed.");
    return 0;
}
