#include "gte_basis_transform.h"
#include "hierarchy_transform.h"
#include "matrix_transpose.h"
#include "transform_composition.h"
#include "transform_reset_backend.h"

int32_t func_800731C0(TransformResetDescriptor *descriptor) {
    TransformResetMatrix hierarchy, inverse;
    int32_t transformed[3];
    const volatile uint32_t *source;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    TransformResetMatrix *working = &g_transform_working_matrix;
    register volatile uint32_t *output __asm__("$5");
    register uint32_t first __asm__("$2");
    register uint32_t second __asm__("$3");
    register uint32_t third __asm__("$4");
#else
    TransformResetMatrix *working = &g_transform_working_matrix;
    volatile uint32_t *output;
    uint32_t first, second, third;
#endif
    HierarchyTransformNode *binding;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(working) : "0"(working) : "memory");
#endif
    component_record_copy(&working->record, &descriptor->matrix_00.record);
    binding = descriptor->binding_20;
    if (binding == 0) {
        return 0;
    }

    func_80073324(binding, &hierarchy);
    func_80077C00(hierarchy.rotation.elements, inverse.rotation.elements);
    func_80077298(&inverse.rotation, hierarchy.rotation.translation, transformed);
    inverse.rotation.translation[0] = (int32_t)(0U - (uint32_t)transformed[0]);
    inverse.rotation.translation[1] = (int32_t)(0U - (uint32_t)transformed[1]);
    inverse.rotation.translation[2] = (int32_t)(0U - (uint32_t)transformed[2]);
    func_8007242C(&working->rotation, &inverse.rotation);
    component_record_copy(&working->record, &inverse.record);

    source = working->words;
    output = g_transform_render_matrix.words;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(output) : "0"(output));
#endif
    first = source[0];
    second = source[1];
    third = source[2];
    output[0] = first;
    output[1] = second;
    output[2] = third;
    first = source[3];
    second = source[4];
    third = source[5];
    output[3] = first;
    output[4] = second;
    output[5] = third;
    first = source[6];
    second = source[7];
    output[6] = first;
    output[7] = second;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : : : "memory");
#endif
    return 0;
}
