#include "controller_input.h"

int32_t func_8004FCF8(uint32_t *buttons_out) {
    uint8_t controller_type[2];
    uint16_t buttons[2];
    int32_t i;

    for (i = 0; i < 2; ++i) {
        if (g_controller_packets[i][0] != 0) {
            controller_type[i] = 0;
        } else {
            controller_type[i] = g_controller_packets[i][1];
        }
        if (controller_type[i] == CONTROLLER_DIGITAL_TYPE) {
            buttons[i] = ~((g_controller_packets[i][2] << 8) | g_controller_packets[i][3]);
        } else {
            buttons[i] = 0;
            controller_type[i] = 0;
        }
    }

    *buttons_out = (uint32_t)buttons[0] | ((uint32_t)buttons[1] << 16);
    if (controller_type[0] == 0) {
        return controller_type[1] == 0 ? CONTROLLER_NONE : CONTROLLER_PORT_2_ONLY;
    }
    return controller_type[1] == 0 ? CONTROLLER_PORT_1_ONLY : CONTROLLER_BOTH;
}
