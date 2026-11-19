#ifndef BUBSY3D_CONTROLLER_GROUNDED_INPUT_PATH_H
#define BUBSY3D_CONTROLLER_GROUNDED_INPUT_PATH_H

#include "controller_bitmap_dispatch.h"
#include "controller_bitmap_gate.h"
#include "controller_input_prepare.h"
#include "controller_input_route.h"

typedef enum ControllerGroundedInputOutcome {
    CONTROLLER_GROUNDED_INPUT_TIMEOUT = 0,
    CONTROLLER_GROUNDED_INPUT_GATED = 1,
    CONTROLLER_GROUNDED_INPUT_DISPATCHED = 2,
    CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT = 4
} ControllerGroundedInputOutcome;

/* The caller supplies live latch and bitmap state; mode-one button handling exits before bitmap dispatch. */
ControllerGroundedInputOutcome controller_grounded_input_path(
    GameTransitionEventRecord *event,
    ControllerInputPrepareState *prepare,
    ControllerInputRouteResult *route,
    const ControllerBitmapDispatch *dispatch,
    GameCyclicEntry *cycle_entry
);

#endif
