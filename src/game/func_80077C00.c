#include "matrix_transpose.h"

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
/* Keep the original load/store order, including for overlapping matrices. */
#define TRANSPOSE_KEEP(value) __asm__ volatile("" : "=r"(value) : "0"(value) : "memory")
#else
#define TRANSPOSE_KEEP(value) ((void)0)
#endif

uint16_t *func_80077C00(const uint16_t *source, uint16_t *destination) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    const int16_t *input = (const int16_t *)source;
    register uint16_t *output __asm__("$5") = destination;
    register uint16_t *result __asm__("$2");
    register int32_t first __asm__("$9");
    register int32_t second __asm__("$10");
    register int32_t third __asm__("$11");
#else
    const volatile int16_t *input = (const volatile int16_t *)source;
    volatile uint16_t *output = destination;
    uint16_t *result;
    int32_t first, second, third;
#endif

    first = input[0];
    TRANSPOSE_KEEP(first);
    result = destination;
    TRANSPOSE_KEEP(result);
    output[0] = (uint16_t)first;
    second = input[3];
    TRANSPOSE_KEEP(second);
    first = input[6];
    TRANSPOSE_KEEP(first);
    output[1] = (uint16_t)second;
    third = input[1];
    TRANSPOSE_KEEP(third);
    output[2] = (uint16_t)first;
    second = input[4];
    TRANSPOSE_KEEP(second);
    output[3] = (uint16_t)third;
    first = input[7];
    TRANSPOSE_KEEP(first);
    output[4] = (uint16_t)second;
    third = input[2];
    TRANSPOSE_KEEP(third);
    output[5] = (uint16_t)first;
    second = input[5];
    TRANSPOSE_KEEP(second);
    output[6] = (uint16_t)third;
    first = input[8];
    TRANSPOSE_KEEP(first);
    output[7] = (uint16_t)second;
    output[8] = (uint16_t)first;
    return result;
}
