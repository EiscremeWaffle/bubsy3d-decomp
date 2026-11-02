#ifndef BUBSY3D_GROUNDED_STATE_H
#define BUBSY3D_GROUNDED_STATE_H

#include <stdint.h>

typedef struct BubsyGroundedDescriptor {
    int16_t state_00;
    int16_t state_02;
    uint8_t unknown_04[0x0C];
    uint8_t state_10;
    uint8_t state_11;
    uint8_t unknown_12[2];
    uint8_t state_14;
    uint8_t unknown_15[0x37];
    uint8_t state_4C;
    uint8_t counter_4D;
    uint8_t unknown_4E[0x1A];
    uint8_t state_68;
} BubsyGroundedDescriptor;

typedef struct BubsyGroundedShared {
    uint8_t unknown_00[0x30];
    int32_t stride_30;
    int32_t stride_delta_34;
} BubsyGroundedShared;

typedef struct BubsyGroundedActor {
    uint8_t state_00;
    uint8_t state_01;
    uint8_t unknown_02[2];
    uint32_t flags_04;
    uint32_t unknown_08;
    uint32_t heading_0C;
    uint32_t state_10;
    uint32_t state_14;
    uint32_t unknown_18;
    void *component_1C;
    uint8_t unknown_20[8];
    BubsyGroundedShared *shared_28;
} BubsyGroundedActor;

typedef struct BubsyGroundedState {
    uint16_t id;
} BubsyGroundedState;

typedef union BubsyGroundedLookup {
    int32_t words[6];
    struct {
        uint8_t unknown_00[0x0D];
        uint8_t gate_0D;
        uint8_t unknown_0E[6];
        int16_t index_14;
    } fields;
} BubsyGroundedLookup;

void bubsy_grounded_state_handler(
    BubsyGroundedState *state,
    BubsyGroundedActor *actor,
    void *context,
    BubsyGroundedLookup *lookup,
    BubsyGroundedShared *shared
);

void func_80035ACC(BubsyGroundedActor *actor, void *transform);

#endif
