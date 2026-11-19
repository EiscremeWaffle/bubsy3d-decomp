#include "hierarchy_transform.h"
#include "transform_composition.h"

void func_80073324(HierarchyTransformNode *binding, TransformResetMatrix *output) {
    HierarchyTransformNode *node = binding;
    int32_t depth = 0;
    int32_t last_uncached = 100;
    for (;;) {
        HierarchyTransformNode *parent;

        g_hierarchy_chain[depth] = node;
        parent = node->parent_48;
        if (parent == 0) {
            if (node->generation_00 == g_hierarchy_generation || node->generation_00 == 0) {
                component_record_copy(&node->cached_24.record, &node->local_04.record);
                component_record_copy(&output->record, &node->cached_24.record);
                node->generation_00 = g_hierarchy_generation;
            } else if (last_uncached == 100) {
                component_record_copy(&output->record, &g_hierarchy_chain[0]->cached_24.record);
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                __asm__ volatile("" : : : "memory");
#endif
                depth = 0;
            } else {
                depth = last_uncached + 1;
                component_record_copy(&output->record, &g_hierarchy_chain[depth]->cached_24.record);
            }
            break;
        }
        if (node->generation_00 == g_hierarchy_generation) {
            component_record_copy(&output->record, &node->cached_24.record);
            break;
        }
        if (node->generation_00 == 0) {
            last_uncached = depth;
        }
        node = parent;
        ++depth;
    }

    if (depth > 0) {
        /* The original decrements the cursor once beyond the first slot. */
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register uintptr_t offset __asm__("$2") = (uintptr_t)depth * sizeof(*g_hierarchy_chain);
        uintptr_t cursor;

        __asm__ volatile("" : "=r"(offset) : "0"(offset));
        cursor = (uintptr_t)g_hierarchy_chain - sizeof(*g_hierarchy_chain) + offset;
#else
        uintptr_t cursor = (uintptr_t)g_hierarchy_chain + (uintptr_t)depth * sizeof(*g_hierarchy_chain)
            - sizeof(*g_hierarchy_chain);
#endif

        do {
            HierarchyTransformNode **slot = (HierarchyTransformNode **)cursor;

            func_800724B0(&output->rotation, &(*slot)->local_04.rotation);
            --depth;
            component_record_copy(&(*slot)->cached_24.record, &output->record);
            (*slot)->generation_00 = g_hierarchy_generation;
            cursor -= sizeof(*g_hierarchy_chain);
        } while (depth > 0);
    }
}
