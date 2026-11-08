#include "bubsy_grounded_turn.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t g_stride_marker;
uint8_t g_stride_mode;
uint8_t g_turn_direction;
uint8_t g_runtime_state_61;
int16_t g_turn_counter;
const char g_stride_error_condition[] = "turn sequence";
const char g_stride_source_path[] = "../f/turn.c";

static BubsyGroundedActor actor;
static BubsyGroundedState state;
static BubsyGroundedTurnShared captured_shared;
static BubsyGroundedTurnShared replacement_shared;
static int component;
static uint32_t sample;
static int32_t sequence;
static int32_t selection_result;
static int32_t selected_sequence;
static uint32_t assertion_line;
static unsigned int time_calls;
static unsigned int clear_calls;
static unsigned int set_calls;
static unsigned int get_sequence_calls;
static unsigned int select_calls;
static unsigned int start_calls;
static unsigned int assertion_calls;
static int32_t last_set_selector;
static int replace_shared_on_clear;

void func_8001AE88(uint32_t *value) {
    ++time_calls;
    *value = sample;
}

void func_8002262C(BubsyGroundedActor *value, int32_t selector) {
    assert(value == &actor);
    assert(selector == 5);
    ++clear_calls;
    actor.state_10 &= ~(1u << selector);
    if (replace_shared_on_clear) {
        actor.shared_28 = (BubsyGroundedShared *)(void *)&replacement_shared;
    }
}

void func_80022650(BubsyGroundedActor *value, int32_t selector) {
    assert(value == &actor);
    ++set_calls;
    last_set_selector = selector;
    actor.state_10 |= 1u << selector;
}

void func_80052ABC(void *value, int32_t *result) {
    assert(value == &component);
    ++get_sequence_calls;
    *result = sequence;
}

int32_t func_80052AEC(void *value, int32_t selection) {
    assert(value == &component);
    ++select_calls;
    selected_sequence = selection;
    return selection_result;
}

void func_80052E18(void *value) {
    assert(value == &component);
    ++start_calls;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 1);
    assert(condition == g_stride_error_condition);
    assert(source == g_stride_source_path);
    ++assertion_calls;
    assertion_line = line;
}

static void reset(void) {
    memset(&actor, 0, sizeof(actor));
    memset(&state, 0, sizeof(state));
    memset(&captured_shared, 0, sizeof(captured_shared));
    memset(&replacement_shared, 0, sizeof(replacement_shared));
    actor.component_1C = &component;
    actor.shared_28 = (BubsyGroundedShared *)(void *)&captured_shared;
    captured_shared.turn_4C = 1234;
    replacement_shared.turn_4C = 5678;
    state.id = 8;
    g_turn_direction = 1;
    g_stride_marker = 1;
    g_stride_mode = 0;
    g_runtime_state_61 = 0;
    g_turn_counter = 0;
    sample = 0;
    sequence = 0;
    selection_result = 0;
    selected_sequence = -1;
    assertion_line = 0;
    time_calls = clear_calls = set_calls = 0;
    get_sequence_calls = select_calls = start_calls = assertion_calls = 0;
    last_set_selector = -1;
    replace_shared_on_clear = 0;
}

static void run(int32_t counter) {
    func_80034EF8(&actor, &state, 0, counter);
    assert(g_stride_marker == 0);
}

static void test_timed_modes(void) {
    reset();
    g_stride_mode = 1;
    actor.state_10 = 0x208;
    actor.state_14 = UINT32_MAX - 2;
    sample = 3;
    run(0);
    assert(time_calls == 1 && set_calls == 0);
    assert(actor.state_14 == 3 && captured_shared.turn_4C == 39);

    reset();
    g_stride_mode = 1;
    actor.state_10 = 0x40;
    actor.state_14 = 10;
    sample = 14;
    run(0);
    assert(actor.state_14 == 10 && captured_shared.turn_4C == 1234);
    sample = 15;
    run(0);
    assert(actor.state_14 == 15 && captured_shared.turn_4C == 28);

    reset();
    g_stride_mode = 1;
    actor.state_10 = 0x8;
    sample = 999;
    run(0);
    assert(time_calls == 2 && set_calls == 1 && last_set_selector == 9);
    assert(actor.state_14 == 999 && captured_shared.turn_4C == 1234);

    reset();
    g_stride_mode = 1;
    state.id = 9;
    g_turn_direction = 0;
    run(0);
    assert(set_calls == 1 && last_set_selector == 5);
    assert(captured_shared.turn_4C == -22);
}

static void test_scaled_turn(void) {
    static const int32_t inputs[] = {-32768, -1, 0, 4, 5, 6, 32767};
    unsigned int index;
    for (index = 0; index < sizeof(inputs) / sizeof(inputs[0]); ++index) {
        int32_t value = inputs[index];
        double scale = value < 5 ? value * 0.2 : (value - 5) * 0.06 + 1.0;
        int32_t expected = (int32_t)(scale * 4096.0) / 360;
        reset();
        actor.state_10 = 0x208;
        run(value);
        assert(captured_shared.turn_4C == (expected == 0 ? 1234 : expected));
        assert(set_calls == 0 && time_calls == 0);
    }
}

static void test_sequence_switch(void) {
    static const struct {
        uint16_t state_id;
        int32_t before;
        int32_t after;
    } cases[] = {
        {8, 0x2B8, 0x2B5}, {8, 0x2BB, 0x2BB},
        {9, 0x2BB, 0x2B5}, {9, 0x2B8, 0x2B8},
    };
    unsigned int index;
    for (index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        reset();
        state.id = cases[index].state_id;
        g_turn_direction = state.id == 8;
        actor.state_10 = 0x40;
        sequence = cases[index].before;
        selection_result = 1;
        run(0);
        assert(get_sequence_calls == 1 && select_calls == 1 && start_calls == 1);
        assert(selected_sequence == cases[index].after);
        assert(assertion_calls == 1 && assertion_line == 0x375);
        assert(set_calls == 1 && last_set_selector == 10);
        assert(captured_shared.turn_4C == (state.id == 8 ? 22 : -22));
    }

    reset();
    g_stride_mode = 2;
    actor.state_10 = 0x60;
    g_turn_counter = 4;
    run(0);
    assert(get_sequence_calls == 1 && select_calls == 0);
    assert(g_turn_counter == 5 && captured_shared.turn_4C == 15);
}

static void test_state_entry_and_counter(void) {
    reset();
    g_turn_counter = 17;
    selection_result = -1;
    run(0);
    assert(set_calls == 1 && last_set_selector == 5 && g_turn_counter == 0);
    assert(select_calls == 1 && selected_sequence == 0x288 && start_calls == 1);
    assert(assertion_calls == 1 && assertion_line == 0x38E);
    assert(captured_shared.turn_4C == 1234);

    reset();
    state.id = 9;
    g_turn_direction = 0;
    selection_result = -1;
    run(0);
    assert(selected_sequence == 0x28E && assertion_line == 0x397);

    reset();
    g_runtime_state_61 = 2;
    run(0);
    assert(select_calls == 0 && start_calls == 0);

    reset();
    actor.state_10 = 0x20;
    sequence = 0x309;
    selection_result = -1;
    g_turn_counter = 45;
    run(0);
    assert(select_calls == 1 && selected_sequence == 0x288 && start_calls == 1);
    assert(assertion_calls == 0 && g_turn_counter == 46);
    assert(captured_shared.turn_4C == 56);

    reset();
    actor.state_10 = 0x20;
    actor.flags_04 = 0x80;
    g_turn_counter = -11;
    run(0);
    assert(select_calls == 0 && g_turn_counter == -10);
    assert(captured_shared.turn_4C == 1234);

    reset();
    actor.state_10 = 0x20;
    g_turn_counter = 7;
    g_runtime_state_61 = 2;
    run(0);
    assert(g_turn_counter == 7 && captured_shared.turn_4C == 22);
}

static void test_captured_shared_pointer(void) {
    reset();
    state.id = 9;
    actor.state_10 = 0x20;
    g_stride_mode = 1;
    replace_shared_on_clear = 1;
    run(0);
    assert(clear_calls == 1 && g_turn_direction == 0);
    assert(actor.shared_28 == (BubsyGroundedShared *)(void *)&replacement_shared);
    assert(captured_shared.turn_4C == -22 && replacement_shared.turn_4C == 5678);
}

int main(void) {
    test_timed_modes();
    test_scaled_turn();
    test_sequence_switch();
    test_state_entry_and_counter();
    test_captured_shared_pointer();
    puts("grounded turn behavior tests passed");
    return 0;
}
