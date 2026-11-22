#ifndef BUBSY3D_CYCLE_MOVEMENT_SETUP_H
#define BUBSY3D_CYCLE_MOVEMENT_SETUP_H

#include "grounded_transform.h"

extern uint8_t g_unknown_800BE020;
extern uint8_t g_unknown_80186108;
extern uint8_t g_unknown_80186109;
extern uint8_t g_unknown_800BE022;
extern uint32_t g_unknown_80186110;
extern uint32_t g_unknown_80186114;
extern uint32_t g_unknown_80186118[3];
extern uint32_t g_unknown_80186148[3];
extern void (*g_unknown_80186144)(void);

/* Models func_8002EB70 only with zero a4 and a7. */
void cycle_movement_setup(
    const GroundedTransformVector *direction, int32_t distance, uint32_t height,
    int32_t duration, void (*callback)(void)
);
void cycle_movement_setup_with_target(
    const GroundedTransformVector *direction, int32_t distance, uint32_t height,
    int32_t duration, void (*callback)(void), const GroundedTransformVector *target
);
int32_t func_80032A2C(uint8_t mode);

#endif
