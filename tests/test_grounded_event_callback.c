#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bubsy_grounded_state.h"
#include "grounded_event_records.h"
#include "level_records.h"

int16_t g_grounded_counter;
void *g_grounded_event;
int32_t g_level_record_count;
GameLevelRecord *volatile g_level_records;

static BubsyGroundedActor level_actor;
static BubsyGroundedActor fallback_actor;
static BubsyGroundedActor ignored_actor;
static GameLevelRecord level_record;
static GroundedEventRecord initial_event;
static GroundedEventRecord selected_event;
static GroundedEventRecord replacement_event;
static BubsyGroundedActor *expected_actor;
static GroundedEventRecord *expected_event;
static uint32_t released_flags;
static int mutate_counter;
static int select_null_event;
static unsigned int calls[3];
static unsigned int call_count;

static int16_t signed_halfword(uint16_t value) {
    return value <= INT16_MAX ? (int16_t)value : (int16_t)((int32_t)value - 0x10000);
}

void *func_80040FC0(int32_t index) {
    assert(index == 0);
    assert(call_count < 3);
    calls[call_count++] = 1;
    g_grounded_event = select_null_event ? 0 : &selected_event;
    return &fallback_actor;
}

int32_t func_8004FFEC(GroundedEventRecord *event) {
    assert(call_count < 3);
    calls[call_count++] = 2;
    assert(event == expected_event);
    expected_actor->state_10 = released_flags;
    expected_actor->state_14 = 0xDEADBEEFu;
    level_record.object_1C = expected_actor == &level_actor ? &fallback_actor : &level_actor;
    g_grounded_event = &replacement_event;
    if (mutate_counter) {
        g_grounded_counter = 17;
    }
    return 3;
}

void func_8003539C(void *actor, int32_t mode) {
    assert(call_count < 3);
    calls[call_count++] = 3;
    assert(actor == expected_actor);
    assert(mode == 1);
    assert(expected_actor->state_10 == (released_flags & ~0x220u));
    assert(expected_actor->state_14 == 0);
    assert(g_grounded_event == 0);
    assert(g_grounded_counter == (mutate_counter ? 17 : 0));
}

int main(void) {
    static const int32_t counts[] = {INT32_MIN, -1, 0, 1, INT32_MAX};
    BubsyGroundedState unused_state;
    uint32_t input;

    for (input = 0; input <= UINT16_MAX; ++input) {
        uint16_t reduced = (uint16_t)(input - 2u);
        uint16_t expected_counter = (reduced & 0x8000u) != 0 ? 0 : reduced;
        BubsyGroundedActor *other_actor;
        void *initial_event_pointer = (input & 0x10u) != 0 ? 0 : &initial_event;
        int32_t count = counts[input % (sizeof(counts) / sizeof(counts[0]))];

        memset(&level_actor, 0, sizeof(level_actor));
        memset(&fallback_actor, 0, sizeof(fallback_actor));
        memset(&ignored_actor, 0, sizeof(ignored_actor));
        memset(&level_record, 0, sizeof(level_record));
        level_actor.state_10 = 0x12345678u;
        level_actor.state_14 = 0x87654321u;
        fallback_actor.state_10 = 0x11223344u;
        fallback_actor.state_14 = 0x44332211u;
        ignored_actor.state_10 = 0xA5A5A5A5u;
        unused_state.id = 0xBEEF;
        level_record.object_1C = &level_actor;
        g_level_records = &level_record;
        g_level_record_count = count;
        g_grounded_counter = signed_halfword((uint16_t)input);
        g_grounded_event = initial_event_pointer;
        expected_actor = count > 0 ? &level_actor : &fallback_actor;
        other_actor = count > 0 ? &fallback_actor : &level_actor;
        select_null_event = (input & 0x20u) != 0;
        expected_event = count > 0 ? (GroundedEventRecord *)initial_event_pointer
                                   : (select_null_event ? 0 : &selected_event);
        released_flags = 0xFEED0123u ^ input;
        mutate_counter = (input & 0x08u) != 0;
        call_count = 0;

        func_8004B2A4(&ignored_actor, &unused_state, &level_record, -123);

        assert(ignored_actor.state_10 == 0xA5A5A5A5u);
        assert(unused_state.id == 0xBEEF);
        if (expected_counter != 0) {
            assert(call_count == 0);
            assert(g_grounded_counter == signed_halfword(expected_counter));
            assert(g_grounded_event == initial_event_pointer);
            assert(level_actor.state_10 == 0x12345678u && level_actor.state_14 == 0x87654321u);
            assert(fallback_actor.state_10 == 0x11223344u && fallback_actor.state_14 == 0x44332211u);
            assert(level_record.object_1C == &level_actor);
        } else {
            unsigned int first = count > 0 ? 0 : 1;
            assert(call_count == first + 2);
            if (first != 0) {
                assert(calls[0] == 1);
            }
            assert(calls[first] == 2 && calls[first + 1] == 3);
            assert(g_grounded_counter == (mutate_counter ? 17 : 0));
            assert(g_grounded_event == 0);
            assert(expected_actor->state_10 == (released_flags & ~0x220u));
            assert(expected_actor->state_14 == 0);
            if (other_actor == &level_actor) {
                assert(other_actor->state_10 == 0x12345678u && other_actor->state_14 == 0x87654321u);
            } else {
                assert(other_actor->state_10 == 0x11223344u && other_actor->state_14 == 0x44332211u);
            }
        }
    }

    puts("Grounded event callback: all 65536 counter values passed.");
    return 0;
}
