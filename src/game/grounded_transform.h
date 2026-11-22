#ifndef BUBSY3D_GROUNDED_TRANSFORM_H
#define BUBSY3D_GROUNDED_TRANSFORM_H

#include <stdint.h>

typedef union GroundedTransformVector {
    struct {
        uint32_t x, y, z;
    } coordinates;
    int32_t values[3];
} GroundedTransformVector;

extern void *g_unknown_800BE528;

void func_8002E858(
    const GroundedTransformVector *direction, int32_t distance,
    uint32_t height, GroundedTransformVector *output
);
void func_8002E954(GroundedTransformVector *translation, uint32_t offset);
void func_8002EA80(void);

#endif
