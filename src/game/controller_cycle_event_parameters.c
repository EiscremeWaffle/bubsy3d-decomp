#include <stdint.h>

extern uint32_t g_event_parameters[];

void func_80044C04(int32_t index, uint32_t value) {
    g_event_parameters[index] = value;
}

uint32_t func_80044C1C(int32_t index) {
    return g_event_parameters[index];
}
