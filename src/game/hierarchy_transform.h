#ifndef BUBSY3D_HIERARCHY_TRANSFORM_H
#define BUBSY3D_HIERARCHY_TRANSFORM_H

#include "transform_reset_backend.h"

typedef struct HierarchyTransformNode {
    uint32_t generation_00;
    TransformResetMatrix local_04;
    TransformResetMatrix cached_24;
    uint32_t unknown_44;
    struct HierarchyTransformNode *parent_48;
} HierarchyTransformNode;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(HierarchyTransformNode, local_04) == 0x04, "hierarchy local matrix");
_Static_assert(offsetof(HierarchyTransformNode, cached_24) == 0x24, "hierarchy cached matrix");
_Static_assert(offsetof(HierarchyTransformNode, parent_48) == 0x48, "hierarchy parent");
#endif

extern uint32_t g_hierarchy_generation;
extern HierarchyTransformNode *g_hierarchy_chain[];

void func_80073324(HierarchyTransformNode *binding, TransformResetMatrix *output);

#endif
