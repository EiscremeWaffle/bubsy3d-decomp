/* Host model of the input-preparation prefix of func_80023E94, not a byte match. */
#include "controller_input_prepare.h"

extern uint32_t func_80056678(uint32_t value, uint32_t selector);
extern int32_t func_8005668C(uint32_t *value, uint8_t selector);

static void mirror_button(uint32_t *buttons, uint32_t source, uint8_t from, uint8_t to) {
    if ((uint8_t)func_80056678(source, from) != 0) {
        func_8005668C(buttons, to);
    }
}

void controller_prepare_input_event(
    GameTransitionEventRecord *event, ControllerInputPrepareState *state
) {
    GameTransitionInputPayload *input = &event->payload.input;
    uint32_t first;
    uint32_t second;

    if (input->digital_pad_1 == 0) {
        if (state->port_1_latched != 0) {
            state->port_1_latched = 0;
            input->buttons = 0;
            func_8005668C(&input->buttons, 11);
        }
    } else if (state->port_1_latched == 0) {
        state->port_1_latched = 1;
    }

    if (state->second_port_enabled != 0) {
        if (input->digital_pad_2 == 0) {
            if (state->port_2_latched != 0) {
                state->port_2_latched = 0;
                input->buttons = 0;
                func_8005668C(&input->buttons, 11);
            }
        } else if (state->port_2_latched == 0) {
            state->port_2_latched = 1;
        }
    }

    first = input->buttons & 0xFFFFU;
    second = input->buttons >> 16;
    mirror_button(&input->buttons, second, 8, 8);
    mirror_button(&input->buttons, second, 11, 11);
    mirror_button(&input->buttons, first, 8, 24);
    mirror_button(&input->buttons, first, 11, 27);
    if (state->mirror_extra_buttons != 0) {
        mirror_button(&input->buttons, second, 12, 12);
        mirror_button(&input->buttons, second, 14, 14);
        mirror_button(&input->buttons, second, 6, 6);
    }
}
