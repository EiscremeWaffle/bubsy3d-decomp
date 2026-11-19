#ifndef BUBSY3D_TRANSFORM_RESET_BACKEND_H
#define BUBSY3D_TRANSFORM_RESET_BACKEND_H

#include "component_records.h"
#include "component_rotation.h"

typedef union TransformResetMatrix {
    ComponentRotationMatrix rotation;
    ComponentRecord record;
    uint32_t words[8];
} TransformResetMatrix;

struct HierarchyTransformNode;

typedef struct TransformResetDescriptor {
    TransformResetMatrix matrix_00;
    struct HierarchyTransformNode *binding_20;
} TransformResetDescriptor;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(TransformResetMatrix) == 0x20, "transform reset matrix size");
_Static_assert(offsetof(TransformResetDescriptor, binding_20) == 0x20, "transform reset binding offset");
#endif

extern TransformResetMatrix g_transform_working_matrix;
extern TransformResetMatrix g_transform_render_matrix;

int32_t func_800731C0(TransformResetDescriptor *descriptor);

#endif
