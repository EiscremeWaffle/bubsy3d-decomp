#include "controller_callback_helpers.h"
#include "heading_callback_gates.h"
#include "heading_entry_gates.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

volatile uint8_t g_grounded_case_event;
int16_t g_actor_environment;
uint8_t g_heading_context_gate;
const char g_heading_gate_condition[] = "condition";
const char g_heading_gate_source_path[] = "source";
uint32_t g_unknown_8009F58C;
uint32_t global_0x800BDE98;
uint32_t g_heading_previous_counter;
uint8_t g_heading_stop_byte;
uint8_t g_heading_active_byte;
void *g_actor_handles[2];

static GameTransitionPayload *current_event;
static unsigned calls;
static unsigned activate_on_callback;
static uint16_t event_after_cleanup;
static int first_handle;
static int second_handle;
static int replacement_handle;

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(0);
}

void func_8004733C(int32_t mode) {
    assert(mode == 1);
    ++calls;
    if (activate_on_callback) {
        g_heading_stop_byte = 1;
    }
}

int32_t func_80052BDC(void *handle, int32_t mode) {
    assert(mode == 1);
    if (handle == &first_handle) {
        g_actor_handles[1] = &replacement_handle;
        current_event->event_id = event_after_cleanup;
    } else {
        assert(handle == &replacement_handle);
    }
    ++calls;
    return 0;
}

typedef struct GateCase {
    int16_t environment;
    uint8_t context_gate;
    uint8_t stop;
    uint8_t active;
    uint32_t word;
    uint16_t event;
    uint32_t counter;
    uint32_t previous;
    uint8_t activate;
    uint16_t after_cleanup;
    uint8_t changed;
    uint8_t proceed;
    unsigned expected_calls;
    uint8_t expected_active;
} GateCase;

int main(void) {
    static const GateCase cases[] = {
        {18, 1, 0, 1, 0, 8, 2, 1, 0, 0, 1, 1, 0, 1},
        {19, 1, 0, 0, 0, 8, 2, 2, 0, 0, 0, 1, 1, 0},
        {19, 1, 0, 1, 0, 8, 2, 1, 1, 46, 1, 1, 3, 0},
        {20, 1, 1, 1, 1, 46, 4, 4, 0, 8, 0, 0, 2, 0},
        {19, 1, 0, 0, 1, 8, 0, 0, 0, 0, 0, 1, 0, 0},
        {19, 2, 0, 0, 0, 8, 0, 0, 0, 0, 0, 1, 0, 0},
        {19, 1, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0},
        {19, 1, 0, 0, 0x80000000U, 46, 0, 0, 1, 0, 0, 1, 1, 0},
    };
    unsigned index;

    for (index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        const GateCase *test = &cases[index];
        GameTransitionPayload event;
        HeadingEntryDecision decision;

        memset(&event, 0, sizeof(event));
        event.event_id = test->event;
        current_event = &event;
        g_actor_environment = test->environment;
        g_heading_context_gate = test->context_gate;
        g_heading_stop_byte = test->stop;
        g_heading_active_byte = test->active;
        g_unknown_8009F58C = test->word;
        global_0x800BDE98 = test->counter;
        g_heading_previous_counter = test->previous;
        g_actor_handles[0] = &first_handle;
        g_actor_handles[1] = &second_handle;
        activate_on_callback = test->activate;
        event_after_cleanup = test->after_cleanup;
        calls = 0;

        decision = heading_entry_gates(&event);
        assert(decision.counter_changed == test->changed);
        assert(decision.proceed == test->proceed);
        assert(g_heading_previous_counter == test->counter);
        assert(g_heading_active_byte == test->expected_active);
        assert(calls == test->expected_calls);
    }
    puts("heading entry gate tests passed");
    return 0;
}
