#include "secondary_controller_rotation.h"

extern uint32_t func_80056678(uint32_t value, uint32_t selector);

static int32_t signed_word(uint32_t value) {
    return (value & 0x80000000u) != 0 ? -1 - (int32_t)~value : (int32_t)value;
}

static int32_t rotation_y_adjustment(int32_t direction_y) {
    uint32_t magnitude = (uint32_t)direction_y;
    uint32_t shifted;
    int32_t signed_shifted;

    if (direction_y < 0) {
        magnitude = 0u - magnitude;
    }
    shifted = magnitude << 12;
    signed_shifted = signed_word(shifted);
    return (signed_shifted / 360) / 4096;
}

int32_t secondary_controller_rotation_position(int32_t position[3], const int32_t direction[3]) {
    uint32_t sum_y;

    position[0] = signed_word((uint32_t)position[0] + (uint32_t)direction[0]);
    sum_y = (uint32_t)position[1] + (uint32_t)direction[1];
    position[2] = signed_word((uint32_t)position[2] + (uint32_t)direction[2]);
    if (direction[1] >= 950) {
        sum_y = sum_y + 950u - (uint32_t)direction[1];
    }
    if (signed_word(sum_y - (uint32_t)direction[1]) < 320 && direction[1] < -924) {
        sum_y = sum_y - 925u - (uint32_t)direction[1];
        position[1] = signed_word(sum_y);
        return 1;
    }
    position[1] = signed_word(sum_y);
    return 0;
}

void secondary_controller_rotation_buttons(
    uint32_t buttons, int32_t direction_y, ComponentRotationMatrix *basis, int32_t rotation[3]
) {
    if ((uint8_t)func_80056678(buttons, 13) != 0) {
        rotation[1] = 11 + rotation_y_adjustment(direction_y);
        func_80048FA8(basis, rotation);
    }
    if ((uint8_t)func_80056678(buttons, 15) != 0) {
        rotation[1] = -11 - rotation_y_adjustment(direction_y);
        func_80048FA8(basis, rotation);
    }
    if ((uint8_t)func_80056678(buttons, 12) != 0) {
        rotation[0] = -11;
        func_80049080(basis, rotation);
    }
    if ((uint8_t)func_80056678(buttons, 14) != 0) {
        rotation[0] = 11;
        func_80049080(basis, rotation);
    }
}
