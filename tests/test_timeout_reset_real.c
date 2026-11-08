#include "game_transition.h"
#include "input_events.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

uint8_t g_event_input_gate;
uint8_t g_event_runtime_byte;
uint32_t global_0x800BDE98;
uint8_t g_timeout_event_id;
volatile uint32_t g_timeout_event_flags;
volatile int32_t g_timeout_event_heading;
const char g_sequence_query_condition[] = "condition";
const char g_transition_source_path[] = "transition source";

enum CallKind { CHECK, CLEANUP, ACTOR, CONTEXT, GATE, BACKEND_45094, BACKEND_69758, RESET_EVENT, TIMEOUT_EVENT };
static enum CallKind calls[9];
static size_t call_count;
static uint8_t context[0x6E];
static uint8_t actor[16];
static uint8_t replacement_id;

static void record(enum CallKind kind) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = kind;
}

int32_t func_80062890(void) {
    record(CHECK);
    return 0;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(0 && "unexpected assertion callback");
}

void func_80048488(void) {
    record(CLEANUP);
}

void *func_80028764(int32_t index) {
    record(ACTOR);
    assert(index == 0 && g_event_runtime_byte == 0x5A);
    return actor;
}

int32_t func_8003AA1C(void **output) {
    record(CONTEXT);
    assert(g_event_runtime_byte == 0);
    *output = context;
    return 1;
}

int32_t func_80045278(void) {
    record(GATE);
    return 1;
}

void func_80045094(void) {
    record(BACKEND_45094);
    assert(global_0x800BDE98 == 0);
}

void func_80069758(void) {
    record(BACKEND_69758);
    assert(global_0x800BDE98 == 0);
}

void func_8001B558(int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
                   void *argument_4, void *argument_5, void *argument_6, void *argument_7) {
    assert(argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    if (mode == 1) {
        record(RESET_EVENT);
        assert(event_id == 4 && flags == 0xFFFE && heading == 0xFFFE);
        assert((uintptr_t)argument_4 == (uintptr_t)actor + 0x0C);
        g_timeout_event_id = replacement_id;
        g_timeout_event_flags = 0xAABBCCDDu;
        g_timeout_event_heading = INT32_MIN;
        g_event_input_gate = 0xA5;
    } else {
        record(TIMEOUT_EVENT);
        assert(mode == 0 && event_id == replacement_id);
        assert(flags == 0xAABBCCDDu && heading == INT32_MIN && argument_4 == 0);
    }
}

int main(void) {
    static const uint8_t ids[] = {0, 4, 9, 0x16, 0xFD, 0xFE, 0xFF};
    static const enum CallKind prefix[] = {
        CHECK, CLEANUP, ACTOR, CONTEXT, GATE, BACKEND_45094, BACKEND_69758, RESET_EVENT
    };
    size_t index, call;

    for (index = 0; index < sizeof(ids) / sizeof(ids[0]); ++index) {
        memset(context, 0, sizeof(context));
        replacement_id = ids[index];
        g_timeout_event_id = 7;
        g_timeout_event_flags = 0;
        g_timeout_event_heading = 0;
        g_event_input_gate = 0x5A;
        g_event_runtime_byte = 0x5A;
        global_0x800BDE98 = UINT32_MAX;
        call_count = 0;

        assert(func_8001C04C() == 0);
        assert(call_count == sizeof(prefix) / sizeof(prefix[0]) + (ids[index] < 0xFE));
        for (call = 0; call < sizeof(prefix) / sizeof(prefix[0]); ++call) {
            assert(calls[call] == prefix[call]);
        }
        if (ids[index] < 0xFE) {
            assert(calls[call] == TIMEOUT_EVENT);
        }
        assert(global_0x800BDE98 == 0);
        assert(g_event_runtime_byte == 0);
        assert(g_event_input_gate == 0xA5);
    }
    puts("timeout and real event-reset integration tests passed");
    return 0;
}
