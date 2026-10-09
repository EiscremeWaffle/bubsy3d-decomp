#include <stdint.h>

#ifndef MATRIX_SCALE_FUNCTION
#define MATRIX_SCALE_FUNCTION func_80072F4C
#endif

int32_t MATRIX_SCALE_FUNCTION(int16_t *matrix, int32_t scale_x, int32_t scale_y, int32_t scale_z) {
    register int32_t scale __asm__("$5");
    register int32_t sign_extend_temp __asm__("$2");
    int32_t changed = 0;

    scale = (int16_t)scale_x;
    if (scale != 0x1000) {
        matrix[0] = (int16_t)(((int32_t)matrix[0] * scale) >> 12);
        matrix[3] = (int16_t)(((int32_t)matrix[3] * scale) >> 12);
        matrix[6] = (int16_t)(((int32_t)matrix[6] * scale) >> 12);
        changed = 1;
    }

    sign_extend_temp = (int32_t)((uint32_t)(uint16_t)scale_y << 16);
    scale = sign_extend_temp >> 16;
    if (scale != 0x1000) {
        matrix[1] = (int16_t)(((int32_t)matrix[1] * scale) >> 12);
        matrix[4] = (int16_t)(((int32_t)matrix[4] * scale) >> 12);
        matrix[7] = (int16_t)(((int32_t)matrix[7] * scale) >> 12);
        changed = 1;
    }

    sign_extend_temp = (int32_t)((uint32_t)(uint16_t)scale_z << 16);
    scale = sign_extend_temp >> 16;
    if (scale != 0x1000) {
        matrix[2] = (int16_t)(((int32_t)matrix[2] * scale) >> 12);
        matrix[5] = (int16_t)(((int32_t)matrix[5] * scale) >> 12);
        matrix[8] = (int16_t)(((int32_t)matrix[8] * scale) >> 12);
        changed = 1;
    }

    return changed;
}
