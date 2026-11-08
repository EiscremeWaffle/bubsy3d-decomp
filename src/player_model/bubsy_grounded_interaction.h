#ifndef BUBSY3D_GROUNDED_INTERACTION_H
#define BUBSY3D_GROUNDED_INTERACTION_H

#include "bubsy_grounded_state.h"
#include "level_records.h"

typedef struct BubsyInteractionState {
    int32_t target_index;
    uint8_t unknown_04;
    uint8_t phase_05;
} BubsyInteractionState;

typedef struct BubsyInteractionComponent {
    uint8_t unknown_00[0x14];
    void *transform_14;
} BubsyInteractionComponent;

typedef struct BubsyInteractionTarget {
    uint8_t unknown_00[0x0C];
    int32_t heading_0C;
    uint8_t unknown_10[0x0C];
    BubsyInteractionComponent *component_1C;
    uint8_t unknown_20[0x0C];
    uint8_t used_2C;
} BubsyInteractionTarget;

typedef GameLevelRecord BubsyInteractionRecord;

void func_80039158(BubsyGroundedActor *actor, void *context);

#endif
