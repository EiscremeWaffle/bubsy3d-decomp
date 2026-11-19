#ifndef BUBSY3D_CONTROLLER_INPUT_PREPARE_H
#define BUBSY3D_CONTROLLER_INPUT_PREPARE_H

#include "game_transition.h"
#include <stdint.h>

typedef struct ControllerInputPrepareState {
    uint8_t port_1_latched;
    uint8_t port_2_latched;
    uint8_t second_port_enabled;
    uint8_t mirror_extra_buttons;
} ControllerInputPrepareState;

void controller_prepare_input_event(
    GameTransitionEventRecord *event, ControllerInputPrepareState *state
);

#endif
