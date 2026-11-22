#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
#define signed_word(value) ((int32_t)(value))
#define shift_floor(value, shift) ((value) >> (shift))
#else
static int32_t signed_word(uint32_t value) {
    return value <= INT32_MAX ? (int32_t)value : (int32_t)((int64_t)value - 4294967296LL);
}

static int32_t shift_floor(int32_t value, uint32_t shift) {
    int32_t divisor = (int32_t)(1U << shift);
    return value >= 0 ? value / divisor : -1 - ((-1 - value) / divisor);
}
#endif

int32_t func_80022B5C(const int32_t position[3], const int32_t other[3]) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t px __asm__("$3") = position[0];
    register int32_t qx __asm__("$2") = other[0];
    register int32_t pz __asm__("$4"), qz __asm__("$5");
    register int32_t x __asm__("$6") = signed_word((uint32_t)px - (uint32_t)qx);
    register int32_t z __asm__("$2");
    register int32_t sum __asm__("$3");
    register int32_t difference __asm__("$4");
    register int32_t half __asm__("$5");
    register int32_t quarter __asm__("$7");
#else
    int32_t px = position[0], qx = other[0];
    int32_t pz, qz;
    int32_t x = signed_word((uint32_t)px - (uint32_t)qx);
    int32_t z;
    int32_t sum, difference, half, quarter;
#endif

    if (x < 0) x = signed_word((uint32_t)qx - (uint32_t)px);
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(x) : "0"(x) : "memory");
#endif
    pz = position[2];
    qz = other[2];
    z = signed_word((uint32_t)pz - (uint32_t)qz);
    if (z < 0) z = signed_word((uint32_t)qz - (uint32_t)pz);
    sum = signed_word((uint32_t)x + (uint32_t)z);
    difference = signed_word((uint32_t)x - (uint32_t)z);
    half = shift_floor(sum, 1);
    quarter = shift_floor(sum, 2);
    if (difference < 0) difference = signed_word((uint32_t)z - (uint32_t)x);
    if (difference < half) {
        sum = signed_word((uint32_t)sum - (uint32_t)quarter);
    } else {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register int32_t upper __asm__("$2");
        __asm__ volatile("" : "=r"(half), "=r"(quarter) : "0"(half), "1"(quarter));
#else
        int32_t upper;
#endif
        upper = signed_word((uint32_t)half + (uint32_t)quarter);
        if (difference <= upper) sum = signed_word((uint32_t)sum - (uint32_t)shift_floor(sum, 3));
    }
    return sum;
}
