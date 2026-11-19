#include "bubsy_actor_state_update.h"
#include "bubsy_grounded_state.h"
#include "game_transition.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int16_t g_actor_update_phase;
int32_t g_actor_update_counter;
int32_t g_actor_message_index;
int16_t g_actor_environment;
uint8_t g_actor_update_marker;
int32_t g_actor_update_value;
uint8_t g_stride_marker;
uint8_t g_stride_mode;
const char *g_actor_message_strings[] = {"zero", "one", "two", "three", "four", "five"};
const char g_stride_error_condition[] = "actor sequence";
const char g_stride_source_path[] = "../f/bubsy.c";

static BubsyGroundedActor actor;
static int component;
static int32_t sequence;
static int32_t selection_result;
static int32_t selected_sequence;
static int32_t random_value;
static int32_t ready_result;
static int32_t animation_state;
static uint8_t boundary_result;
static int change_boundary_globals;
static unsigned int animation_calls;
static unsigned int select_calls;
static unsigned int restart_calls;
static unsigned int sequence_mode_calls;
static unsigned int boundary_calls;
static unsigned int assertion_calls;
static unsigned int activate_calls;
static unsigned int begin_calls;
static unsigned int ready_calls;
static unsigned int copy_calls;
static unsigned int event_count;
static int32_t assertion_level;
static uint32_t assertion_line;
static int32_t events[4];
static const char *copied_string;

void func_80037214(int32_t mode, BubsyGroundedActor *value) {
    assert(mode == 0 && value == &actor);
    ++animation_calls;
    if (animation_state != -1) {
        actor.state_10 = (uint32_t)animation_state;
    }
}

void func_80052ABC(void *value, int32_t *output) {
    assert(value == &component);
    *output = sequence;
}

int32_t func_80052AEC(void *value, int32_t selected) {
    assert(value == &component);
    ++select_calls;
    selected_sequence = selected;
    return selection_result;
}

void func_80052E18(void *value) {
    assert(value == &component);
    ++restart_calls;
}

int32_t func_80052BDC(void *value, int32_t mode) {
    assert(value == &component && mode == 0);
    ++sequence_mode_calls;
    return 0;
}

void func_8005290C(void *value, uint8_t *output) {
    assert(value == &component);
    ++boundary_calls;
    *output = boundary_result;
    if (change_boundary_globals) {
        g_stride_mode = 1;
        g_actor_update_counter = 10;
        g_actor_environment = 0;
    }
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(condition == g_stride_error_condition && source == g_stride_source_path);
    ++assertion_calls;
    assertion_level = failed;
    assertion_line = line;
}

void func_80026A0C(int32_t mode, int32_t value) {
    assert(mode == 7 && value == 0);
    ++activate_calls;
}

void func_8002C488(void) {
    ++begin_calls;
}

int32_t func_800222E0(void) {
    ++ready_calls;
    return ready_result;
}

int32_t func_80082ECC(void) {
    return random_value;
}

void func_80082E8C(char *destination, const char *source) {
    ++copy_calls;
    copied_string = source;
    memcpy(destination, "message", 8);
}

static uint32_t read_word(const uint8_t *bytes, unsigned int offset) {
    uint32_t value;
    memcpy(&value, bytes + offset, sizeof(value));
    return value;
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    assert(event_count < 4);
    events[event_count++] = event_id;
    if (event_id == 0x2C) {
        assert(mode == 4 && flags == 0xFFFE && heading == 0xFFFE);
        assert(*(int32_t *)argument_4 == -10 && *(uint8_t *)argument_5 == 0);
        assert(argument_5 == argument_6 && argument_6 == argument_7);
    } else if (event_id == 4) {
        const uint8_t *message = argument_4;
        assert(mode == 5 && flags == 0xFFFF && heading == 0xFFFF);
        assert(argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
        assert(memcmp(message, "message", 8) == 0);
        assert(read_word(message, 0x80) == 0x200 && read_word(message, 0x84) == 0xF0);
        assert(read_word(message, 0x88) == 0 && read_word(message, 0x8C) == 0);
        assert(read_word(message, 0x90) == UINT32_MAX);
        assert(message[0x94] == 0 && message[0x95] == 0 && message[0x96] == 0);
        assert(message[0x98] == 0x28 && message[0xA5] == 1 && message[0xA6] == 0);
        assert(read_word(message, 0x9C) == 0xFFFE && read_word(message, 0xA0) == 0xFFFE);
    } else {
        assert(event_id == 0x16 && mode == 4);
        assert(flags == actor.heading_0C && heading == (int32_t)actor.heading_0C);
        assert(argument_4 == 0 && argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    }
}

static void reset(void) {
    memset(&actor, 0, sizeof(actor));
    memset(events, 0, sizeof(events));
    actor.component_1C = &component;
    actor.heading_0C = 0x1234;
    g_actor_update_phase = 0;
    g_actor_update_counter = 0;
    g_actor_message_index = 0;
    g_actor_environment = 0;
    g_actor_update_marker = 1;
    g_actor_update_value = 123;
    g_stride_marker = 1;
    g_stride_mode = 0;
    sequence = 0;
    selection_result = 0;
    selected_sequence = -1;
    random_value = 0;
    ready_result = 0;
    animation_state = -1;
    boundary_result = 0;
    change_boundary_globals = 0;
    animation_calls = select_calls = restart_calls = sequence_mode_calls = 0;
    boundary_calls = assertion_calls = activate_calls = begin_calls = ready_calls = 0;
    copy_calls = event_count = 0;
    assertion_level = -1;
    assertion_line = 0;
    copied_string = 0;
}

static void test_entry_and_sequence_selection(void) {
    reset();
    actor.state_10 = 1;
    g_actor_update_counter = 7;
    func_8003539C(&actor, 1);
    assert(g_stride_marker == 0 && g_actor_update_counter == 0);
    assert(animation_calls == 0 && event_count == 0);

    reset();
    actor.flags_04 = 0x80;
    sequence = 0;
    selection_result = 1;
    func_8003539C(&actor, 0);
    assert(select_calls == 1 && selected_sequence == 0x309 && restart_calls == 1);
    assert(assertion_calls == 1 && assertion_level == 1 && assertion_line == 0x41A);
    assert(sequence_mode_calls == 0 && events[0] == 0x16);

    reset();
    actor.flags_04 = 0x80;
    animation_state = 0x800000;
    func_8003539C(&actor, 0);
    assert(select_calls == 0 && event_count == 1 && events[0] == 0x16);

    reset();
    sequence = 0x32E;
    selection_result = 1;
    func_8003539C(&actor, 0);
    assert(selected_sequence == 0x35 && restart_calls == 1 && sequence_mode_calls == 1);
    assert(assertion_calls == 1 && assertion_level == 0 && assertion_line == 0x495);
}

static void test_boundary_and_mode_mutation(void) {
    reset();
    sequence = 0x35;
    g_actor_update_counter = -1;
    boundary_result = 1;
    func_8003539C(&actor, 0);
    assert(boundary_calls == 1 && g_actor_update_counter == 0 && activate_calls == 1);

    reset();
    sequence = 0x35;
    actor.flags_04 = 0x40;
    boundary_result = 1;
    func_8003539C(&actor, 0);
    assert(boundary_calls == 1 && g_actor_update_counter == 0 && activate_calls == 1);
    assert(selected_sequence == 0x35);

    reset();
    sequence = 0x35;
    boundary_result = 1;
    change_boundary_globals = 1;
    func_8003539C(&actor, 0);
    assert(g_stride_mode == 1 && g_actor_update_counter == 11 && activate_calls == 1);
    assert(g_actor_update_value == -0xF40 && selected_sequence == 0x54);

    reset();
    g_stride_mode = 1;
    g_actor_update_counter = 11;
    sequence = 0x9A;
    boundary_result = 1;
    actor.flags_04 = 0x818;
    func_8003539C(&actor, 0);
    assert(g_actor_update_counter == 0 && activate_calls == 0);
    assert(selected_sequence == 0x9A);
}

static void test_update_phases_and_payload(void) {
    reset();
    g_actor_update_counter = 1;
    sequence = 0x35;
    func_8003539C(&actor, 0);
    assert(boundary_calls == 0);
    assert(g_actor_update_phase == 3 && begin_calls == 1);
    assert(event_count == 2 && events[0] == 0x2C && events[1] == 0x16);

    reset();
    g_actor_update_counter = 1;
    g_actor_environment = 0x14;
    func_8003539C(&actor, 0);
    assert(g_actor_update_phase == 0 && begin_calls == 0);
    assert(event_count == 1 && events[0] == 0x16);

    reset();
    g_actor_update_phase = 3;
    g_actor_update_counter = 1;
    actor.state_10 = 0xFFFFFFFFu;
    ready_result = 1;
    func_8003539C(&actor, 255);
    assert(actor.state_10 == 0 && ready_calls == 1 && g_actor_update_phase == 2);
    assert(event_count == 1 && events[0] == 0x16);

    reset();
    g_actor_update_phase = 2;
    g_actor_update_counter = 1;
    g_actor_message_index = 6;
    random_value = 13;
    func_8003539C(&actor, 0);
    assert(copy_calls == 1 && copied_string == g_actor_message_strings[1]);
    assert(g_actor_message_index == 6 && g_actor_update_phase == 0 && g_actor_update_counter == 0);
    assert(event_count == 2 && events[0] == 4 && events[1] == 0x16);
    assert(selected_sequence == 0x35);

    reset();
    g_actor_update_phase = 2;
    g_actor_update_counter = 1;
    g_actor_message_index = 5;
    func_8003539C(&actor, 0);
    assert(copied_string == g_actor_message_strings[5] && g_actor_message_index == 6);
}

int main(void) {
    test_entry_and_sequence_selection();
    test_boundary_and_mode_mutation();
    test_update_phases_and_payload();
    puts("actor-state update behavior tests passed");
    return 0;
}
