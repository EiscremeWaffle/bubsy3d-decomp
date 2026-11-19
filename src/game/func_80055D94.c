#include "controller_support.h"

int32_t func_80055D94(ControllerScreenPosition *position, uint32_t x, uint32_t y) {
    if (position == 0) {
        return 22;
    }

    position->x_04 = (uint16_t)(x - g_controller_screen_origin.x);
    position->y_06 = (uint16_t)(y - g_controller_screen_origin.y);
    return 0;
}
