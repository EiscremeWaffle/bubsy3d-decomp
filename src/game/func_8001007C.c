#include <stdint.h>

int32_t func_8001007C(int32_t left, int32_t right, int32_t *output) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t high __asm__("$4");
    register uint32_t low __asm__("$5");
    register uint32_t high_scaled __asm__("$2");
    register uint32_t low_scaled __asm__("$3");

    __asm__ volatile(
        "mult %1,%2\n\t"
        "mfhi %0"
        : "=r"(high)
        : "r"(left), "r"(right)
        : "hi", "lo"
    );
    high_scaled = high << 20;
    __asm__ volatile("mflo %0" : "=r"(low) : : "hi", "lo");
    low_scaled = low >> 12;
    *output = (int32_t)(high_scaled + low_scaled);
#else
    uint32_t bits = (uint32_t)((uint64_t)((int64_t)left * right) >> 12);

    *output = bits <= INT32_MAX ? (int32_t)bits : (int32_t)((int64_t)bits - 4294967296LL);
#endif
    return 0;
}
