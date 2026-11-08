#include "game_transition.h"
#include "input_events.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

uint8_t g_event_runtime_byte;
uint32_t global_0x800BDE98;

enum CallKind { ACTOR_LOOKUP, CONTEXT_LOOKUP, GATE_CHECK, BACKEND_45094, BACKEND_69758, EVENT };

static enum CallKind calls[6];
static size_t call_count;
static uint8_t context[0x6E];
static uint8_t actor_storage[16];
static void *actor_result;
static int32_t gate_result;
static uint32_t expected_count;

static void record(enum CallKind call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

void *func_80028764(int32_t index) {
    record(ACTOR_LOOKUP);
    assert(index == 0);
    assert(g_event_runtime_byte == 0x5A);
    g_event_runtime_byte = 0xA5;
    return actor_result;
}

int32_t func_8003AA1C(void **output) {
    record(CONTEXT_LOOKUP);
    assert(g_event_runtime_byte == 0);
    *output = context;
    return 1;
}

int32_t func_80045278(void) {
    record(GATE_CHECK);
    return gate_result;
}

void func_80045094(void) {
    record(BACKEND_45094);
    assert(global_0x800BDE98 == expected_count);
}

void func_80069758(void) {
    record(BACKEND_69758);
    assert(global_0x800BDE98 == expected_count);
}

void func_8001B558(int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
                   void *argument_4, void *argument_5, void *argument_6, void *argument_7) {
    record(EVENT);
    assert(event_id == 4 && mode == 1 && flags == 0xFFFE && heading == 0xFFFE);
    assert((uintptr_t)argument_4 == (uintptr_t)actor_result + 0x0C);
    assert(argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    assert(global_0x800BDE98 == expected_count);
}

static void test_reset(void) {
    static const uint8_t flags[] = {0, 1, 255};
    static const int32_t results[] = {0, 1, 0x10000, INT32_MIN, -1};
    static const uint32_t counts[] = {0, UINT32_MAX};
    size_t a, b, result, count, actor;

    for (a = 0; a < sizeof(flags) / sizeof(flags[0]); ++a) {
        for (b = 0; b < sizeof(flags) / sizeof(flags[0]); ++b) {
            for (result = 0; result < sizeof(results) / sizeof(results[0]); ++result) {
                for (count = 0; count < sizeof(counts) / sizeof(counts[0]); ++count) {
                    for (actor = 0; actor < 2; ++actor) {
                        int check_gate = flags[a] == 0 && flags[b] == 0;
                        int triggered = check_gate && ((uint32_t)results[result] & 0xFFFFu) != 0;
                        size_t next = 0;

                        memset(context, 0, sizeof(context));
                        context[0x6D] = flags[a];
                        context[0x2B] = flags[b];
                        actor_result = actor ? actor_storage : 0;
                        gate_result = results[result];
                        global_0x800BDE98 = counts[count];
                        expected_count = counts[count] + (uint32_t)triggered;
                        g_event_runtime_byte = 0x5A;
                        call_count = 0;

                        func_8001BC28();

                        assert(calls[next++] == ACTOR_LOOKUP);
                        assert(calls[next++] == CONTEXT_LOOKUP);
                        if (check_gate) {
                            assert(calls[next++] == GATE_CHECK);
                        }
                        if (triggered) {
                            assert(calls[next++] == BACKEND_45094);
                            assert(calls[next++] == BACKEND_69758);
                            assert(calls[next++] == EVENT);
                        }
                        assert(call_count == next);
                        assert(global_0x800BDE98 == expected_count);
                        assert(g_event_runtime_byte == 0);
                    }
                }
            }
        }
    }
}

int main(void) {
    test_reset();
    puts("event-runtime reset tests passed");
    return 0;
}
