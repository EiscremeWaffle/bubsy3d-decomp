#ifndef BUBSY3D_SECONDARY_CONTROLLER_ACTIVE_H
#define BUBSY3D_SECONDARY_CONTROLLER_ACTIVE_H

#include "grounded_case37.h"

typedef enum SecondaryControllerActiveResult {
    SECONDARY_ACTIVE_EXIT,
    SECONDARY_ACTIVE_CONTINUE
} SecondaryControllerActiveResult;

/* Bounded model of 0x80049A24..0x80049B08, not a byte match; outputs are separate storage. */
SecondaryControllerActiveResult secondary_controller_active_gate(
    uint32_t buttons, GroundedCase37ComponentView **object, void **component
);

#endif
