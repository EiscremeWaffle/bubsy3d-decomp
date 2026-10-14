#include <stdint.h>

int32_t func_80054AF0(const int16_t *component, int32_t *output) {
    output[0] = component[2];
    output[1] = -component[5];
    output[2] = component[8];
    return 0;
}