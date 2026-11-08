#include "game_transition.h"

extern const GameTransitionModeHandlers g_default_transition_mode_handlers[];
extern const GameTransitionModeHandlers *g_transition_mode_handlers;

void func_8002278C(void) {
    g_transition_mode_handlers = g_default_transition_mode_handlers;
}
