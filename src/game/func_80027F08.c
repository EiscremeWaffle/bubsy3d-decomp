#include "game_transition.h"

extern const GameTransitionHeadingTable *g_level_heading_table;
extern const uint32_t *g_level_event_parameters;
extern const GameTransitionHeadingTable *g_transition_heading_handlers;

void func_80027F08(void) {
    g_transition_heading_handlers = g_level_heading_table;
    func_80044C34(g_level_event_parameters);
}
