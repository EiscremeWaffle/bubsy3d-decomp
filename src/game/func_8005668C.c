#include <stdint.h>

extern uint32_t func_80056678(uint32_t value, uint32_t selector);

int32_t func_8005668C(uint32_t *state, uint8_t selector) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t *state_value __asm__("$17") = state;
    register uint32_t mask __asm__("$16") = 1u;
    register uint32_t result __asm__("$2");
    mask <<= selector;
#else
    uint32_t *state_value = state;
    uint32_t mask = 1u << (selector & 31u);
    uint32_t result;
#endif
    result = (uint8_t)func_80056678(*state_value, selector);
    if (result == 0) {
        result = *state_value ^ mask;
        *state_value = result;
    }
    return (int32_t)result;
}