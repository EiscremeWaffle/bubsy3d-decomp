#include "component_rotation.h"

extern int32_t func_8001007C(int32_t left, int32_t right, int32_t *output);

int32_t func_8005541C(ComponentRotationMatrix *input_component, const int32_t input_scale[3]) {
    const int32_t *scale = input_scale;
    ComponentRotationMatrix *component = input_component;
    int32_t result;
    int32_t product;

    func_8001007C((int16_t)component->elements[0], scale[0], &result);
    product = result;
    if (product > 0x7FFF) {
        product = 0x7FFF;
    }
    if (product < -0x7FFF) {
        product = -0x7FFF;
    }
    component->elements[0] = (uint16_t)product;

    func_8001007C((int16_t)component->elements[3], scale[0], &result);
    product = result;
    if (product > 0x7FFF) {
        product = 0x7FFF;
    }
    if (product < -0x7FFF) {
        product = -0x7FFF;
    }
    component->elements[3] = (uint16_t)product;

    func_8001007C((int16_t)component->elements[6], scale[0], &result);
    product = result;
    if (product > 0x7FFF) {
        product = 0x7FFF;
    }
    if (product < -0x7FFF) {
        product = -0x7FFF;
    }
    component->elements[6] = (uint16_t)product;

    func_8001007C((int16_t)component->elements[1], scale[1], &result);
    product = result;
    if (product > 0x7FFF) {
        product = 0x7FFF;
    }
    if (product < -0x7FFF) {
        product = -0x7FFF;
    }
    component->elements[1] = (uint16_t)product;

    func_8001007C((int16_t)component->elements[4], scale[1], &result);
    product = result;
    if (product > 0x7FFF) {
        product = 0x7FFF;
    }
    if (product < -0x7FFF) {
        product = -0x7FFF;
    }
    component->elements[4] = (uint16_t)product;

    func_8001007C((int16_t)component->elements[7], scale[1], &result);
    product = result;
    if (product > 0x7FFF) {
        product = 0x7FFF;
    }
    if (product < -0x7FFF) {
        product = -0x7FFF;
    }
    component->elements[7] = (uint16_t)product;

    func_8001007C((int16_t)component->elements[2], scale[2], &result);
    product = result;
    if (product > 0x7FFF) {
        product = 0x7FFF;
    }
    if (product < -0x7FFF) {
        product = -0x7FFF;
    }
    component->elements[2] = (uint16_t)product;

    func_8001007C((int16_t)component->elements[5], scale[2], &result);
    product = result;
    if (product > 0x7FFF) {
        product = 0x7FFF;
    }
    if (product < -0x7FFF) {
        product = -0x7FFF;
    }
    component->elements[5] = (uint16_t)product;

    func_8001007C((int16_t)component->elements[8], scale[2], &result);
    product = result;
    if (product > 0x7FFF) {
        product = 0x7FFF;
    }
    if (product < -0x7FFF) {
        product = -0x7FFF;
    }
    component->elements[8] = (uint16_t)product;
    return 0;
}
