#include <stdint.h>

extern uint32_t g_runtime_sample_counter;

void func_8001AE88(uint32_t *output) {
    *output = g_runtime_sample_counter;
}
