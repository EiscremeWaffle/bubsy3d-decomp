#include "game_transition.h"
#include "input_events.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

uint8_t g_event_input_gate;
uint8_t g_timeout_event_id;
volatile uint32_t g_timeout_event_flags;
volatile int32_t g_timeout_event_heading;
const char g_sequence_query_condition[] = "condition";
const char g_transition_source_path[] = "transition source";

enum CallKind { CHECK, ASSERTION, CLEANUP, RESET, EVENT };
typedef struct Call {
    enum CallKind kind;
    int32_t event_id;
    uintptr_t flags;
    int32_t heading;
} Call;

static Call calls[5];
static size_t call_count;
static int32_t check_result;
static int check_replacement;
static int assertion_replacement;
static int cleanup_replacement;
static int reset_replacement;
static int reset_mutates_payload;

static void record(enum CallKind kind, int32_t event_id, uintptr_t flags, int32_t heading) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count].kind = kind;
    calls[call_count].event_id = event_id;
    calls[call_count].flags = flags;
    calls[call_count].heading = heading;
    ++call_count;
}

int32_t func_80062890(void) {
    record(CHECK, 0, 0, 0);
    if (check_replacement >= 0) {
        g_timeout_event_id = (uint8_t)check_replacement;
    }
    return check_result;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    record(ASSERTION, 0, 0, 0);
    assert(failed == 0 && condition == g_sequence_query_condition);
    assert(source == g_transition_source_path && line == 0x49A);
    if (assertion_replacement >= 0) {
        g_timeout_event_id = (uint8_t)assertion_replacement;
    }
}

void func_80048488(void) {
    record(CLEANUP, 0, 0, 0);
    if (cleanup_replacement >= 0) {
        g_timeout_event_id = (uint8_t)cleanup_replacement;
    }
}

void func_8001BC28(void) {
    record(RESET, 0, 0, 0);
    assert(g_event_input_gate == 0);
    if (reset_replacement >= 0) {
        g_timeout_event_id = (uint8_t)reset_replacement;
    }
    if (reset_mutates_payload) {
        g_timeout_event_flags = 0xCAFEBABEu;
        g_timeout_event_heading = INT32_MIN;
        g_event_input_gate = 0xA5;
    }
}

void func_8001B558(int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
                   void *argument_4, void *argument_5, void *argument_6, void *argument_7) {
    record(EVENT, event_id, flags, heading);
    assert(mode == 0);
    assert(argument_4 == 0 && argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
}

static void reset_fixture(uint8_t event_id, int32_t status) {
    g_event_input_gate = 0x5A;
    g_timeout_event_id = event_id;
    g_timeout_event_flags = 0x80000000u;
    g_timeout_event_heading = -1;
    check_result = status;
    check_replacement = -1;
    assertion_replacement = -1;
    cleanup_replacement = -1;
    reset_replacement = -1;
    reset_mutates_payload = 0;
    call_count = 0;
}

static void expect_call(size_t *next, enum CallKind kind) {
    assert(*next < call_count);
    assert(calls[*next].kind == kind);
    ++*next;
}

static void test_all_ids(void) {
    static const int32_t statuses[] = {0, 1, -1};
    unsigned int id;
    size_t status;
    for (id = 0; id < 256; ++id) {
        for (status = 0; status < sizeof(statuses) / sizeof(statuses[0]); ++status) {
            size_t next = 0;
            reset_fixture((uint8_t)id, statuses[status]);
            assert(func_8001C04C() == 0);
            if (id != 0xFE && id != 0x16) {
                expect_call(&next, CHECK);
                if (statuses[status] != 0) {
                    expect_call(&next, ASSERTION);
                }
                expect_call(&next, CLEANUP);
            }
            if (id != 0xFE) {
                expect_call(&next, RESET);
            }
            if (id != 0xFE && id != 0xFF) {
                expect_call(&next, EVENT);
                assert(calls[next - 1].event_id == (int32_t)id);
                assert(calls[next - 1].flags == 0x80000000u);
                assert(calls[next - 1].heading == -1);
            }
            assert(call_count == next);
            assert(g_event_input_gate == 0);
        }
    }
}

static void test_callback_mutations(void) {
    static const struct {
        uint8_t initial;
        int cleanup;
        int reset;
        int expected_event;
    } cases[] = {
        {1, 0xFE, -1, -1},
        {1, 0xFF, 3, 3},
        {0x16, -1, 0xFF, -1},
        {0x16, -1, 0xFD, 0xFD},
    };
    size_t index;
    for (index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        size_t event_count = 0;
        size_t call;
        reset_fixture(cases[index].initial, 0);
        cleanup_replacement = cases[index].cleanup;
        reset_replacement = cases[index].reset;
        reset_mutates_payload = cases[index].reset >= 0;
        assert(func_8001C04C() == 0);
        for (call = 0; call < call_count; ++call) {
            if (calls[call].kind == EVENT) {
                ++event_count;
                assert(calls[call].event_id == cases[index].expected_event);
                assert(calls[call].flags == 0xCAFEBABEu);
                assert(calls[call].heading == INT32_MIN);
            }
        }
        assert(event_count == (size_t)(cases[index].expected_event >= 0));
        assert(g_event_input_gate == (reset_mutates_payload ? 0xA5 : 0));
    }

    reset_fixture(1, 1);
    check_replacement = 0xFE;
    assertion_replacement = 0xFF;
    cleanup_replacement = 7;
    reset_replacement = 8;
    reset_mutates_payload = 1;
    assert(func_8001C04C() == 0);
    assert(call_count == 5);
    assert(calls[0].kind == CHECK && calls[1].kind == ASSERTION);
    assert(calls[2].kind == CLEANUP && calls[3].kind == RESET);
    assert(calls[4].kind == EVENT && calls[4].event_id == 8);
    assert(calls[4].flags == 0xCAFEBABEu && calls[4].heading == INT32_MIN);
    assert(g_event_input_gate == 0xA5);
}

int main(void) {
    test_all_ids();
    test_callback_mutations();
    puts("timeout dispatch tests passed");
    return 0;
}
