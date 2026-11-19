#include "component_fields.h"

int32_t func_800532CC(
    void *component, int32_t first, int32_t second, int32_t third, uint32_t flags
) {
    uint32_t packed = flags | ((uint32_t)(int16_t)second << 16) |
        ((uint32_t)(int16_t)first << 8) | (uint32_t)(int16_t)third;

    *(uint32_t *)((uint8_t *)component + 0x24) = packed;
    return (int32_t)packed;
}