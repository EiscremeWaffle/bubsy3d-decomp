#ifndef BUBSY3D_COMPONENT_ROTATION_H
#define BUBSY3D_COMPONENT_ROTATION_H

#include <stdint.h>

typedef struct ComponentRotationMatrix {
    uint16_t elements[9];
    uint16_t padding;
    int32_t translation[3];
} ComponentRotationMatrix;

int32_t func_800542A4(const ComponentRotationMatrix *input, ComponentRotationMatrix *output);
int32_t func_800547C8(ComponentRotationMatrix *component, const int32_t rotation[3]);
int32_t func_800549E8(ComponentRotationMatrix *component, const ComponentRotationMatrix *basis);
int32_t func_800550A4(ComponentRotationMatrix *component, const ComponentRotationMatrix *basis);
int32_t func_8005541C(ComponentRotationMatrix *component, const int32_t scale[3]);

#endif
