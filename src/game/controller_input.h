#ifndef BUBSY3D_CONTROLLER_INPUT_H
#define BUBSY3D_CONTROLLER_INPUT_H

#include <stdint.h>

enum {
    CONTROLLER_DIGITAL_TYPE = 0x41,
    CONTROLLER_NONE = 0x2A,
    CONTROLLER_PORT_2_ONLY = 0x2B,
    CONTROLLER_PORT_1_ONLY = 0x2C,
    CONTROLLER_BOTH = 0x2D
};

extern volatile uint8_t g_controller_packets[2][8];
extern uint8_t g_input_stream_mode;
extern int32_t g_input_stream_limit;
extern uint32_t g_input_stream_index;
extern int32_t g_input_stream_words[];
extern int32_t g_input_stream_sample;
extern uint8_t g_input_effect_active;
extern int8_t g_input_effect_channel;
extern uint8_t g_input_effect_context[];
extern int32_t g_unknown_800BE400;

void func_8001A6E0(void);
int32_t func_8004FCBC(void);
int32_t func_8004FCF8(uint32_t *buttons_out);
int32_t func_8004FF28(int32_t sample);
int32_t func_80061A64(int32_t value);

#endif
