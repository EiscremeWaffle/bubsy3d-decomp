#include "bubsy_actor_state_update.h"
#include "bubsy_grounded_interaction.h"
#include "game_transition.h"
#include "sequence_queries.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

BubsyInteractionState g_interaction_state;
int32_t g_interaction_previous_phase;
uint8_t g_interaction_active;
uint8_t g_interaction_marker;
uint8_t g_stride_mode;
GameLevelRecord *volatile g_level_records;

typedef struct RecordedEvent {
    int32_t event_id;
    uintptr_t flags;
    int32_t heading;
} RecordedEvent;

static BubsyGroundedActor actor;
static BubsyGroundedShared shared;
static BubsyInteractionTarget target;
static BubsyInteractionTarget confirmed;
static BubsyInteractionComponent component;
static BubsyInteractionRecord records[3];
static int target_transform;
static int context;
static int reference;
static int32_t context_position[3];
static int32_t target_position[3];
static int32_t written_context[3];
static int32_t written_target[3];
static void *orient_left;
static void *orient_right;
static BubsyInteractionTarget *interaction_target;
static RecordedEvent events[4];
static unsigned int event_count;
static unsigned int lookup_count;
static unsigned int poll_count;
static unsigned int clear_count;
static unsigned int set_count;
static unsigned int update_count;
static unsigned int reset_count;
static unsigned int start_count;
static unsigned int interaction_count;
static uint8_t poll_results[2];
static uint8_t interaction_result;
static int32_t interaction_mode;
static int change_phase_on_poll;
static unsigned int change_phase_poll_number;
static int change_mode_on_context_write;
static int change_index_on_event;
static int change_heading_on_interaction;
static int change_heading_on_first_dispatch;

void *func_80028764(int32_t index) {
    assert(index == 1 || index == 2);
    ++lookup_count;
    return lookup_count == 1 ? &target : &confirmed;
}

int32_t func_8001C700(int32_t selection, int32_t value) {
    assert(selection == 5 && value == 1);
    if (change_index_on_event) {
        g_interaction_state.target_index = 2;
    }
    return 0;
}

void func_80037214(int32_t mode, BubsyGroundedActor *value) {
    assert((mode == 0 || mode == 1) && value == &actor);
}

void func_8002262C(BubsyGroundedActor *value, int32_t selector) {
    assert(value == &actor);
    ++clear_count;
    actor.state_10 &= ~(1u << selector);
}

void func_80022650(BubsyGroundedActor *value, int32_t selector) {
    assert(value == &actor && selector == 17);
    ++set_count;
    actor.state_10 |= 1u << selector;
}

void func_8003539C(void *value, int32_t mode) {
    assert(value == &actor && mode == 1);
    ++update_count;
}

int32_t func_80054B20(void *transform, int32_t mode, int32_t *position) {
    assert(mode == 0);
    assert(transform == &target_transform || transform == &context);
    memcpy(position, transform == &context ? context_position : target_position, sizeof(context_position));
    return 0;
}

int32_t func_80054C7C(void *transform, int32_t mode, const int32_t *position) {
    assert(mode == 0);
    assert(transform == &target_transform || transform == &context);
    memcpy(transform == &context ? written_context : written_target, position, sizeof(written_context));
    if (transform == &context && change_mode_on_context_write) {
        g_stride_mode = 1;
    }
    return 0;
}

int32_t func_800516F8(void **output) {
    *output = &reference;
    return 0;
}

int32_t func_800550A4(void *left, void *right) {
    orient_left = left;
    orient_right = right;
    return 0;
}

int32_t func_8001C6BC(int32_t selection, int32_t value, int32_t enabled) {
    assert(selection == 5 && value == 1 && enabled == 1);
    ++start_count;
    return 1;
}

uint8_t func_8001C7C4(int32_t selection, int32_t value) {
    uint8_t result;
    assert(selection == 5 && value == 1 && poll_count < 2);
    result = poll_results[poll_count++];
    if (poll_count == change_phase_poll_number && change_phase_on_poll >= 0) {
        g_interaction_state.phase_05 = (uint8_t)change_phase_on_poll;
    }
    return result;
}

uint8_t func_80038C0C(
    BubsyGroundedActor *value,
    void *context_value,
    BubsyInteractionTarget *target_value,
    int32_t mode
) {
    assert(value == &actor && context_value == &context);
    ++interaction_count;
    interaction_target = target_value;
    interaction_mode = mode;
    if (change_heading_on_interaction) {
        actor.heading_0C = 0x5678;
    }
    return interaction_result;
}

void func_8002DBD4(int32_t mode, void *value) {
    assert(mode == 1 && value == 0);
    ++reset_count;
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    assert(mode == 4 && event_count < 4);
    assert(argument_4 == 0 && argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    events[event_count].event_id = event_id;
    events[event_count].flags = flags;
    events[event_count].heading = heading;
    ++event_count;
    if (event_count == 1 && change_heading_on_first_dispatch) {
        actor.heading_0C = 0xABCD;
    }
}

static void reset(void) {
    memset(&actor, 0, sizeof(actor));
    memset(&shared, 0, sizeof(shared));
    memset(&target, 0, sizeof(target));
    memset(&confirmed, 0, sizeof(confirmed));
    memset(&component, 0, sizeof(component));
    memset(records, 0, sizeof(records));
    memset(&g_interaction_state, 0, sizeof(g_interaction_state));
    memset(events, 0, sizeof(events));
    memset(written_context, 0, sizeof(written_context));
    memset(written_target, 0, sizeof(written_target));
    context_position[0] = 40;
    context_position[1] = 50;
    context_position[2] = 60;
    target_position[0] = 10;
    target_position[1] = 20;
    target_position[2] = 30;
    actor.shared_28 = &shared;
    actor.heading_0C = 0x1234;
    actor.state_10 = 0xFFFFFFFFu;
    shared.stride_30 = 1;
    shared.stride_delta_34 = 2;
    target.component_1C = &component;
    target.heading_0C = 0x4321;
    confirmed.component_1C = &component;
    confirmed.heading_0C = 0x8765;
    component.transform_14 = &target_transform;
    g_interaction_state.target_index = 1;
    g_interaction_previous_phase = -1;
    g_interaction_active = 0;
    g_interaction_marker = 1;
    g_stride_mode = 0;
    g_level_records = records;
    event_count = lookup_count = poll_count = clear_count = set_count = 0;
    update_count = reset_count = start_count = interaction_count = 0;
    poll_results[0] = poll_results[1] = 0;
    interaction_result = 0;
    interaction_mode = -1;
    interaction_target = 0;
    orient_left = orient_right = 0;
    change_phase_on_poll = -1;
    change_phase_poll_number = 1;
    change_mode_on_context_write = 0;
    change_index_on_event = 0;
    change_heading_on_interaction = 0;
    change_heading_on_first_dispatch = 0;
}

static void test_null_and_used_target(void) {
    reset();
    g_interaction_active = 1;
    func_80039158(0, 0);
    assert(g_interaction_active == 0 && lookup_count == 0);

    reset();
    g_interaction_state.phase_05 = 2;
    target.used_2C = 1;
    g_interaction_active = 1;
    func_80039158(&actor, &context);
    assert(g_interaction_active == 0 && g_interaction_state.phase_05 == 0);
    assert(clear_count == 2 && update_count == 1 && actor.state_10 == 0);
    assert(event_count == 0);
}

static void test_position_paths(void) {
    reset();
    g_interaction_state.phase_05 = 2;
    change_mode_on_context_write = 1;
    func_80039158(&actor, &context);
    assert(g_interaction_active == 1 && set_count == 1 && start_count == 1);
    assert(actor.state_10 == (1u << 17));
    assert(shared.stride_30 == 0 && shared.stride_delta_34 == 0);
    assert(written_context[0] == 10 && written_context[1] == 50 && written_context[2] == 30);
    assert(written_target[0] == 10 && written_target[1] == 50 + 0x60 && written_target[2] == 30);
    assert(orient_left == &reference && orient_right == &context);
    assert(event_count == 1 && events[0].event_id == 0x25);

    reset();
    g_interaction_state.phase_05 = 2;
    g_interaction_active = 1;
    func_80039158(&actor, &context);
    assert(poll_count == 1 && event_count == 0);
    assert(written_target[0] == 40 && written_target[1] == 50 + 0x40 && written_target[2] == 60);
    assert(orient_left == &target_transform && orient_right == &reference);
}

static void test_confirmed_target_and_mutation(void) {
    reset();
    g_interaction_state.phase_05 = 2;
    g_interaction_active = 1;
    poll_results[0] = poll_results[1] = 1;
    change_heading_on_interaction = 1;
    change_heading_on_first_dispatch = 1;
    func_80039158(&actor, &context);
    assert(lookup_count == 2 && interaction_count == 1);
    assert(interaction_target == &target && interaction_mode == 1);
    assert(g_interaction_state.phase_05 == 3 && g_interaction_previous_phase == 2);
    assert(confirmed.used_2C == 1 && target.used_2C == 0);
    assert(event_count == 2 && events[0].event_id == 0x1B);
    assert(events[0].flags == 0x5678 && events[0].heading == 0x8765);
    assert(events[1].event_id == 0x25 && events[1].flags == 0xABCD && events[1].heading == 0xABCD);

    reset();
    g_interaction_state.phase_05 = 2;
    g_interaction_active = 1;
    poll_results[0] = poll_results[1] = 1;
    confirmed.used_2C = 1;
    func_80039158(&actor, &context);
    assert(interaction_count == 0 && event_count == 1 && events[0].event_id == 0x25);
}

static void test_later_phases(void) {
    reset();
    g_interaction_state.phase_05 = 2;
    g_interaction_active = 1;
    poll_results[0] = 1;
    change_phase_on_poll = 3;
    interaction_result = 1;
    func_80039158(&actor, &context);
    assert(interaction_count == 1 && interaction_mode == 0 && interaction_target == &target);
    assert(g_interaction_active == 0 && g_interaction_state.phase_05 == 0);
    assert(clear_count == 2 && update_count == 1 && event_count == 0);

    reset();
    g_interaction_state.phase_05 = 2;
    g_interaction_active = 1;
    poll_results[0] = 1;
    poll_results[1] = 0;
    change_phase_on_poll = 3;
    change_phase_poll_number = 2;
    interaction_result = 1;
    func_80039158(&actor, &context);
    assert(poll_count == 2 && interaction_count == 1 && interaction_mode == 0);
    assert(g_interaction_active == 0 && g_interaction_state.phase_05 == 0);
    assert(clear_count == 2 && update_count == 1 && event_count == 0);

    reset();
    g_interaction_state.phase_05 = 4;
    change_index_on_event = 1;
    func_80039158(&actor, &context);
    assert(g_interaction_state.phase_05 == 5 && g_interaction_marker == 0);
    assert(event_count == 2 && events[0].event_id == 0x2F && events[0].heading == 2);
    assert(events[1].event_id == 0x16);

    reset();
    g_interaction_state.phase_05 = 4;
    records[1].flag_20 = 1;
    func_80039158(&actor, &context);
    assert(g_interaction_state.phase_05 == 0 && clear_count == 2);
    assert(event_count == 1 && events[0].event_id == 0x16);

    reset();
    g_interaction_state.phase_05 = 5;
    func_80039158(&actor, &context);
    assert(g_interaction_state.phase_05 == 0 && g_interaction_marker == 0);
    assert(clear_count == 2 && reset_count == 1 && update_count == 1 && event_count == 0);
}

int main(void) {
    test_null_and_used_target();
    test_position_paths();
    test_confirmed_target_and_mutation();
    test_later_phases();
    puts("grounded interaction behavior tests passed");
    return 0;
}
