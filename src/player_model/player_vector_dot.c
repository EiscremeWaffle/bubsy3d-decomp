#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
#include "player_vector_q12.h"

int32_t func_800100A0(const int32_t *left, const int32_t *right, int32_t *output) {
    register uint32_t total __asm__("$10") = 0;

    __asm__ volatile("" : "=r"(total) : "0"(total));
    total += player_q12_product_primary(left[0], right[0]);
    __asm__ volatile("" : "=r"(total) : "0"(total) : "memory");
    total += player_q12_product_primary(left[1], right[1]);
    __asm__ volatile("" : "=r"(total) : "0"(total) : "memory");
    total += player_q12_product_primary(left[2], right[2]);
    *output = (int32_t)total;
    __asm__ volatile("" : : : "memory");
    return 0;
}
#else
int32_t func_800100A0(const int32_t *left, const int32_t *right, int32_t *output) {
    *output = (int32_t)(((long long)left[0] * right[0] >> 12) +
        ((long long)left[1] * right[1] >> 12) +
        ((long long)left[2] * right[2] >> 12));
    return 0;
}
#endif
