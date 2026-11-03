#include "component_rotation.h"

extern void func_80077C50(const uint16_t angles[3], ComponentRotationMatrix *output);
extern void func_80076890(ComponentRotationMatrix *rotation, const ComponentRotationMatrix *basis);
extern void func_80054488(const ComponentRotationMatrix *input, ComponentRotationMatrix *output);

int32_t func_800547C8(ComponentRotationMatrix *component, const int32_t rotation[3]) {
    ComponentRotationMatrix saved_basis;
    ComponentRotationMatrix rotated_basis;
    ComponentRotationMatrix converted_basis;
    uint16_t angles[3];

    if (component == 0) {
        return 0x18;
    }
    angles[0] = (uint16_t)(0u - (uint16_t)rotation[0]);
    angles[1] = (uint16_t)rotation[1];
    angles[2] = (uint16_t)(0u - (uint16_t)rotation[2]);
    saved_basis = *component;
    func_80077C50(angles, &rotated_basis);
    func_80076890(&rotated_basis, &saved_basis);
    func_800542A4(&rotated_basis, &converted_basis);
    func_80054488(&converted_basis, component);
    return 0;
}
