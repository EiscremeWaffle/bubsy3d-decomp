#include "component_rotation.h"

int32_t func_80054A6C(
    ComponentRotationMatrix *component,
    const ComponentRotationMatrix *basis
) {
    func_800549E8(component, basis);
    component->translation[0] = basis->translation[0];
    component->translation[1] = (int32_t)(0u - (uint32_t)basis->translation[1]);
    component->translation[2] = basis->translation[2];
    return 0;
}
