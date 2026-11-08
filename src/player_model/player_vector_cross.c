#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
#include "player_vector_q12.h"

int32_t func_80010128(const int32_t *left, const int32_t *right, int32_t *output) {
    register uint32_t first __asm__("$8");
    register uint32_t second __asm__("$10");

    first = player_q12_product_primary(left[1], right[2]);
    __asm__ volatile("" : "=r"(first) : "0"(first) : "memory");
    second = player_q12_product_secondary(left[2], right[1]);
    output[0] = (int32_t)(first - second);
    __asm__ volatile("" : : : "memory");

    first = player_q12_product_primary(left[2], right[0]);
    __asm__ volatile("" : "=r"(first) : "0"(first) : "memory");
    second = player_q12_product_secondary(left[0], right[2]);
    output[1] = (int32_t)(first - second);
    __asm__ volatile("" : : : "memory");

    first = player_q12_product_primary(left[0], right[1]);
    __asm__ volatile("" : "=r"(first) : "0"(first) : "memory");
    second = player_q12_product_secondary(left[1], right[0]);
    output[2] = (int32_t)(first - second);
    __asm__ volatile("" : : : "memory");
    return 0;
}
#else
int32_t func_80010128(const int32_t *left, const int32_t *right, int32_t *output) {
    output[0] = (int32_t)(((long long)left[1] * right[2] >> 12) -
        ((long long)left[2] * right[1] >> 12));
    output[1] = (int32_t)(((long long)left[2] * right[0] >> 12) -
        ((long long)left[0] * right[2] >> 12));
    output[2] = (int32_t)(((long long)left[0] * right[1] >> 12) -
        ((long long)left[1] * right[0] >> 12));
    return 0;
}
#endif
