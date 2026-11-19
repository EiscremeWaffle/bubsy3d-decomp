#include "component_fields.h"

int32_t func_80052AD4(const void *component, int32_t *output) {
    *output = ((const uint8_t *)component)[4] & 3;
    return 0;
}
