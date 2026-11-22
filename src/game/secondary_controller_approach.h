#ifndef BUBSY3D_SECONDARY_CONTROLLER_APPROACH_H
#define BUBSY3D_SECONDARY_CONTROLLER_APPROACH_H

#include "component_rotation.h"
#include "move_preparation.h"
#include <stddef.h>

typedef union SecondaryApproachPosition {
    int32_t words[4];
    MovePreparationVector vector;
} SecondaryApproachPosition;

typedef struct SecondaryControllerApproachActor {
    uint32_t reset;
    uint8_t unknown_04[0x14];
    void *handle_18;
    uint8_t unknown_1c[8];
    MovePreparationInfo *info_24;
} SecondaryControllerApproachActor;

typedef struct SecondaryControllerApproach {
    SecondaryApproachPosition position;
    int32_t offset[3];
    ComponentRotationMatrix *component;
    uint32_t iteration;
} SecondaryControllerApproach;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && UINTPTR_MAX == 0xFFFFFFFFu
_Static_assert(offsetof(SecondaryControllerApproachActor, handle_18) == 0x18, "approach handle offset");
_Static_assert(offsetof(SecondaryControllerApproachActor, info_24) == 0x24, "approach info offset");
_Static_assert(sizeof(SecondaryApproachPosition) == 16, "approach position size");
#endif

extern SecondaryControllerApproachActor g_secondary_controller_approach_actor;
extern MovePreparationInfo g_secondary_controller_approach_move_info;

/*
 * Bounded model of 0x80049CB8..0x80049DD0, not a byte match.
 * The input and output storage must not overlap each other or the component.
 */
int32_t secondary_controller_approach(
    const int32_t position[4], const int32_t direction[3],
    SecondaryControllerApproach *output
);

/*
 * Bounded model of 0x80049DD0..0x80049E78, not a byte match.
 * The base, position, and step arrays must not overlap.
 */
void secondary_controller_approach_step(
    int32_t iteration, int32_t base[4], int32_t position[4], const int32_t step[3],
    ComponentRotationMatrix *anchor, ComponentRotationMatrix *component
);

#endif
