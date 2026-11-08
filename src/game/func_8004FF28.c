#include "controller_input.h"

int32_t func_8004FF28(int32_t sample) {
    g_input_stream_sample = sample;
    return 0;
}
