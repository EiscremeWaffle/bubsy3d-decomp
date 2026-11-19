#ifndef BUBSY3D_CONTROLLER_MODE_ONE_TAIL_H
#define BUBSY3D_CONTROLLER_MODE_ONE_TAIL_H

#include "controller_bitmap_gate.h"

typedef enum ControllerModeOneTailResult {
    CONTROLLER_MODE_ONE_COMMON_GATE = 0,
    CONTROLLER_MODE_ONE_EXIT = 1,
    CONTROLLER_MODE_ONE_BUTTON_UNMODELED = 3
} ControllerModeOneTailResult;

extern volatile uint8_t g_controller_pulse_enabled;
extern volatile uint8_t g_controller_pulse_value;
extern volatile uint8_t g_controller_pulse_direction;
extern volatile uint8_t g_controller_pulse_selection;
extern int8_t g_controller_prompt_tag;
extern int8_t g_controller_pulse_tag_a;
extern int8_t g_controller_pulse_tag_b;
extern int8_t g_controller_button_tag;
extern uint8_t g_controller_prompt_text[];
extern const char g_controller_button_prompt_text[];
extern const char g_controller_display_text_b[];
extern const char g_controller_display_text_c[];
extern volatile uint8_t g_controller_flash_enabled;

ControllerModeOneTailResult controller_mode_one_tail(ControllerGameContext *context, uint8_t mode);
ControllerModeOneTailResult controller_mode_one_button_color(uint32_t buttons);
ControllerModeOneTailResult controller_mode_one_enabled_pulse_button(
    uint32_t buttons,
    uint8_t mode,
    ControllerGameContext *context
);
ControllerModeOneTailResult controller_mode_one_disabled_pulse_button(uint32_t buttons);

#endif
