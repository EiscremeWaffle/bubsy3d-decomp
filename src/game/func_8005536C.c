#include "component_rotation.h"

extern const ComponentRotationMatrix g_component_identity_basis;
extern int32_t func_80054C7C(
    ComponentRotationMatrix *component,
    const ComponentRotationMatrix *basis,
    const int32_t translation[3]
);

int32_t func_8005536C(
    ComponentRotationMatrix *component,
    const ComponentRotationMatrix *basis,
    const int32_t translation[3]
) {
    ComponentRotationMatrix target = g_component_identity_basis;

    if (component == 0) {
        return 0x18;
    }
    if (translation == 0) {
        return 1;
    }
    func_80054C7C(&target, basis, translation);
    func_800550A4(component, &target);
    return 0;
}
