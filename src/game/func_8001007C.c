#include <stdint.h>

int32_t func_8001007C(int32_t left, int32_t right, int32_t *output) {
    register int32_t high __asm__("$4");
    register uint32_t low __asm__("$5");
    register int32_t high_scaled __asm__("$2");
    register int32_t low_scaled __asm__("$3");

    __asm__ volatile(
        "mult %1,%2\n\t"
        "mfhi %0"
        : "=r"(high)
        : "r"(left), "r"(right)
        : "hi", "lo"
    );
    high_scaled = high << 20;
    __asm__ volatile("mflo %0" : "=r"(low) : : "hi", "lo");
    low_scaled = (int32_t)(low >> 12);
    *output = high_scaled + low_scaled;
    return 0;
}
