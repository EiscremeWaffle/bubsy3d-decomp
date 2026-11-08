#include <assert.h>
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
GroundedEventRecord *g_grounded_event_records;
uint8_t g_grounded_event_busy;

static BubsyGroundedActor level_actor;
static BubsyGroundedActor fallback_actor;
static BubsyGroundedActor *expected_actor;
static GameLevelRecord level_record;
static GroundedEventRecord replacement_event;
static GroundedEventRecord *freed_event;
static unsigned int fallback_calls;
static unsigned int update_calls;

void *func_80040FC0(int32_t index) {
    assert(index == 0);
    ++fallback_calls;
    return &fallback_actor;
}

void func_80082EFC(void *event) {
    assert(g_grounded_event_busy == 1);
    assert(freed_event == 0);
    freed_event = event;
    expected_actor->state_10 = 0xFEED0123u;
    expected_actor->state_14 = 0xDEADBEEFu;
    level_record.object_1C = &fallback_actor;
    g_grounded_event = &replacement_event;
    g_grounded_counter = 13;
}

void func_8003539C(void *actor, int32_t mode) {
    assert(actor == expected_actor);
    assert(mode == 1);
    assert(g_grounded_event == 0);
    assert(expected_actor->state_14 == 0);
    assert(expected_actor->state_10 ==
           ((freed_event != 0 ? 0xFEED0123u : 0x12345678u) & ~0x220u));
    assert(g_grounded_counter == (freed_event != 0 ? 13 : 0));
    ++update_calls;
}

int main(void) {
    unsigned int kind;

    for (kind = 0; kind < 5; ++kind) {
        GroundedEventRecord records[3];
        GroundedEventRecord missing;
        GroundedEventRecord *target;
        BubsyGroundedState ignored_state;

        memset(&level_actor, 0, sizeof(level_actor));
        memset(&fallback_actor, 0, sizeof(fallback_actor));
        memset(&level_record, 0, sizeof(level_record));
        memset(records, 0, sizeof(records));
        memset(&missing, 0, sizeof(missing));
        records[0].next = &records[1];
        records[1].next = &records[2];
        target = kind < 3 ? &records[kind] : (kind == 3 ? &missing : 0);
        g_grounded_event_records = &records[0];
        g_grounded_event_busy = 0xA5;
        g_grounded_event = target;
        g_grounded_counter = 3;
        g_level_record_count = kind % 2 == 0 ? 1 : 0;
        level_record.object_1C = &level_actor;
        g_level_records = &level_record;
        expected_actor = g_level_record_count > 0 ? &level_actor : &fallback_actor;
        expected_actor->state_10 = 0x12345678u;
        expected_actor->state_14 = 0x87654321u;
        ignored_state.id = 0xBEEF;
        freed_event = 0;
        fallback_calls = 0;
        update_calls = 0;

        func_8004B2A4(0, &ignored_state, 0, 0);
        assert(g_grounded_counter == 1);
        assert(g_grounded_event == target);
        assert(g_grounded_event_records == &records[0]);
        assert(g_grounded_event_busy == 0xA5);
        assert(fallback_calls == 0 && update_calls == 0 && freed_event == 0);

        func_8004B2A4(0, &ignored_state, 0, 0);
        assert(ignored_state.id == 0xBEEF);
        assert(update_calls == 1);
        assert(fallback_calls == (g_level_record_count > 0 ? 0u : 1u));
        assert(freed_event == (kind < 3 ? target : 0));
        assert(g_grounded_event_busy == (kind == 4 ? 0xA5 : 0));
        assert(g_grounded_event_records == (kind == 0 ? &records[1] : &records[0]));
        assert(records[0].next == (kind == 1 ? &records[2] : &records[1]));
        assert(records[1].next == (kind == 2 ? 0 : &records[2]));
    }

    puts("Grounded event callback: real list release integration passed.");
    return 0;
}
