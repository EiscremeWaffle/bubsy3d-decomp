#include "component_rotation.h"

extern int32_t func_800540BC(const int32_t *input, const ComponentRotationMatrix *basis, int32_t *output);

int32_t func_80064528(const int32_t *input, const ComponentRotationMatrix *basis, int32_t *output) {
    return func_800540BC(input, basis, output);
}
