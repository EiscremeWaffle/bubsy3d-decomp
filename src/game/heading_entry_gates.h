#ifndef BUBSY3D_HEADING_ENTRY_GATES_H
#define BUBSY3D_HEADING_ENTRY_GATES_H

#include "game_transition.h"
#include <stdint.h>

typedef struct HeadingEntryDecision {
    uint8_t counter_changed;
    uint8_t proceed;
} HeadingEntryDecision;

extern uint32_t global_0x800BDE98;
extern uint32_t g_heading_previous_counter;
extern uint8_t g_heading_stop_byte;
extern uint8_t g_heading_active_byte;
extern void *g_actor_handles[2];

HeadingEntryDecision heading_entry_gates(GameTransitionPayload *event);

#endif
