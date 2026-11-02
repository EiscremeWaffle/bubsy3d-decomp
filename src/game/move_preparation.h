#ifndef BUBSY3D_MOVE_PREPARATION_H
#define BUBSY3D_MOVE_PREPARATION_H

#include <stdint.h>

typedef struct MovePreparationVector {
    int32_t x, y, z, padding;
} MovePreparationVector;

typedef struct MovePreparationBasis {
    int16_t matrix[9];
    int16_t padding;
    int32_t translation[3];
} MovePreparationBasis;

typedef struct MovePreparationInfo {
    int32_t counter;
    int32_t index;
    MovePreparationVector target;
    int32_t separation_18;
    uint8_t mode_1C;
    uint8_t unknown_1D[0x13];
    int32_t stride_30;
    int32_t unknown_34;
    MovePreparationVector pending_38;
    MovePreparationVector rotation_48;
    MovePreparationVector origin_58;
    uint8_t unknown_68[0x20];
    MovePreparationVector transform_88;
    uint8_t unknown_98[0x0C];
} MovePreparationInfo;

typedef struct MovePreparationComponent {
    uint8_t unknown_00[0x14];
    void *transform_14;
} MovePreparationComponent;

typedef struct MovePreparationActor {
    uint8_t unknown_00[0x0C];
    int32_t context_0C;
    uint8_t unknown_10[0x0C];
    MovePreparationComponent *component_1C;
    uint8_t unknown_20[8];
    MovePreparationInfo *move_info_28;
} MovePreparationActor;

typedef struct MovePreparationEntry {
    uint8_t tag;
    uint8_t selector;
    uint16_t reserved;
    int32_t limit;
    const MovePreparationVector *payload;
} MovePreparationEntry;

int32_t func_80025A98(MovePreparationActor *actor, const void *basis_input,
    uint32_t operation_flags, uint8_t *completion);

#endif
