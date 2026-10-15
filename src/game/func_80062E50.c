#include <stdint.h>

int32_t func_80062E50(const int32_t *current, const int32_t *target, int32_t *distance_out) {
    const volatile int32_t *current_values = current;
    const volatile int32_t *target_values = target;
    register int32_t current_component __asm__("$7");
    register int32_t target_component __asm__("$2");
    register int32_t current_z __asm__("$4");
    register int32_t target_z __asm__("$5");
    register int32_t x __asm__("$3");
    register int32_t y __asm__("$8");
    register int32_t z __asm__("$7");
    register int32_t *output __asm__("$6") = distance_out;
    register int32_t temporary __asm__("$2");
    register int32_t result __asm__("$2");

    current_component = current_values[0];
    target_component = target_values[0];
    x = current_component - target_component;
    if (x < 0) x = target_component - current_component;
    current_component = current_values[1];
    target_component = target_values[1];
    y = current_component - target_component;
    if (y < 0) y = target_component - current_component;
    current_z = current_values[2];
    target_z = target_values[2];
    z = current_z - target_z;
    if (z < 0) z = target_z - current_z;

    if (x < y) {
        temporary = x;
        x = y;
        y = temporary;
    }
    if (x < z) {
        temporary = x;
        x = z;
        z = temporary;
    }

    result = y + z;
    result >>= 2;
    __asm__ volatile("addu $2,$3,$2" : "=r"(result) : "0"(result), "r"(x));
    *output = result;
    return 0;
}
