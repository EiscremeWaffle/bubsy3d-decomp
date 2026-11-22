#ifndef BUBSY3D_SECONDARY_CONTROLLER_CONFIRM_H
#define BUBSY3D_SECONDARY_CONTROLLER_CONFIRM_H

#include "controller_result.h"
#include "component_rotation.h"
#include "controller_support.h"
#include "grounded_event_records.h"

typedef enum SecondaryControllerConfirmResult {
    SECONDARY_CONFIRM_EXIT,
    SECONDARY_CONFIRM_ACTIVE,
    SECONDARY_CONFIRM_START
} SecondaryControllerConfirmResult;

/* Bounded model of 0x8004989C..0x80049920, not a byte match. */
SecondaryControllerConfirmResult secondary_controller_confirm_gate(
    uint32_t buttons, ResultCounterContext *context
);

extern ComponentRotationMatrix g_secondary_controller_basis;
extern ControllerScreenPosition g_secondary_controller_confirm_position;
extern GroundedEventRecord *g_secondary_controller_confirm_event;

/* The reference basis returned by the getter must not overlap the saved basis. */
SecondaryControllerConfirmResult secondary_controller_confirm_start(ResultCounterContext *context);

#endif
