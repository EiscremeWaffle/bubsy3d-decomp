#ifndef BUBSY3D_GROUNDED_TURN_H
#define BUBSY3D_GROUNDED_TURN_H

#include "bubsy_grounded_state.h"

typedef struct BubsyGroundedTurnShared {
    uint8_t unknown_00[0x4C];
    int32_t turn_4C;
} BubsyGroundedTurnShared;

void func_80034EF8(
    BubsyGroundedActor *actor,
    BubsyGroundedState *state,
    void *context,
    int32_t counter
);

#endif
