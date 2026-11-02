#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
#define MATCH_REGISTER(name) __asm__(name)
#else
#define MATCH_REGISTER(name)
#endif

int32_t func_80062E50(const int32_t *current, const int32_t *target, int32_t *distance_out) {
    const volatile int32_t *current_values = current;
    const volatile int32_t *target_values = target;
    register int32_t current_component MATCH_REGISTER("$7");
    register int32_t target_component MATCH_REGISTER("$2");
    register int32_t current_z MATCH_REGISTER("$4");
    register int32_t target_z MATCH_REGISTER("$5");
    register int32_t x MATCH_REGISTER("$3");
    register int32_t y MATCH_REGISTER("$8");
    register int32_t z MATCH_REGISTER("$7");
    register int32_t *output MATCH_REGISTER("$6") = distance_out;
    register int32_t temporary MATCH_REGISTER("$2");
    register int32_t result MATCH_REGISTER("$2");

    current_component = current_values[0];
    target_component = target_values[0];
    x = (int32_t)((uint32_t)current_component - (uint32_t)target_component);
    if (x < 0) x = (int32_t)((uint32_t)target_component - (uint32_t)current_component);
    current_component = current_values[1];
    target_component = target_values[1];
    y = (int32_t)((uint32_t)current_component - (uint32_t)target_component);
    if (y < 0) y = (int32_t)((uint32_t)target_component - (uint32_t)current_component);
    current_z = current_values[2];
    target_z = target_values[2];
    z = (int32_t)((uint32_t)current_z - (uint32_t)target_z);
    if (z < 0) z = (int32_t)((uint32_t)target_z - (uint32_t)current_z);

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

    result = (int32_t)((uint32_t)y + (uint32_t)z);
    result >>= 2;
    result = (int32_t)((uint32_t)x + (uint32_t)result);
    *output = result;
    return 0;
}
