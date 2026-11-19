#ifndef BUBSY3D_CYCLIC_ENTRIES_H
#define BUBSY3D_CYCLIC_ENTRIES_H

#include <stdint.h>

typedef struct GameCyclicEntry {
    uint32_t words[4];
} GameCyclicEntry;

extern GameCyclicEntry g_cyclic_entries[7];

int32_t func_8001A280(volatile int32_t *index, GameCyclicEntry *output);

#endif
