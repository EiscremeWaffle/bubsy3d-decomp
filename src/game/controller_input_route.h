#ifndef BUBSY3D_CONTROLLER_INPUT_ROUTE_H
#define BUBSY3D_CONTROLLER_INPUT_ROUTE_H

#include "game_transition.h"

typedef struct ControllerInputRouteResult {
    void *game_context;
    uint32_t buttons;
    uint8_t mode;
} ControllerInputRouteResult;

extern volatile uint8_t g_controller_second_port_mode;
extern volatile int16_t g_controller_port_selection;
extern volatile uint8_t g_controller_timeout_ticks;

/* Returns nonzero when the timeout path exits the controller callback. */
int32_t controller_route_input_event(GameTransitionEventRecord *event, ControllerInputRouteResult *result);

#endif
