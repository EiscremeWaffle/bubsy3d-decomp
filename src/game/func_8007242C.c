#include "gte_basis_transform.h"
#include "transform_composition.h"

void func_8007242C(const ComponentRotationMatrix *first, ComponentRotationMatrix *second) {
    int32_t transformed[3];
    const volatile ComponentRotationMatrix *live_first = first;
    volatile ComponentRotationMatrix *live_second = second;

    func_80077298(first, second->translation, transformed);
    func_8007699C(first, second);
    live_second->translation[0] = (int32_t)(
        (uint32_t)transformed[0] + (uint32_t)live_first->translation[0]);
    live_second->translation[1] = (int32_t)(
        (uint32_t)transformed[1] + (uint32_t)live_first->translation[1]);
    live_second->translation[2] = (int32_t)(
        (uint32_t)transformed[2] + (uint32_t)live_first->translation[2]);
}
