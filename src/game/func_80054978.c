#include "component_rotation.h"

extern void func_80054488(
    const ComponentRotationMatrix *input, ComponentRotationMatrix *output);

int32_t func_80054978(
    ComponentRotationMatrix *component, const int32_t rotation[3]) {
    ComponentRotationMatrix rotated_basis;
    uint16_t angles[3];

    if (component == 0) {
        return 0x18;
    }

    angles[0] = (uint16_t)(0u - (uint16_t)rotation[0]);
    angles[1] = (uint16_t)rotation[1];
    angles[2] = (uint16_t)(0u - (uint16_t)rotation[2]);
    func_80077C50(angles, &rotated_basis);
    func_80054488(&rotated_basis, component);
    return 0;
}