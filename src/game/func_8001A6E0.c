#include "controller_input.h"
#include "game_transition.h"
#include "sequence_queries.h"
#include "texture_font_slots.h"

extern void func_8004E8E0(void);
extern void func_800486D0(void *context, int32_t first, int32_t second, int8_t *channel);
extern void func_800481D4(int32_t channel, int32_t first, int32_t second);

void func_8001A6E0(void) {
    uint32_t buttons;
    uint8_t port_1;
    uint8_t port_2;
    int32_t status;
    int16_t sample;
    int16_t command;

    if (g_input_stream_mode == 1) {
        if ((int32_t)g_input_stream_index >= (int32_t)((uint32_t)g_input_stream_limit - 1u)) {
            func_8004E8E0();
        }
    }

    status = func_8004FCF8(&buttons);
    if (status == CONTROLLER_NONE) {
        port_1 = 0;
        port_2 = 0;
    } else if (status == CONTROLLER_PORT_2_ONLY) {
        port_1 = 0;
        port_2 = 1;
    } else if (status == CONTROLLER_PORT_1_ONLY) {
        port_1 = 1;
        port_2 = 0;
    } else {
        port_1 = 1;
        port_2 = 1;
    }

    if (buttons != 0 && g_input_stream_mode != 0) {
        func_8001C6BC(4, 5, 1);
        func_8004E8E0();
    }
    if (g_input_stream_mode == 1) {
        sample = (int16_t)(g_input_stream_words[g_input_stream_index] >> 16);
        command = sample & 0x3F;
        if (command >= 0 && command < 5 && g_input_effect_active == 0) {
            g_input_effect_active = 1;
            func_800486D0(g_input_effect_context, 0x23, 0x1A9, &g_input_effect_channel);
            func_800481D4(g_input_effect_channel, 0x1000, 0x1000);
            func_800483C0(g_input_effect_channel);
        } else if (command >= 0x20 && command < 0x25 && g_input_effect_active != 0) {
            g_input_effect_active = 0;
            func_80048504(g_input_effect_channel);
            func_80048C48(g_input_effect_channel);
        }
        func_8004FF28(sample);
        buttons = g_input_stream_words[g_input_stream_index++] & 0xFFFF;
    }

    func_8001B558(3, 0, 0xFFFE, 0xFFFE, &buttons, &port_1, &port_2, 0);
}
