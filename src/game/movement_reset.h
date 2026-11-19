#ifndef BUBSY3D_MOVEMENT_RESET_H
#define BUBSY3D_MOVEMENT_RESET_H

#include <stdint.h>

typedef struct MovementResetComponent {
    uint8_t unknown_00[4];
    uint8_t flags_04;
    uint8_t unknown_05[0x0B];
    uint32_t field_10;
    void *transform_14;
} MovementResetComponent;

typedef struct MovementResetObject {
    uint8_t unknown_00[4];
    uint32_t field_04;
    uint32_t field_08;
    int32_t record_id_0C;
    uint32_t field_10;
    uint32_t field_14;
    uint32_t field_18;
    MovementResetComponent *component_1C;
    MovementResetComponent *volatile component_20;
    MovementResetComponent *component_24;
    uint32_t field_28;
    uint8_t field_2C;
    uint8_t unknown_2D;
    int16_t field_2E;
    uint32_t field_30;
} MovementResetObject;

void func_80028600(MovementResetObject *object, MovementResetComponent *component);
void func_80028698(MovementResetObject *object);

#endif
