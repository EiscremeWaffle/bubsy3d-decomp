/* Bounded host model of func_80023E94, not a byte match. */
#include "controller_grounded_input_path.h"
#include "controller_mode_one_tail.h"

ControllerGroundedInputOutcome controller_grounded_input_path(
    GameTransitionEventRecord *event,
    ControllerInputPrepareState *prepare,
    ControllerInputRouteResult *route,
    const ControllerBitmapDispatch *dispatch,
    GameCyclicEntry *cycle_entry
) {
    prepare->second_port_enabled = g_controller_second_port_mode;
    prepare->mirror_extra_buttons = g_controller_pulse_enabled;
    controller_prepare_input_event(event, prepare);
    if (controller_route_input_event(event, route)) {
        return CONTROLLER_GROUNDED_INPUT_TIMEOUT;
    }
    if (route->mode == 1) {
        ControllerGameContext *context = route->game_context;
        ControllerModeOneTailResult mode_result;

        if (event->payload.input.buttons != 0) {
            uint8_t state = context->mode_6d;
            if (state != 1 && state != 3) {
                mode_result = controller_mode_one_button_color(event->payload.input.buttons);
                if (mode_result == CONTROLLER_MODE_ONE_EXIT) {
                    return CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT;
                }
                mode_result = controller_mode_one_enabled_pulse_button(
                    event->payload.input.buttons, route->mode, context
                );
                if (mode_result == CONTROLLER_MODE_ONE_EXIT) {
                    return CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT;
                }
                mode_result = controller_mode_one_disabled_pulse_button(event->payload.input.buttons);
                if (mode_result == CONTROLLER_MODE_ONE_EXIT) {
                    return CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT;
                }
                return CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT;
            }
        }
        mode_result = controller_mode_one_tail(context, route->mode);
        if (mode_result == CONTROLLER_MODE_ONE_EXIT) {
            return CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT;
        }
    }
    if (!controller_gate_bitmap(route->game_context, route->mode, route->buttons, cycle_entry)) {
        return CONTROLLER_GROUNDED_INPUT_GATED;
    }
    controller_bitmap_dispatch(dispatch, route->buttons);
    return CONTROLLER_GROUNDED_INPUT_DISPATCHED;
}
