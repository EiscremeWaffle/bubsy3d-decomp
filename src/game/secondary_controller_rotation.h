#ifndef BUBSY3D_SECONDARY_CONTROLLER_ROTATION_H
#define BUBSY3D_SECONDARY_CONTROLLER_ROTATION_H

#include "component_rotation.h"

/* Bounded model of 0x80049664..0x800497A4, not a byte match. */
void secondary_controller_rotation_buttons(
    uint32_t buttons, int32_t direction_y, ComponentRotationMatrix *basis, int32_t rotation[3]
);

/* Models the position calculation from 0x8004980C; inputs must not overlap. */
int32_t secondary_controller_rotation_position(int32_t position[3], const int32_t direction[3]);

#endif
