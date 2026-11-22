#ifndef BUBSY3D_MODE_CAMERA_HELPERS_H
#define BUBSY3D_MODE_CAMERA_HELPERS_H

#include "camera_slots.h"
#include <stdint.h>

extern int32_t g_unknown_801D89C0[3];
extern uint8_t g_unknown_800BE5A4[3];

void func_80028100(int32_t *first, int32_t *second, int32_t *third);
int32_t func_8004EDEC(uint32_t camera_index, const int32_t position[3]);
int32_t func_8004EF10(int32_t first, int32_t second, int32_t third);
void func_80027114(int32_t first, int32_t second);
void func_80072DF0(int32_t first, int32_t second, int32_t third);

#endif
