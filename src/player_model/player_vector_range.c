#include <stdint.h>

int32_t func_80053D68(const int32_t *input, int32_t *output) {
    int32_t absolute_x = input[0] < 0 ? -input[0] : input[0];
    int32_t absolute_y = input[1] < 0 ? -input[1] : input[1];
    int32_t absolute_z = input[2] < 0 ? -input[2] : input[2];
    int32_t largest = absolute_x;
    int32_t shift;

    if (largest < absolute_y) {
        largest = absolute_y;
    }
    if (largest < absolute_z) {
        largest = absolute_z;
    }

    if (largest < 0x1001) {
        shift = 8;
        output[0] = input[0] << shift;
        output[1] = input[1] << shift;
        output[2] = input[2] << shift;
    } else if (largest <= 0x4000) {
        shift = 2;
        output[0] = input[0] << shift;
        output[1] = input[1] << shift;
        output[2] = input[2] << shift;
    } else if (largest <= 0x01000000) {
        output[0] = input[0] >> 4;
        output[1] = input[1] >> 4;
        output[2] = input[2] >> 4;
    } else {
        output[0] = input[0] >> 11;
        output[1] = input[1] >> 11;
        output[2] = input[2] >> 11;
    }

    return 0;
}
