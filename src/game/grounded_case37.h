#ifndef BUBSY3D_GROUNDED_CASE37_H
#define BUBSY3D_GROUNDED_CASE37_H

#include <stdint.h>

typedef struct GroundedCase37ComponentView {
    uint8_t padding_00[4];
    uint8_t flags_04;
    uint8_t padding_05[0x0F];
    void *transform_14;
    uint8_t padding_18[4];
    struct GroundedCase37ComponentView *owner_1C;
} GroundedCase37ComponentView;

typedef struct GroundedCase37ObjectView {
    uint8_t state_00;
    uint8_t state_01;
    uint8_t padding_02[2];
    uint32_t flags_04;
    uint8_t padding_08[4];
    int32_t record_id_0C;
    uint32_t state_10;
    uint8_t padding_14[8];
    GroundedCase37ComponentView *component_1C;
    uint8_t padding_20[8];
    uint32_t word_28;
    uint8_t state_2C;
    uint8_t padding_2D;
    uint16_t state_2E;
} GroundedCase37ObjectView;

extern GroundedCase37ObjectView *g_grounded_case37_object;

void func_80037B14(GroundedCase37ObjectView *expected, GroundedCase37ObjectView *replacement);
void func_80037B30(void *initial_object);

#endif
