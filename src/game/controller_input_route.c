/* Host model of the selection and timeout prefix of func_80023E94, not a byte match. */
#include "controller_input_route.h"
#include "input_events.h"

extern int32_t func_8003AA1C(void **output);
extern void func_8001AE88(uint32_t *output);
extern void func_80049454(uint32_t buttons);

int32_t controller_route_input_event(GameTransitionEventRecord *event, ControllerInputRouteResult *result) {
    void *context;
    uint32_t sample;
    uint32_t buttons;
    uint8_t mode;

    func_8003AA1C(&context);
    mode = func_8001BCE4();
    func_8001AE88(&sample);
    result->game_context = context;
    result->mode = mode;
    if (func_8001BCF0() != 0) {
        uint32_t threshold = func_8001BD00() + 100U;
        if ((threshold ^ 0x80000000U) < (sample ^ 0x80000000U)
            && event->payload.input.buttons != 0) {
            g_controller_timeout_ticks = 40;
            func_8001C04C();
            return 1;
        }
    }

    buttons = event->payload.input.buttons;
    if (g_controller_second_port_mode == 1) {
        if (g_controller_port_selection == 0) {
            result->buttons = buttons & 0xFFFFU;
            func_80049454(buttons >> 16);
        } else {
            result->buttons = buttons >> 16;
            func_80049454(buttons & 0xFFFFU);
        }
    } else {
        result->buttons = buttons;
    }
    return 0;
}
