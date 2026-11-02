#ifndef BUBSY3D_MOVE_STEP_H
#define BUBSY3D_MOVE_STEP_H

#include <stdint.h>

typedef struct MoveInfoView {
    uint32_t unused_00;
    int32_t mode_04;
    uint32_t unused_08[4];
    int32_t clamp_18;
    uint32_t unused_1C[5];
    int32_t step_30;
    int32_t step_delta_34;
    int32_t pending_38[3];
    uint32_t unused_44[17];
    int32_t direction_88[3];
} MoveInfoView;

typedef struct ActorView {
    uint8_t state_00;
    uint8_t unused_01[11];
    void *descriptor_0C;
    uint8_t unused_10[12];
    void *component_holder_1C;
    uint8_t unused_20[8];
    MoveInfoView *move_info_28;
} ActorView;

typedef struct ComponentHolderView {
    uint8_t unused_00[0x14];
    void *component_transform_14;
} ComponentHolderView;

void func_80022944(ActorView *actor);

#endif
