#include "game_transition.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

GameTransitionQueue g_transition_queue_input;
void func_8001B044(const GameTransitionEventRecord *record);

static void test_appends_complete_event(void) {
    GameTransitionEventRecord source = {
        .event_id = 0x1234,
        .mode = 0x5678,
        .flags = (int32_t)0x89ABCDEFu,
        .heading = (int32_t)0xFEDCBA98u,
        .payload.opaque = {0, 0x10203040, 0x50607080, 0x90A0B0C0},
    };
    GameTransitionEventRecord expected = source;
    unsigned index;

    memset(&g_transition_queue_input, 0xA5, sizeof(g_transition_queue_input));
    g_transition_queue_input.count = 3;
    for (index = 0; index < 3; ++index) {
        memset(&g_transition_queue_input.records[index], (int)(0x10 + index),
            sizeof(g_transition_queue_input.records[index]));
    }

    func_8001B044(&source);

    assert(g_transition_queue_input.count == 4);
    assert(memcmp(&g_transition_queue_input.records[3], &expected, sizeof(expected)) == 0);
    assert(g_transition_queue_input.records[0].words[0] == 0x10101010u);
    assert(g_transition_queue_input.records[1].words[0] == 0x11111111u);
    assert(g_transition_queue_input.records[2].words[0] == 0x12121212u);
    assert(g_transition_queue_input.records[4].words[0] == 0xA5A5A5A5u);
}

static void test_zero_count_copies_first_record(void) {
    GameTransitionEventRecord source = {
        .event_id = 7,
        .mode = 2,
        .flags = -1,
        .heading = 0xFFFE,
        .payload.opaque = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00},
    };

    memset(&g_transition_queue_input, 0x5A, sizeof(g_transition_queue_input));
    g_transition_queue_input.count = 0;
    func_8001B044(&source);
    assert(g_transition_queue_input.count == 1);
    assert(memcmp(&g_transition_queue_input.records[0], &source, sizeof(source)) == 0);
    assert(g_transition_queue_input.records[1].words[0] == 0x5A5A5A5Au);
}

int main(void) {
    test_appends_complete_event();
    test_zero_count_copies_first_record();
    puts("Deferred event copy: complete record append and queue preservation passed.");
    return 0;
}
