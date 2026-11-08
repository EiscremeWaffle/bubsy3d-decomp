#include "game_transition.h"

extern uint32_t g_event_parameters[];

void func_80044C34(const uint32_t *parameters) {
    int32_t i;

    for (i = 0; i < 8; ++i) {
        g_event_parameters[i] = parameters[i];
    }
}
