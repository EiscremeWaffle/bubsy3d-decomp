#ifndef BUBSY3D_CONTROLLER_RESULT_H
#define BUBSY3D_CONTROLLER_RESULT_H

#include <stdint.h>

typedef struct ControllerResultContext {
    uint8_t unknown_00[0x0C];
    uint8_t mode_0C;
    uint8_t unknown_0D[7];
    int16_t selection_14;
} ControllerResultContext;

extern int16_t g_result_previous_selection;
extern const char g_result_player_one_text[];
extern const char g_result_player_two_text[];

void func_8002C370(void *argument0, void *argument1);

#endif
