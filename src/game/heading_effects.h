#ifndef BUBSY3D_HEADING_EFFECTS_H
#define BUBSY3D_HEADING_EFFECTS_H

#include "bubsy_grounded_state.h"
#include <stdint.h>

extern uint32_t g_heading_effect_started;
extern uint32_t g_heading_effect_duration;
extern uint8_t g_heading_effect_previous;
extern uint8_t g_heading_effect_latched;

void func_80039AB4(BubsyGroundedActor *actor, void *transform);

#endif
