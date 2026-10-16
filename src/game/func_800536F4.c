#include <stdint.h>

int32_t func_800536F4(const int32_t *input, int32_t *output) {
    int32_t value = input[0];
    output[0] = value >= 0 ? value >> 12 : -((int32_t)(0u - (uint32_t)value) >> 12);
    value = input[1];
    output[1] = value >= 0 ? value >> 12 : -((int32_t)(0u - (uint32_t)value) >> 12);
    value = input[2];
    output[2] = value >= 0 ? value >> 12 : -((int32_t)(0u - (uint32_t)value) >> 12);
    return 0;
}