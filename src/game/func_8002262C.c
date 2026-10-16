#include <stdint.h>

extern int32_t func_800566E8(uint32_t *state, uint8_t selector);

int32_t func_8002262C(void *actor, uint8_t selector) {
    return func_800566E8((uint32_t *)((uint8_t *)actor + 0x10), selector);
}