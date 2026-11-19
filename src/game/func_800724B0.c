#include "gte_basis_transform.h"
#include "transform_composition.h"

void func_800724B0(ComponentRotationMatrix *first, const ComponentRotationMatrix *second) {
    int32_t transformed[3];
    volatile ComponentRotationMatrix *live_first = first;

    func_80077298(first, second->translation, transformed);
    func_80076890(first, second);
    live_first->translation[0] = (int32_t)(
        (uint32_t)transformed[0] + (uint32_t)live_first->translation[0]);
    live_first->translation[1] = (int32_t)(
        (uint32_t)transformed[1] + (uint32_t)live_first->translation[1]);
    live_first->translation[2] = (int32_t)(
        (uint32_t)transformed[2] + (uint32_t)live_first->translation[2]);
}
