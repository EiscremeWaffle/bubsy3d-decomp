#include "cyclic_entries.h"

int32_t func_8001A280(volatile int32_t *index, GameCyclicEntry *output) {
    if (*index >= 7 || *index < 0) {
        return 0;
    }

    *output = g_cyclic_entries[*index];
    if ((int32_t)((uint32_t)*index + 1U) >= 7) {
        *index = 0;
    } else {
        *index = (int32_t)((uint32_t)*index + 1U);
    }
    return 1;
}
