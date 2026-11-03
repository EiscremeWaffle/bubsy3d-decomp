#include "component_rotation.h"

int32_t func_800549E8(ComponentRotationMatrix *component, const ComponentRotationMatrix *basis) {
    uint16_t *output = (uint16_t *)component;
    const uint16_t *input = (const uint16_t *)basis;

    output[0] = input[0];
    output[1] = (uint16_t)(0u - input[1]);
    output[2] = input[2];
    output[3] = (uint16_t)(0u - input[3]);
    output[4] = input[4];
    output[5] = (uint16_t)(0u - input[5]);
    output[6] = input[6];
    output[7] = (uint16_t)(0u - input[7]);
    output[8] = input[8];
    return 0;
}
