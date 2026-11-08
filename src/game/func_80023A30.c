#include "input_events.h"

uint8_t func_80023A30(int32_t interval) {
    uint32_t sample;

    func_80050128(&sample);
    if (sample < g_input_gate_last_sample + (int16_t)interval) {
        return 0;
    }
    g_input_gate_last_sample = sample;
    return 1;
}
