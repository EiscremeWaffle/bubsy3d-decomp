#include <stdint.h>

int32_t vector_range_condition_q12(const int32_t *input, int32_t *output) __asm__("func_80053D68");

int32_t vector_range_condition_q12(const int32_t *input, int32_t *output) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t x __asm__("$2");
    register int32_t z __asm__("$3");
    register int32_t y __asm__("$2");
    register int32_t absolute_x __asm__("$7");
    register int32_t largest __asm__("$6");
#else
    int32_t x;
    int32_t z;
    int32_t y;
    int32_t absolute_x;
    int32_t largest;
#endif
    int32_t absolute_z;
    int32_t shift;

    x = ((const volatile int32_t *)input)[0];
    z = ((const volatile int32_t *)input)[2];
    absolute_x = x < 0 ? -x : x;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : : : "memory");
#endif
    y = ((const volatile int32_t *)input)[1];
    absolute_z = z < 0 ? -z : z;
    largest = y < 0 ? -y : y;

    if (largest < absolute_x) {
        largest = absolute_x;
    }
    if (largest < absolute_z) {
        largest = absolute_z;
    }

    if (largest < 0x1001) {
        shift = 8;
        output[0] = (int32_t)((uint32_t)((const volatile int32_t *)input)[0] << shift);
        output[1] = (int32_t)((uint32_t)((const volatile int32_t *)input)[1] << shift);
        output[2] = (int32_t)((uint32_t)((const volatile int32_t *)input)[2] << shift);
    } else if (largest <= 0x40000) {
        shift = 2;
        output[0] = (int32_t)((uint32_t)((const volatile int32_t *)input)[0] << shift);
        output[1] = (int32_t)((uint32_t)((const volatile int32_t *)input)[1] << shift);
        output[2] = (int32_t)((uint32_t)((const volatile int32_t *)input)[2] << shift);
    } else if (largest <= 0x01000000) {
        output[0] = ((const volatile int32_t *)input)[0] >> 4;
        output[1] = ((const volatile int32_t *)input)[1] >> 4;
        output[2] = ((const volatile int32_t *)input)[2] >> 4;
    } else {
        output[0] = ((const volatile int32_t *)input)[0] >> 11;
        output[1] = ((const volatile int32_t *)input)[1] >> 11;
        output[2] = ((const volatile int32_t *)input)[2] >> 11;
    }

    return 0;
}
