#include <stdint.h>

extern uint32_t func_80056678(uint32_t value, uint32_t selector);

uint8_t func_80022604(const void *actor, uint8_t selector) {
    return (uint8_t)func_80056678(*(const uint32_t *)((const uint8_t *)actor + 0x10), selector);
}
