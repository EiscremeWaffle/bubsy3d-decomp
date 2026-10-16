#include <stdint.h>

uint32_t func_80056678(uint32_t value, uint32_t selector) {
#ifndef BUBSY3D_MATCH_ORIGINAL_L0
    selector &= 31u;
#endif
    return (value & (1u << selector)) != 0;
}