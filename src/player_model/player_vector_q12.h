#ifndef BUBSY3D_PLAYER_VECTOR_Q12_H
#define BUBSY3D_PLAYER_VECTOR_Q12_H

#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
/* Return the low 32 bits of a signed product shifted right by 12. */
static __inline__ uint32_t player_q12_product_primary(int32_t left, int32_t right) {
    register uint32_t high __asm__("$8") = (uint32_t)left;
    register uint32_t low __asm__("$9") = (uint32_t)right;

    __asm__ volatile(
        "mult %0,%1\n\t"
        "mfhi %0\n\t"
        "mflo %1"
        : "=r"(high), "=r"(low)
        : "0"(high), "1"(low)
        : "hi", "lo"
    );
    high <<= 20;
    low >>= 12;
    __asm__ volatile("" : "=r"(high) : "0"(high));
    return high + low;
}

static __inline__ uint32_t player_q12_product_secondary(int32_t left, int32_t right) {
    register uint32_t high __asm__("$10") = (uint32_t)left;
    register uint32_t low __asm__("$9") = (uint32_t)right;

    __asm__ volatile(
        "mult %0,%1\n\t"
        "mfhi %0\n\t"
        "mflo %1"
        : "=r"(high), "=r"(low)
        : "0"(high), "1"(low)
        : "hi", "lo"
    );
    high <<= 20;
    low >>= 12;
    __asm__ volatile("" : "=r"(high) : "0"(high));
    return high + low;
}
#endif

#endif
