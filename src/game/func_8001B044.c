#include "game_transition.h"
#include "word_copy.h"

extern GameTransitionQueue g_transition_queue_input;

void func_8001B044(const GameTransitionEventRecord *record) {
    uint32_t count = g_transition_queue_input.count;

    func_80010220(
        g_transition_queue_input.records[count].words,
        (const uint32_t *)record,
        7
    );
    ++g_transition_queue_input.count;
}
