#include <stdint.h>

extern int32_t func_8005668C(uint32_t *state, uint8_t selector);

int32_t func_800226C0(void *actor, uint8_t selector) {
    return func_8005668C((uint32_t *)((uint8_t *)actor + 4), selector);
}
