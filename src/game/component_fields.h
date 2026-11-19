#ifndef BUBSY3D_COMPONENT_FIELDS_H
#define BUBSY3D_COMPONENT_FIELDS_H

#include <stdint.h>

int32_t func_80052AD4(const void *component, int32_t *output);
int32_t func_800532CC(
    void *component, int32_t first, int32_t second, int32_t third, uint32_t flags
);

#endif