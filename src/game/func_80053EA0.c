#include <stdint.h>

static __inline__ int32_t half_component(int32_t value) {
    if (value >= 0) {
        return value >> 1;
    }
    return (int32_t)(0U - (uint32_t)((int32_t)(0U - (uint32_t)value) >> 1));
}

int32_t func_80053EA0(const int32_t *input, int32_t *output) {
    output[0] = half_component(input[0]);
    output[1] = half_component(input[1]);
    output[2] = half_component(input[2]);
    return 0;
}
