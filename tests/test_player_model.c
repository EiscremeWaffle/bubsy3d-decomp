#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "bubsy_actor_update.h"
#include "bubsy_actor_events.h"
#include "bubsy_death_state.h"
#include "move_requests.h"
#include "player_model.h"

static const char *loaded_path;
static void *loaded_asset;
static int32_t loaded_previous_size;
static int32_t loader_result;
static unsigned int assertion_count;
static int asset_marker_storage;
static void *asset_marker = &asset_marker_storage;
void *g_player_model_asset;
volatile uint8_t g_player_model_mode;
static int32_t vector_length_result;
static int32_t vector_length_input;
static unsigned int vector_length_call_count;
static unsigned int vector_square_call_count;

int32_t func_8001007C(int32_t left, int32_t right, int32_t *output) {
    *output = (int32_t)(((long long)left * right) >> 12);
    vector_square_call_count++;
    return 0;
}

int32_t func_8005359C(int32_t value, int32_t *output) {
    vector_length_input = value;
    vector_length_call_count++;
    *output = vector_length_result;
    return 0;
}

int32_t func_80053350(int32_t value, int32_t scale, int32_t *output) {
    *output = (int32_t)(((long long)value * scale) >> 12);
    return 0;
}

static int32_t fake_load(const char *path, void **asset_out, int32_t previous_size) {
    loaded_path = path;
    loaded_asset = asset_out;
    loaded_previous_size = previous_size;
    *asset_out = asset_marker;
    return loader_result;
}

static void fake_assert(int32_t failed, const char *condition, const char *source_path, uint32_t source_line) {
    assert(failed == 1);
    assert(strcmp(condition, "errorFlag == EGSBoolTrue") == 0);
    assert(strcmp(source_path, "../f/level.c") == 0);
    assert(source_line == 0x495);
    assertion_count++;
}

static void test_model_selection(int is_pliskin, int is_swimming, const char *expected_path) {
    LevelModelConfig config = {0};
    PlayerModelState state = {0};
    void *asset_out = NULL;

    config.flags_2A2A = is_pliskin ? 0x10 : 0;
    state.is_swimming = (uint8_t)is_swimming;
    state.config = &config;
    state.asset_size = 0x200;
    loader_result = 0x180;
    assertion_count = 0;

    assert(level_select_player_model_resource(&state, fake_load, fake_assert) == 0x180);
    assert(strcmp(loaded_path, expected_path) == 0);
    assert(loaded_asset == &state.asset);
    assert(loaded_previous_size == 0x200);
    assert(state.asset == asset_marker);
    assert(state.asset_size == 0x180);
    assert(assertion_count == 0);

    level_get_player_model(&state, &asset_out);
    assert(asset_out == asset_marker);
    g_player_model_asset = asset_marker;
    asset_out = NULL;
    assert(level_get_player_model_global(&asset_out) == asset_marker);
    assert(asset_out == asset_marker);
    g_player_model_mode = (uint8_t)is_swimming;
    assert(level_get_player_model_mode() == (uint8_t)is_swimming);
}

static void test_actor_flag_helpers(void) {
    PlayerActorFlagByte actor = {{0, 0, 0, 0}, 0xA1};
    assert(player_actor_set_flag_04(&actor) == 0);
    assert(actor.flags_04 == 0xA5);
    assert(player_actor_clear_flag_04(&actor) == 0);
    assert(actor.flags_04 == 0xA1);
}

static void test_actor_sequence_boundary(void) {
    const int32_t entries[] = {2500, -1, 3500, 50};
    PlayerActorSequenceData sequence = {.entries = entries};
    PlayerActorSequenceView actor = {0};
    uint8_t result = 0xFF;

    actor.cursor_08 = 250;
    actor.threshold_0E = -2;
    actor.sequence_18 = &sequence;
    assert(player_actor_sequence_boundary_reached(&actor, &result) == 0);
    assert(result == 1);

    actor.flags_04 = 0x04;
    actor.cursor_08 = 0;
    actor.advance_cursor_07 = 1;
    actor.state_05 = 0;
    actor.threshold_0E = 2;
    result = 0xFF;
    assert(player_actor_sequence_boundary_reached(&actor, &result) == 0);
    assert(result == 1);

    actor.cursor_08 = 2;
    actor.advance_cursor_07 = 0;
    actor.threshold_0E = 2;
    result = 0xFF;
    assert(player_actor_sequence_boundary_reached(&actor, &result) == 0);
    assert(result == 0);

    actor.cursor_08 = 2;
    actor.advance_cursor_07 = 1;
    actor.threshold_0E = 10;
    result = 0xFF;
    assert(player_actor_sequence_boundary_reached(&actor, &result) == 0);
    assert(result == 0);

    actor.cursor_08 = 0;
    actor.advance_cursor_07 = 1;
    actor.threshold_0E = 1;
    result = 0xFF;
    assert(player_actor_sequence_boundary_reached(&actor, &result) == 0);
    assert(result == 0);
}

static void test_actor_sequence_helpers(void) {
    const int32_t entries[] = {2500, -1001, 3500};
    PlayerActorSequenceData sequence = {.entries = entries};
    PlayerActorSequenceView actor = {0};
    int32_t result = 0;

    actor.sequence_18 = &sequence;
    actor.cursor_08 = 0;
    assert(player_actor_read_sequence_remainder(&actor, &result) == 0);
    assert(result == 500);
    actor.cursor_08 = 1;
    assert(player_actor_read_sequence_remainder(&actor, &result) == 0);
    assert(result == -1);

    sequence.flags_2C = 0x04;
    result = -1;
    assert(player_actor_read_sequence_remainder(&actor, &result) == 0);
    assert(result == 1);
    actor.cursor_08 = -5;
    assert(player_actor_read_sequence_remainder(&actor, &result) == 0);
    assert(result == -5);

    actor.cursor_08 = 12;
    actor.previous_cursor_0C = 9;
    assert(player_actor_read_cursor_delta(&actor, &result) == 0);
    assert(result == 3);

    actor.previous_cursor_0C = -3;
    assert(player_actor_write_previous_cursor_minus_two(&actor, &result) == 0);
    assert(result == -5);
    actor.previous_cursor_0C = INT16_MAX;
    assert(player_actor_write_previous_cursor_minus_two(&actor, &result) == 0);
    assert(result == INT16_MAX - 2);
}

static void test_actor_sequence_state_selection(void) {
    const int32_t entries[] = {-3, 0x12345, 0};
    PlayerActorSequenceData sequence = {
        .entries = entries,
        .entry_count = 2,
    };
    PlayerActorSequenceView actor = {
        .sequence_18 = &sequence,
        .state_05 = 9,
        .cursor_08 = 4,
        .unknown_0A = 6,
        .previous_cursor_0C = 5,
        .threshold_0E = 7,
    };

    assert(player_actor_select_sequence_state(&actor, 0) == PLAYER_SEQUENCE_STATE_SELECTED);
    assert(actor.state_05 == -3);
    assert(actor.cursor_08 == 2 && actor.previous_cursor_0C == 2);
    assert(actor.unknown_0A == 0x2345 && actor.threshold_0E == 1);

    actor.state_05 = 9;
    actor.cursor_08 = 4;
    actor.unknown_0A = 6;
    actor.previous_cursor_0C = 5;
    actor.threshold_0E = 7;
    assert(player_actor_select_sequence_state(&actor, 1) == PLAYER_SEQUENCE_ENTRY_NOT_NEGATIVE);
    assert(actor.state_05 == 9 && actor.cursor_08 == 4);
    assert(actor.unknown_0A == 6 && actor.previous_cursor_0C == 5);
    assert(actor.threshold_0E == 7);

    assert(player_actor_select_sequence_state(&actor, 2) == PLAYER_SEQUENCE_INDEX_PAST_TABLE);
    assert(actor.state_05 == 9 && actor.cursor_08 == 4);
}

static void test_actor_shared_state_progress_reset(void) {
    PlayerActorSharedStateView shared_state = {0};

    shared_state.progress_30 = 7;
    shared_state.progress_34 = 12;
    assert(player_actor_shared_state_clear_progress(0x1000, 0x2A6, &shared_state) == 1);
    assert(shared_state.progress_30 == 0 && shared_state.progress_34 == 0);

    shared_state.progress_30 = 3;
    shared_state.progress_34 = 9;
    assert(player_actor_shared_state_clear_progress(0, 0x2A7, &shared_state) == 1);
    assert(shared_state.progress_30 == 0 && shared_state.progress_34 == 0);

    shared_state.progress_30 = 3;
    shared_state.progress_34 = 9;
    assert(player_actor_shared_state_clear_progress(0, 0x2A6, &shared_state) == 0);
    assert(shared_state.progress_30 == 3 && shared_state.progress_34 == 9);
    assert(player_actor_shared_state_clear_progress(0x0008, 0x2A6, &shared_state) == 0);
    assert(shared_state.progress_30 == 3 && shared_state.progress_34 == 9);
}

static void test_actor_shared_state_mode1_numeric_update(void) {
    PlayerActorSharedStateView shared_state = {0};

    shared_state.progress_30 = 3;
    shared_state.progress_34 = 9;
    player_actor_shared_state_apply_mode1_numeric_update(0x8000, &shared_state);
    assert(shared_state.progress_30 == 3 && shared_state.progress_34 == 9);

    player_actor_shared_state_apply_mode1_numeric_update(0x10000, &shared_state);
    assert(shared_state.progress_30 == -0x34 && shared_state.progress_34 == 9);

    shared_state.progress_30 = 4;
    player_actor_shared_state_apply_mode1_numeric_update(0, &shared_state);
    assert(shared_state.progress_30 == 4 && shared_state.progress_34 == 9);
}

static void test_actor_shared_state_mode0_progress_clamp(void) {
    PlayerActorSharedStateView shared_state = {0};

    shared_state.progress_30 = 3;
    shared_state.progress_34 = 8;
    player_actor_shared_state_clamp_mode0_progress(0, &shared_state);
    assert(shared_state.progress_30 == 5 && shared_state.progress_34 == 8);

    shared_state.progress_30 = 10;
    player_actor_shared_state_clamp_mode0_progress(0, &shared_state);
    assert(shared_state.progress_30 == 10 && shared_state.progress_34 == 8);

    shared_state.progress_30 = 18;
    player_actor_shared_state_clamp_mode0_progress(0, &shared_state);
    assert(shared_state.progress_30 == 0x11 && shared_state.progress_34 == 8);

    shared_state.progress_30 = 2;
    shared_state.progress_34 = 8;
    player_actor_shared_state_clamp_mode0_progress(0x8000, &shared_state);
    assert(shared_state.progress_30 == 2 && shared_state.progress_34 == 8);
}

static void test_actor_shared_state_mode23_plan(void) {
    PlayerActorSharedStateMode23Plan plan = {0};

    assert(player_actor_shared_state_make_mode23_plan(1, 0, 0, &plan) == 0);
    assert(player_actor_shared_state_make_mode23_plan(4, 0, 0, &plan) == 0);
    assert(player_actor_shared_state_make_mode23_plan(2, 0x10000, 0, &plan) == 0);

    assert(player_actor_shared_state_make_mode23_plan(2, 0, 0, &plan) == 1);
    assert(plan.callback_flags == -0x400 && plan.temporary_progress == 0xE);
    assert(player_actor_shared_state_make_mode23_plan(3, 0, 0, &plan) == 1);
    assert(plan.callback_flags == 0x400 && plan.temporary_progress == 0xE);
    assert(player_actor_shared_state_make_mode23_plan(2, 0x8000, 0, &plan) == 1);
    assert(plan.temporary_progress == 8);
    assert(player_actor_shared_state_make_mode23_plan(3, 0x8000, 1, &plan) == 1);
    assert(plan.temporary_progress == 6);
}

static void test_actor_state23_event_17_gate(void) {
    assert(player_actor_state23_should_dispatch_event_17(0, 0, 0x29A) == 1);
    assert(player_actor_state23_should_dispatch_event_17(0, 3, 0x29A) == 1);
    assert(player_actor_state23_should_dispatch_event_17(0, 1, 0x29A) == 0);
    assert(player_actor_state23_should_dispatch_event_17(0, 2, 0x29A) == 0);
    assert(player_actor_state23_should_dispatch_event_17(0, 4, 0x29A) == 0);
    assert(player_actor_state23_should_dispatch_event_17(0x2000, 0, 0x29A) == 0);
    assert(player_actor_state23_should_dispatch_event_17(0, 0, 0x298) == 0);
}

static void test_actor_state_target_data(void) {
    assert(sizeof(player_actor_state_targets) == 49 * sizeof(uint32_t));
    assert(player_actor_state_targets[0] == 0x8003653C);
    assert(player_actor_state_targets[1] == 0x80036874);
    assert(player_actor_state_targets[20] == 0x8003677C);
    assert(player_actor_state_targets[21] == 0x800367E0);
    assert(player_actor_state_targets[25] == 0x80036768);
    assert(player_actor_state_targets[48] == 0x800364A0);
}

static unsigned int move_assertion_count;

static void record_move_limit_assertion(int32_t failed, const char *condition, const char *source_path, uint32_t source_line) {
    assert(failed == 1);
    assert(strcmp(condition, "gMoveRequestCount < MAX_MOVE_REQUEST_COUNT") == 0);
    assert(strcmp(source_path, "../f/game.c") == 0);
    assert(source_line == 0xB01);
    move_assertion_count++;
}

static void test_move_request_queue(void) {
    int requests[31] = {0};
    void *entries[31] = {0};
    GameMoveRequestQueue queue = {29, entries};
    move_assertion_count = 0;

    assert(game_enqueue_move_request(&queue, &requests[0], record_move_limit_assertion) == 29 * 4);
    assert(queue.count == 30 && entries[29] == &requests[0]);
    assert(move_assertion_count == 0);

    assert(game_enqueue_move_request(&queue, &requests[1], record_move_limit_assertion) == 30 * 4);
    assert(queue.count == 31 && entries[30] == &requests[1]);
    assert(move_assertion_count == 1);
}

typedef struct EventCallLog {
    unsigned int selected_count;
    unsigned int flag_count;
    unsigned int dispatch_count;
    uint16_t property_id;
    uint32_t dispatch_flags;
} EventCallLog;

static void record_selected_property(void *context, void *actor_component, uint16_t property_id) {
    EventCallLog *log = context;
    assert(actor_component != NULL);
    log->selected_count++;
    log->property_id = property_id;
}

static void record_active_flag(void *context, void *actor_component) {
    EventCallLog *log = context;
    assert(actor_component != NULL);
    log->flag_count++;
}

static int32_t record_actor_dispatch(void *context, void *actor_component, uint32_t flags) {
    EventCallLog *log = context;
    assert(actor_component != NULL);
    log->dispatch_count++;
    log->dispatch_flags = flags;
    return 0x42;
}

static EventCallLog run_actor_event(uint8_t update_mode, uint8_t runtime_mode, uint16_t event_id, uint8_t state_6463, int32_t expected_return) {
    EventCallLog log = {0};
    int actor_component;
    BubsyActorEventOps ops = {
        &log,
        &actor_component,
        runtime_mode,
        state_6463,
        event_id,
        record_selected_property,
        record_active_flag,
        record_actor_dispatch,
    };
    assert(bubsy_process_actor_event(update_mode, &ops) == expected_return);
    return log;
}

static void test_actor_event_dispatch(void) {
    EventCallLog log = run_actor_event(0, 0, BUBSY_EVENT_PROPERTY_KIND_3E0, 0, 0x42);
    assert(log.selected_count == 1 && log.property_id == BUBSY_EVENT_PROPERTY_NORMAL);
    assert(log.flag_count == 1 && log.dispatch_count == 1 && log.dispatch_flags == 0);

    log = run_actor_event(0, 1, BUBSY_EVENT_PROPERTY_KIND_157, 0, 0x42);
    assert(log.selected_count == 1 && log.property_id == BUBSY_EVENT_PROPERTY_SWIM_MODE);
    assert(log.flag_count == 1 && log.dispatch_count == 1 && log.dispatch_flags == 0);

    log = run_actor_event(0, 0, BUBSY_EVENT_PROPERTY_KIND_3E0, 3, BUBSY_EVENT_PROPERTY_KIND_157);
    assert(log.selected_count == 0 && log.flag_count == 0 && log.dispatch_count == 0);

    log = run_actor_event(1, 1, BUBSY_EVENT_PROPERTY_KIND_157, 0, 0x42);
    assert(log.selected_count == 1 && log.property_id == BUBSY_EVENT_PROPERTY_KIND_157);
    assert(log.dispatch_flags == BUBSY_EVENT_SPECIAL_FLAG);

    log = run_actor_event(2, 0, BUBSY_EVENT_PROPERTY_KIND_3E0, 0, 1);
    assert(log.selected_count == 0 && log.flag_count == 0 && log.dispatch_count == 0);

    log = run_actor_event(0, 0, 0x999, 0, BUBSY_EVENT_PROPERTY_KIND_157);
    assert(log.selected_count == 0 && log.flag_count == 0 && log.dispatch_count == 0);
    log = run_actor_event(0, 2, BUBSY_EVENT_PROPERTY_KIND_157, 0, BUBSY_EVENT_PROPERTY_KIND_157);
    assert(log.selected_count == 0 && log.flag_count == 0 && log.dispatch_count == 0);
}

static void test_bubsy_actor_update_entry_gate(void) {
    BubsyActorUpdateActorView actor = {0};
    BubsyActorUpdateEnvironment environment = {0, 7, 9, 12};
    uint8_t local_state = 0xFF;

    assert(bubsy_actor_update_entry_gate(&actor, 0, &environment, &local_state) == BUBSY_ACTOR_UPDATE_CONTINUE);
    assert(local_state == 0 && environment.actor_event_guard_6460 == 0);
    assert(environment.update_mode_647A == 7 && environment.sequence_counter_38C == 12);

    actor.update_state_10 = 0x120;
    environment.actor_event_guard_6460 = 4;
    assert(bubsy_actor_update_entry_gate(&actor, 1, &environment, &local_state) == BUBSY_ACTOR_UPDATE_SKIP);
    assert(local_state == 1 && environment.update_mode_647A == 0);
    assert(environment.sequence_counter_38C == 0 && environment.actor_event_guard_6460 == 4);

    actor.update_state_10 = 0x200;
    environment.update_gate_6454 = 1;
    environment.update_mode_647A = 7;
    environment.sequence_counter_38C = 12;
    assert(bubsy_actor_update_entry_gate(&actor, 1, &environment, &local_state) == BUBSY_ACTOR_UPDATE_CONTINUE);
    assert(local_state == 0 && actor.update_state_10 == 0);
    assert(environment.update_mode_647A == 7 && environment.sequence_counter_38C == 12);
    assert(environment.actor_event_guard_6460 == 0);
}

static void test_bubsy_actor_default_state_route(void) {
    BubsyActorUpdateActorView actor = {0};

    assert(bubsy_actor_state_route_default(&actor, 0x25, 0, 5, 0, 0, 0) == BUBSY_ACTOR_DEFAULT_ROUTE_GROUNDED);
    assert(bubsy_actor_state_route_default(&actor, 0x25, 0, 5, 0, 0, 1) == BUBSY_ACTOR_DEFAULT_ROUTE_SWIM);
    assert(bubsy_actor_state_route_default(&actor, 0x26, -1, 5, 0, 0, 0) == BUBSY_ACTOR_DEFAULT_ROUTE_GROUNDED);
    assert(bubsy_actor_state_route_default(&actor, 0x26, -2, 5, 0, 0, 0) == BUBSY_ACTOR_DEFAULT_ROUTE_SKIP);
    assert(bubsy_actor_state_route_default(&actor, 0x24, 0, 5, 0, 0, 0) == BUBSY_ACTOR_DEFAULT_ROUTE_SKIP);
    assert(bubsy_actor_state_route_default(&actor, 0x25, 0, 4, 0, 0, 0) == BUBSY_ACTOR_DEFAULT_ROUTE_SKIP);
    assert(bubsy_actor_state_route_default(&actor, 0x25, 0, 5, 1, 0, 0) == BUBSY_ACTOR_DEFAULT_ROUTE_SKIP);
    assert(bubsy_actor_state_route_default(&actor, 0x25, 0, 5, 0, 1, 0) == BUBSY_ACTOR_DEFAULT_ROUTE_SKIP);

    actor.flags_04 = 0x100;
    assert(bubsy_actor_state_route_default(&actor, 0x25, 0, 5, 0, 0, 0) == BUBSY_ACTOR_DEFAULT_ROUTE_SKIP);
    actor.flags_04 = 0;
    actor.update_state_10 = 0x10000;
    assert(bubsy_actor_state_route_default(&actor, 0x25, 0, 5, 0, 0, 0) == BUBSY_ACTOR_DEFAULT_ROUTE_SKIP);
}

static void test_bubsy_grounded_case38_descriptor(void) {
    uint8_t descriptor_state = 2;

    assert(bubsy_grounded_case38_update_descriptor(&descriptor_state) ==
        BUBSY_GROUNDED_CASE38_CALL_39158);
    assert(descriptor_state == 4);

    descriptor_state = 1;
    assert(bubsy_grounded_case38_update_descriptor(&descriptor_state) ==
        (BUBSY_GROUNDED_CASE38_CALL_39158 | BUBSY_GROUNDED_CASE38_CALL_2262C_11));
    assert(descriptor_state == 0);

    descriptor_state = 5;
    assert(bubsy_grounded_case38_update_descriptor(&descriptor_state) ==
        BUBSY_GROUNDED_CASE38_CALL_39158);
    assert(descriptor_state == 5);
}

static void test_bubsy_grounded_dispatch_table(void) {
    static const uint32_t expected_targets[33] = {
        0x8004B990, 0x8004BE60, 0x8004BC48, 0x8004BE60, 0x8004BC48,
        0x8004BE60, 0x8004B70C, 0x8004B770, 0x8004B838, 0x8004B8D8,
        0x8004B77C, 0x8004B7F8, 0x8004BFD0, 0x8004BFD0, 0x8004B4D8,
        0x8004B578, 0x8004BFD0, 0x8004BFD0, 0x8004BFD0, 0x8004BFD0,
        0x8004BFD0, 0x8004BFD0, 0x8004BFD0, 0x8004BA18, 0x8004BA28,
        0x8004BA34, 0x8004BB68, 0x8004BDA4, 0x8004BE4C, 0x8004BDA4,
        0x8004BE4C, 0x8004B66C, 0x8004B6C0,
    };
    unsigned int index;

    for (index = 0; index < 33; index++) {
        assert(bubsy_grounded_handler_case_target((uint16_t)(index + 6)) == expected_targets[index]);
    }
    assert(bubsy_grounded_handler_case_target(5) == 0x8004BFD0);
    assert(bubsy_grounded_handler_case_target(39) == 0x8004BFD0);
}

static void test_bubsy_grounded_handler_prelude(void) {
    int16_t descriptor_state = 0;

    assert(bubsy_grounded_handler_should_dispatch_switch(14, 2, 0x1100, 0x1000,
        &descriptor_state) == 1);
    assert(bubsy_grounded_handler_should_dispatch_switch(14, 3, 0x1100, 0,
        &descriptor_state) == 0);
    assert(bubsy_grounded_handler_should_dispatch_switch(14, 3, 0, 0x1000,
        &descriptor_state) == 0);
    assert(descriptor_state == 1);
    descriptor_state = 5;
    assert(bubsy_grounded_handler_should_dispatch_switch(14, 3, 0, 0x1000,
        &descriptor_state) == 0);
    assert(descriptor_state == 5);
    assert(bubsy_grounded_handler_should_dispatch_switch(14, 3, 0, 0,
        &descriptor_state) == 1);
    descriptor_state = 5;
    assert(bubsy_grounded_handler_should_dispatch_switch(15, 3, 0, 0x1000,
        &descriptor_state) == 0);
    assert(descriptor_state == 0);
    assert(bubsy_grounded_handler_should_dispatch_switch(15, 3, 0, 0,
        &descriptor_state) == 1);
    assert(bubsy_grounded_handler_should_dispatch_switch(20, 3, 0x1100, 0x1000,
        &descriptor_state) == 1);
}

static void test_bubsy_grounded_case37(void) {
    uint8_t descriptor_state = 0;

    assert(bubsy_grounded_case37_select_action(0, &descriptor_state) == BUBSY_GROUNDED_CASE37_INITIALIZE);
    assert(descriptor_state == 1);

    descriptor_state = 5;
    assert(bubsy_grounded_case37_select_action(0, &descriptor_state) == BUBSY_GROUNDED_CASE37_NO_ACTION);
    assert(descriptor_state == 5);

    descriptor_state = 3;
    assert(bubsy_grounded_case37_select_action(0, &descriptor_state) == BUBSY_GROUNDED_CASE37_PROCESS_ACTOR);
    assert(descriptor_state == 3);
    assert(bubsy_grounded_case37_select_action(0x100, &descriptor_state) == BUBSY_GROUNDED_CASE37_NO_ACTION);
    assert(bubsy_grounded_case37_select_action(0x1000, &descriptor_state) == BUBSY_GROUNDED_CASE37_NO_ACTION);
    assert(descriptor_state == 3);
}

static void test_bubsy_grounded_cases29_30(void) {
    uint8_t descriptor_state = 0xA5;

    assert(bubsy_grounded_cases29_30_update_descriptor(29, &descriptor_state) == 1);
    assert(descriptor_state == 1);
    assert(bubsy_grounded_cases29_30_update_descriptor(30, &descriptor_state) == 1);
    assert(descriptor_state == 0);
    descriptor_state = 0xA5;
    assert(bubsy_grounded_cases29_30_update_descriptor(31, &descriptor_state) == 0);
    assert(descriptor_state == 0xA5);
}

static void test_bubsy_grounded_cases13_34_36(void) {
    uint8_t actor_state = 0xA5;
    uint16_t descriptor_state = 0xBEEF;

    assert(bubsy_grounded_cases13_34_36_update_state(13, &actor_state, &descriptor_state) == 1);
    assert(actor_state == 2 && descriptor_state == 0xBEEF);
    actor_state = 0xA5;
    assert(bubsy_grounded_cases13_34_36_update_state(34, &actor_state, &descriptor_state) == 1);
    assert(actor_state == 2 && descriptor_state == 0);
    actor_state = 0xA5;
    descriptor_state = 0xBEEF;
    assert(bubsy_grounded_cases13_34_36_update_state(36, &actor_state, &descriptor_state) == 1);
    assert(actor_state == 2 && descriptor_state == 0);
    actor_state = 0xA5;
    descriptor_state = 0xBEEF;
    assert(bubsy_grounded_cases13_34_36_update_state(35, &actor_state, &descriptor_state) == 0);
    assert(actor_state == 0xA5 && descriptor_state == 0xBEEF);
}

static void test_bubsy_grounded_case21(void) {
    int16_t descriptor_state = 4;

    assert(bubsy_grounded_case21_select_action(0, 0x1000, 0, 0xCCB, &descriptor_state) ==
        (BUBSY_GROUNDED_CASE21_CALL_2E7D8 | BUBSY_GROUNDED_CASE21_CALL_22650_08 |
            BUBSY_GROUNDED_CASE21_CALL_347EC_1));
    assert(descriptor_state == 0);

    descriptor_state = 4;
    assert(bubsy_grounded_case21_select_action(0, 0x1000, 0, 0xCCC, &descriptor_state) ==
        BUBSY_GROUNDED_CASE21_CALL_2262C_0C);
    assert(descriptor_state == 0);

    descriptor_state = 4;
    assert(bubsy_grounded_case21_select_action(0x40, 0x1000, 0, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE21_CALL_2262C_0C);
    descriptor_state = 4;
    assert(bubsy_grounded_case21_select_action(0, 0, 0, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE21_CALL_2262C_0C);
    descriptor_state = 4;
    assert(bubsy_grounded_case21_select_action(0, 0x1000, 1, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE21_CALL_2262C_0C);
    descriptor_state = 5;
    assert(bubsy_grounded_case21_select_action(0, 0x1000, 0, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE21_CALL_2262C_0C);
    assert(descriptor_state == 0);
}

static void test_bubsy_grounded_case12(void) {
    uint8_t actor_state = 0;
    int16_t descriptor_state = 2;

    assert(bubsy_grounded_case12_update_state(0x800, 0x1000, &actor_state, &descriptor_state) ==
        (BUBSY_GROUNDED_CASE12_CALL_2262C_0C | BUBSY_GROUNDED_CASE12_CALL_347EC_1));
    assert(actor_state == 1 && descriptor_state == 5);

    actor_state = 1;
    descriptor_state = 2;
    assert(bubsy_grounded_case12_update_state(0x800, 0, &actor_state, &descriptor_state) ==
        BUBSY_GROUNDED_CASE12_NO_ACTION);
    assert(actor_state == 1 && descriptor_state == 5);

    descriptor_state = 2;
    assert(bubsy_grounded_case12_update_state(0x100, 0x1000, &actor_state, &descriptor_state) ==
        BUBSY_GROUNDED_CASE12_NO_ACTION);
    assert(descriptor_state == 2);
    assert(bubsy_grounded_case12_update_state(0x800, 0xA0084, &actor_state, &descriptor_state) ==
        BUBSY_GROUNDED_CASE12_NO_ACTION);
    assert(descriptor_state == 2);
}

static void test_bubsy_grounded_case17(void) {
    uint8_t actor_state_00 = 0xA5;
    uint8_t actor_state_01 = 0xA5;
    uint8_t descriptor_state_14 = 0xA5;

    assert(bubsy_grounded_case17_update_state(0, &actor_state_00, &actor_state_01, &descriptor_state_14) ==
        BUBSY_GROUNDED_CASE17_CALL_1C700);
    assert(actor_state_00 == 0xA5 && actor_state_01 == 2 && descriptor_state_14 == 0);

    actor_state_00 = 0xA5;
    descriptor_state_14 = 0xA5;
    assert(bubsy_grounded_case17_update_state(0x40, &actor_state_00, &actor_state_01, &descriptor_state_14) ==
        (BUBSY_GROUNDED_CASE17_CALL_1C700 | BUBSY_GROUNDED_CASE17_CALL_22650_02 |
            BUBSY_GROUNDED_CASE17_CALL_347EC_1));
    assert(actor_state_00 == 0 && actor_state_01 == 2 && descriptor_state_14 == 0);
}

static void test_bubsy_grounded_case16(void) {
    uint8_t descriptor_state = 0;

    assert(bubsy_grounded_case16_should_dispatch(0, 0, 0, &descriptor_state) == 1);
    assert(descriptor_state == 1);

    descriptor_state = 0;
    assert(bubsy_grounded_case16_should_dispatch(0x2, 0, 0, &descriptor_state) == 0);
    assert(descriptor_state == 0);

    descriptor_state = 1;
    assert(bubsy_grounded_case16_should_dispatch(0, 0, 1, &descriptor_state) == 0);
    assert(descriptor_state == 1);
    assert(bubsy_grounded_case16_should_dispatch(0x100, 0, 0, &descriptor_state) == 0);
    assert(bubsy_grounded_case16_should_dispatch(0, 0x80000, 0, &descriptor_state) == 0);
}

static void test_bubsy_grounded_case14(void) {
    uint32_t actor_state = 0x100;
    uint32_t linked_state = 0x200;

    assert(bubsy_grounded_case14_prepare(0x800, &actor_state, 0, &linked_state) ==
        (BUBSY_GROUNDED_CASE14_CALL_22650_07 | BUBSY_GROUNDED_CASE14_SELECT_2BE |
            BUBSY_GROUNDED_CASE14_ASSERT_ON_SELECTION_FAILURE | BUBSY_GROUNDED_CASE14_CALL_52E18));
    assert(actor_state == 0 && linked_state == 0);

    actor_state = 0x100;
    linked_state = 0x200;
    assert(bubsy_grounded_case14_prepare(0x800, &actor_state, 2, &linked_state) == BUBSY_GROUNDED_CASE14_NO_ACTION);
    assert(actor_state == 0x100 && linked_state == 0x200);
    assert(bubsy_grounded_case14_prepare(0x1104, &actor_state, 0, &linked_state) == BUBSY_GROUNDED_CASE14_NO_ACTION);
    assert(bubsy_grounded_case14_prepare(0, &actor_state, 0, &linked_state) == BUBSY_GROUNDED_CASE14_NO_ACTION);
    actor_state = 0x01000000;
    assert(bubsy_grounded_case14_prepare(0x800, &actor_state, 0, &linked_state) == BUBSY_GROUNDED_CASE14_NO_ACTION);
    assert(actor_state == 0x01000000 && linked_state == 0x200);
}

static void test_bubsy_grounded_case15(void) {
    assert(bubsy_grounded_case15_select_actions(0x2BE, 0) ==
        (BUBSY_GROUNDED_CASE15_SELECT_2C3 | BUBSY_GROUNDED_CASE15_CALL_52E18 |
            BUBSY_GROUNDED_CASE15_REQUEST_MOVE));
    assert(bubsy_grounded_case15_select_actions(0x2BD, 1) ==
        (BUBSY_GROUNDED_CASE15_CALL_2262C_07 | BUBSY_GROUNDED_CASE15_CALL_3539C_1));
    assert(bubsy_grounded_case15_select_actions(0x2BD, 0) ==
        BUBSY_GROUNDED_CASE15_REQUEST_MOVE);
    assert(bubsy_grounded_case15_select_actions(0x2BF, 2) ==
        BUBSY_GROUNDED_CASE15_REQUEST_MOVE);
}

static void test_bubsy_grounded_case6(void) {
    uint8_t descriptor_state = 0;

    assert(bubsy_grounded_case6_select_action(0, 0x44, &descriptor_state) ==
        BUBSY_GROUNDED_CASE6_CALL_358C8_MODE_0);
    assert(descriptor_state == 1);
    descriptor_state = 0;
    assert(bubsy_grounded_case6_select_action(0, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE6_CALL_35ACC);
    assert(descriptor_state == 1);
    descriptor_state = 0;
    assert(bubsy_grounded_case6_select_action(0x4, 0x44, &descriptor_state) ==
        BUBSY_GROUNDED_CASE6_NO_ACTION);
    assert(descriptor_state == 0);
    assert(bubsy_grounded_case6_select_action(0, 0x82000, &descriptor_state) ==
        BUBSY_GROUNDED_CASE6_NO_ACTION);
    assert(bubsy_grounded_case6_select_action(0x40, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE6_NO_ACTION);
    assert(bubsy_grounded_case6_select_action(0, 0x01020080, &descriptor_state) ==
        BUBSY_GROUNDED_CASE6_NO_ACTION);
    assert(descriptor_state == 1);
}

static void test_bubsy_grounded_cases33_35(void) {
    uint16_t descriptor_state = 0xBEEF;
    int32_t linked_counter = 0x1234;
    const BubsyGroundedCases33_35Actions actions = BUBSY_GROUNDED_CASE33_35_CALL_22650_0F |
        BUBSY_GROUNDED_CASE33_35_CALL_22650_08 | BUBSY_GROUNDED_CASE33_35_CALL_347EC_1;

    assert(bubsy_grounded_cases33_35_prepare(33, 0, 0, 0, &descriptor_state, &linked_counter) == actions);
    assert(descriptor_state == 3 && linked_counter == 0);
    descriptor_state = 0xBEEF;
    linked_counter = 0x1234;
    assert(bubsy_grounded_cases33_35_prepare(35, 0, 0, 0, &descriptor_state, &linked_counter) == actions);
    assert(descriptor_state == 2 && linked_counter == 0);

    descriptor_state = 0xBEEF;
    linked_counter = 0x1234;
    assert(bubsy_grounded_cases33_35_prepare(34, 0, 0, 0, &descriptor_state, &linked_counter) == BUBSY_GROUNDED_CASE33_35_NO_ACTION);
    assert(bubsy_grounded_cases33_35_prepare(33, 0x4, 0, 0, &descriptor_state, &linked_counter) == BUBSY_GROUNDED_CASE33_35_NO_ACTION);
    assert(bubsy_grounded_cases33_35_prepare(33, 0x1140, 0, 0, &descriptor_state, &linked_counter) == BUBSY_GROUNDED_CASE33_35_NO_ACTION);
    assert(bubsy_grounded_cases33_35_prepare(33, 0, 0x100000, 0, &descriptor_state, &linked_counter) == BUBSY_GROUNDED_CASE33_35_NO_ACTION);
    assert(bubsy_grounded_cases33_35_prepare(33, 0, 1, 0, &descriptor_state, &linked_counter) == BUBSY_GROUNDED_CASE33_35_NO_ACTION);
    assert(bubsy_grounded_cases33_35_prepare(33, 0, 0, 1, &descriptor_state, &linked_counter) == BUBSY_GROUNDED_CASE33_35_NO_ACTION);
    assert(descriptor_state == 0xBEEF && linked_counter == 0x1234);
}

static void test_bubsy_grounded_case20(void) {
    const uint8_t base_actions = BUBSY_GROUNDED_CASE20_CALL_516F8_54AF0 |
        BUBSY_GROUNDED_CASE20_CALL_22650_0C;

    assert(bubsy_grounded_case20_select_actions(0, 0, 0, 0x348) ==
        (base_actions | BUBSY_GROUNDED_CASE20_SELECT_349));
    assert(bubsy_grounded_case20_select_actions(0, 0, 0, 0x349) == base_actions);
    assert(bubsy_grounded_case20_select_actions(0, 0x80, 0, 0) == base_actions);
    assert(bubsy_grounded_case20_select_actions(1, 0, 0, 0) == BUBSY_GROUNDED_CASE20_NO_ACTION);
    assert(bubsy_grounded_case20_select_actions(0, 0, 1, 0) == BUBSY_GROUNDED_CASE20_NO_ACTION);
    assert(bubsy_grounded_case20_select_actions(0, 0x100, 0, 0) == BUBSY_GROUNDED_CASE20_NO_ACTION);
}

static void test_bubsy_grounded_case31_counter(void) {
    uint8_t counter = 10;

    assert(bubsy_grounded_case31_advance_counter(0, 0, 0, &counter) ==
        BUBSY_GROUNDED_CASE31_COUNTER_BELOW_11);
    assert(counter == 11);
    assert(bubsy_grounded_case31_advance_counter(0, 0, 0, &counter) ==
        BUBSY_GROUNDED_CASE31_COUNTER_SEQUENCE_PATH);
    assert(counter == 12);
    counter = 0xFF;
    assert(bubsy_grounded_case31_advance_counter(0, 0, 0, &counter) ==
        BUBSY_GROUNDED_CASE31_COUNTER_SEQUENCE_PATH);
    assert(counter == 0);

    counter = 4;
    assert(bubsy_grounded_case31_advance_counter(0x4, 0, 0, &counter) == BUBSY_GROUNDED_CASE31_SKIP);
    assert(bubsy_grounded_case31_advance_counter(0, 0x80, 0, &counter) == BUBSY_GROUNDED_CASE31_SKIP);
    assert(bubsy_grounded_case31_advance_counter(0, 0, 1, &counter) == BUBSY_GROUNDED_CASE31_SKIP);
    assert(bubsy_grounded_case31_advance_counter(0x40, 0, 0, &counter) == BUBSY_GROUNDED_CASE31_ALTERNATE);
    assert(bubsy_grounded_case31_advance_counter(0, 0x40, 0, &counter) == BUBSY_GROUNDED_CASE31_ALTERNATE);
    assert(counter == 4);
}

static void test_bubsy_grounded_case31_followups(void) {
    int16_t descriptor_state = 0;
    int32_t linked_counter = 0;
    const uint8_t common_sequence_actions = BUBSY_GROUNDED_CASE31_SET_LINKED_30_TO_MINUS_11 |
        BUBSY_GROUNDED_CASE31_CALL_22944;

    assert(bubsy_grounded_case31_select_alternate_action(0, 0x1000, &descriptor_state) ==
        BUBSY_GROUNDED_CASE31_SET_DESCRIPTOR_02_TO_2);
    assert(descriptor_state == 2);
    descriptor_state = 0;
    assert(bubsy_grounded_case31_select_alternate_action(0, 0x44, &descriptor_state) ==
        BUBSY_GROUNDED_CASE31_CALL_358C8_MODE_1);
    assert(bubsy_grounded_case31_select_alternate_action(0, 0x10044, &descriptor_state) ==
        BUBSY_GROUNDED_CASE31_NO_ACTION);
    assert(bubsy_grounded_case31_select_alternate_action(0x40, 0x44, &descriptor_state) ==
        BUBSY_GROUNDED_CASE31_NO_ACTION);
    descriptor_state = 2;
    assert(bubsy_grounded_case31_select_alternate_action(0, 0x1000, &descriptor_state) ==
        BUBSY_GROUNDED_CASE31_NO_ACTION);
    assert(bubsy_grounded_case31_select_alternate_action(0, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE31_NO_ACTION);

    assert(bubsy_grounded_case31_select_sequence_actions(0, &linked_counter) == common_sequence_actions);
    assert(linked_counter == -0x11);
    linked_counter = 4;
    assert(bubsy_grounded_case31_select_sequence_actions(1, &linked_counter) ==
        (common_sequence_actions | BUBSY_GROUNDED_CASE31_CALL_52AEC_0 |
            BUBSY_GROUNDED_CASE31_CALL_52E18 | BUBSY_GROUNDED_CASE31_CALL_22650_17));
    assert(linked_counter == -0x11);
}

static void test_bubsy_grounded_cases8_10(void) {
    int16_t descriptor_state = 0;

    assert(bubsy_grounded_cases8_10_select_action(8, 0, 0x1000, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE8_10_SET_DESCRIPTOR_STATE);
    assert(descriptor_state == 3);
    descriptor_state = 0;
    assert(bubsy_grounded_cases8_10_select_action(10, 0, 0x1000, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE8_10_SET_DESCRIPTOR_STATE);
    assert(descriptor_state == 4);
    assert(bubsy_grounded_cases8_10_select_action(8, 0, 4, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE8_CALL_358C8_MODE_3);
    assert(bubsy_grounded_cases8_10_select_action(10, 0, 4, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE10_CALL_358C8_MODE_2);
    assert(bubsy_grounded_cases8_10_select_action(8, 0, 0, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE8_10_CONTINUE_COUNTER_PATH);
    assert(bubsy_grounded_cases8_10_select_action(8, 0, 0x80, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE8_10_EPILOGUE);
    assert(bubsy_grounded_cases8_10_select_action(8, 0x4, 0, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE8_10_EPILOGUE);
    assert(bubsy_grounded_cases8_10_select_action(8, 0, 0x82000, 0, &descriptor_state) ==
        BUBSY_GROUNDED_CASE8_10_EPILOGUE);
    assert(bubsy_grounded_cases8_10_select_action(8, 0, 0x1000, 1, &descriptor_state) ==
        BUBSY_GROUNDED_CASE8_10_EPILOGUE);
    assert(descriptor_state == 4);
}

static void test_bubsy_grounded_cases8_10_counter_path(void) {
    int32_t global_gate = 1;
    uint32_t global_state = 8;
    int16_t global_counter = 39;
    uint8_t actions = (uint8_t)bubsy_grounded_cases8_10_update_counter_path(
        8, 0, 0, &global_gate, &global_state, &global_counter);

    assert(actions == (BUBSY_GROUNDED_CASE8_10_CALL_4FFEC |
        BUBSY_GROUNDED_CASE8_10_CLEAR_GP_540 |
        BUBSY_GROUNDED_CASE8_10_INCREMENT_GP_COUNTER |
        BUBSY_GROUNDED_CASE8_10_CALL_34EF8));
    assert(global_gate == 0 && global_state == 8 && global_counter == 40);

    global_gate = 0;
    global_state = 10;
    global_counter = 12;
    actions = (uint8_t)bubsy_grounded_cases8_10_update_counter_path(
        8, 0, 0, &global_gate, &global_state, &global_counter);
    assert(actions == (BUBSY_GROUNDED_CASE8_10_CLEAR_GP_540 |
        BUBSY_GROUNDED_CASE8_10_RESET_GP_COUNTER |
        BUBSY_GROUNDED_CASE8_10_INCREMENT_GP_COUNTER |
        BUBSY_GROUNDED_CASE8_10_CALL_34EF8));
    assert(global_state == 8 && global_counter == 1);

    global_state = 8;
    global_counter = 40;
    actions = (uint8_t)bubsy_grounded_cases8_10_update_counter_path(
        8, 0, 0, &global_gate, &global_state, &global_counter);
    assert((actions & BUBSY_GROUNDED_CASE8_10_CLAMP_GP_COUNTER) != 0);
    assert(global_counter == 40);

    global_gate = 4;
    global_state = 10;
    global_counter = 7;
    assert(bubsy_grounded_cases8_10_update_counter_path(
        8, 0, 5, &global_gate, &global_state, &global_counter) ==
        BUBSY_GROUNDED_CASE8_10_COUNTER_NO_ACTION);
    assert(global_gate == 4 && global_state == 10 && global_counter == 7);
    assert(bubsy_grounded_cases8_10_update_counter_path(
        8, 0x80, 0, &global_gate, &global_state, &global_counter) ==
        BUBSY_GROUNDED_CASE8_10_COUNTER_NO_ACTION);
}

static void test_bubsy_grounded_case32(void) {
    uint8_t actor_state_01 = 0;
    uint8_t descriptor_counter_4D = 3;
    uint8_t descriptor_state_4C = 4;
    uint8_t descriptor_state_68 = 5;
    int16_t descriptor_state_02 = 6;
    int32_t linked_counter_30 = 7;

    assert(bubsy_grounded_case32_update_state(0, 0x1000, &actor_state_01,
        &descriptor_counter_4D, &descriptor_state_4C, &descriptor_state_68,
        &descriptor_state_02, &linked_counter_30) == BUBSY_GROUNDED_CASE32_CLEAR_DESCRIPTOR_02);
    assert(descriptor_state_02 == 0 && descriptor_counter_4D == 3);

    descriptor_state_02 = 6;
    assert(bubsy_grounded_case32_update_state(0, 0x44, &actor_state_01,
        &descriptor_counter_4D, &descriptor_state_4C, &descriptor_state_68,
        &descriptor_state_02, &linked_counter_30) == BUBSY_GROUNDED_CASE32_CALL_2262C_17);
    assert(descriptor_state_4C == 0 && descriptor_state_68 == 1 && descriptor_state_02 == 6);

    descriptor_state_4C = 4;
    descriptor_state_68 = 5;
    assert(bubsy_grounded_case32_update_state(0, 0, &actor_state_01,
        &descriptor_counter_4D, &descriptor_state_4C, &descriptor_state_68,
        &descriptor_state_02, &linked_counter_30) ==
        (BUBSY_GROUNDED_CASE32_CALL_22650_10 | BUBSY_GROUNDED_CASE32_CALL_347EC_1));
    assert(actor_state_01 == 2 && descriptor_counter_4D == 0 && linked_counter_30 == 0);

    actor_state_01 = 0;
    descriptor_counter_4D = 11;
    linked_counter_30 = 7;
    assert(bubsy_grounded_case32_update_state(0, 0, &actor_state_01,
        &descriptor_counter_4D, &descriptor_state_4C, &descriptor_state_68,
        &descriptor_state_02, &linked_counter_30) ==
        (BUBSY_GROUNDED_CASE32_CALL_2262C_17 | BUBSY_GROUNDED_CASE32_CALL_3539C_1));
    assert(descriptor_counter_4D == 0 && linked_counter_30 == 7 && actor_state_01 == 0);

    descriptor_counter_4D = 3;
    assert(bubsy_grounded_case32_update_state(0, 0x01002000, &actor_state_01,
        &descriptor_counter_4D, &descriptor_state_4C, &descriptor_state_68,
        &descriptor_state_02, &linked_counter_30) ==
        (BUBSY_GROUNDED_CASE32_CALL_2262C_17 | BUBSY_GROUNDED_CASE32_CALL_3539C_1));
    assert(descriptor_counter_4D == 0);
    assert(bubsy_grounded_case32_update_state(0x40, 0, &actor_state_01,
        &descriptor_counter_4D, &descriptor_state_4C, &descriptor_state_68,
        &descriptor_state_02, &linked_counter_30) == BUBSY_GROUNDED_CASE32_NO_ACTION);
    assert(bubsy_grounded_case32_update_state(0, 0x20000, &actor_state_01,
        &descriptor_counter_4D, &descriptor_state_4C, &descriptor_state_68,
        &descriptor_state_02, &linked_counter_30) == BUBSY_GROUNDED_CASE32_NO_ACTION);
}

static void test_bubsy_grounded_case7(void) {
    uint8_t actor_state_01 = 0;
    uint8_t descriptor_state_4C = 4;
    uint8_t descriptor_state_68 = 5;
    int16_t descriptor_state_02 = 6;

    assert(bubsy_grounded_case7_update_state(0x1000, &actor_state_01,
        &descriptor_state_4C, &descriptor_state_68, &descriptor_state_02) ==
        BUBSY_GROUNDED_CASE7_SET_ACTOR_STATE);
    assert(actor_state_01 == 2 && descriptor_state_02 == 0);
    assert(descriptor_state_4C == 4 && descriptor_state_68 == 5);

    assert(bubsy_grounded_case7_update_state(0, &actor_state_01,
        &descriptor_state_4C, &descriptor_state_68, &descriptor_state_02) ==
        BUBSY_GROUNDED_CASE7_CALL_2269C_0F);
    assert(descriptor_state_4C == 0 && descriptor_state_68 == 1);
}

static void test_bubsy_grounded_cases9_11(void) {
    uint32_t actor_state_10 = 0x220;
    uint32_t actor_state_14 = 0x1234;
    uint8_t actor_state_01 = 0;
    int16_t descriptor_state_02 = 1;
    uint8_t expected = BUBSY_GROUNDED_CASE9_11_SELECT_2B5 |
        BUBSY_GROUNDED_CASE9_11_CALL_52E18 |
        BUBSY_GROUNDED_CASE9_11_CALL_4FF50 |
        BUBSY_GROUNDED_CASE9_11_CALL_34EF8;

    assert(bubsy_grounded_cases9_11_update_state(9, 0x2BB, &actor_state_10,
        &actor_state_14, &actor_state_01, &descriptor_state_02, 0, 1) == expected);
    assert(actor_state_10 == 0x220 && actor_state_14 == 0x1234);

    actor_state_10 = 0x620;
    actor_state_14 = 0x1234;
    assert(bubsy_grounded_cases9_11_update_state(11, 0x2B8, &actor_state_10,
        &actor_state_14, &actor_state_01, &descriptor_state_02, 1, 1) ==
        (BUBSY_GROUNDED_CASE9_11_SELECT_2B5 | BUBSY_GROUNDED_CASE9_11_CALL_52E18 |
            BUBSY_GROUNDED_CASE9_11_CALL_34EF8));
    assert(actor_state_10 == 0x220 && actor_state_14 == 0x1234);

    actor_state_10 = 0;
    actor_state_14 = 0x1234;
    assert(bubsy_grounded_cases9_11_update_state(9, 0, &actor_state_10,
        &actor_state_14, &actor_state_01, &descriptor_state_02, 0, 1) ==
        BUBSY_GROUNDED_CASE9_11_RESET_AND_CALL_3539C_1);
    assert(actor_state_10 == 0 && actor_state_14 == 0);

    actor_state_10 = 0x1000;
    actor_state_14 = 0x1234;
    actor_state_01 = 0;
    descriptor_state_02 = 5;
    assert(bubsy_grounded_cases9_11_update_state(11, 0x2B8, &actor_state_10,
        &actor_state_14, &actor_state_01, &descriptor_state_02, 0, 0) ==
        BUBSY_GROUNDED_CASE9_11_SET_ACTOR_STATE);
    assert(actor_state_01 == 2 && descriptor_state_02 == 0 && actor_state_14 == 0x1234);
    assert(bubsy_grounded_cases9_11_update_state(10, 0, &actor_state_10,
        &actor_state_14, &actor_state_01, &descriptor_state_02, 0, 0) ==
        BUBSY_GROUNDED_CASE9_11_NO_ACTION);
}

static void test_fixed_q12_dot_product(void) {
    const int32_t left[3] = {0x1000, 0x0800, -0x1000};
    const int32_t right[3] = {0x1000, 0x1000, 0x0800};
    int32_t result = 0;

    assert(func_800100A0(left, right, &result) == 0);
    assert(result == 0x1000);
}

static void test_vector_range_scaling(void) {
    int32_t input[3] = {16, -32, 48};
    int32_t output[3] = {0};

    assert(func_80053D68(input, output) == 0);
    assert(output[0] == 0x1000 && output[1] == -0x2000 && output[2] == 0x3000);

    input[0] = 0x1001;
    input[1] = -2;
    input[2] = 3;
    assert(func_80053D68(input, output) == 0);
    assert(output[0] == 0x4004 && output[1] == -8 && output[2] == 12);

    input[0] = 0x4001;
    input[1] = -32;
    input[2] = 16;
    assert(func_80053D68(input, output) == 0);
    assert(output[0] == 0x400 && output[1] == -2 && output[2] == 1);

    input[0] = 0x01000001;
    input[1] = -0x1000;
    input[2] = 0x800;
    assert(func_80053D68(input, output) == 0);
    assert(output[0] == 0x2000 && output[1] == -2 && output[2] == 1);
}

static void test_vector_length_q12(void) {
    const int32_t input[3] = {3 * 0x1000, 4 * 0x1000, 12 * 0x1000};
    const int32_t zero[3] = {0, 0, 0};
    int32_t output = 0;

    vector_length_result = 13 * 0x1000;
    vector_length_input = 0;
    vector_length_call_count = 0;
    vector_square_call_count = 0;
    assert(func_8005385C(input, &output) == 0);
    assert(vector_length_input == 169 * 0x1000);
    assert(output == 13 * 0x1000);
    assert(vector_square_call_count == 3 && vector_length_call_count == 1);

    vector_length_result = 0;
    vector_length_call_count = 0;
    vector_square_call_count = 0;
    assert(func_8005385C(zero, &output) == 0);
    assert(vector_length_input == 0 && output == 0);
    assert(vector_square_call_count == 0 && vector_length_call_count == 1);
}

static void test_grounded_vector_transform(void) {
    const int32_t input[3] = {16, -32, 48};
    int32_t output[3] = {1, 2, 3};

    vector_length_result = 0;
    assert(func_8005395C(input, output) == 7);
    assert(output[0] == 0 && output[1] == 0 && output[2] == 0);

    vector_length_result = 0x1000;
    assert(func_8005395C(input, output) == 0);
    assert(output[0] == 0x1000 && output[1] == -0x2000 && output[2] == 0x3000);
    assert(vector_length_input == 14 * 0x1000);
}

static void test_bubsy_actor_update_sequence_309_gate(void) {
    BubsyActorUpdateActorView actor = {0};

    actor.flags_04 = 0x80;
    assert(bubsy_actor_update_should_select_309(&actor, 0x308, 0) == 1);
    assert(bubsy_actor_update_should_select_309(&actor, 0x309, 0) == 0);
    assert(bubsy_actor_update_should_select_309(&actor, 0x308, 1) == 0);

    actor.flags_04 = 0x100;
    assert(bubsy_actor_update_should_select_309(&actor, 0x308, 0) == 0);

    actor.flags_04 = 0x80;
    actor.update_state_10 = 0x00800000;
    assert(bubsy_actor_update_should_select_309(&actor, 0x308, 0) == 0);
}

static void test_bubsy_actor_update_local_state_for_sequence(void) {
    BubsyActorUpdateActorView actor = {0};

    actor.flags_04 = 0x100;
    assert(bubsy_actor_update_local_state_for_sequence(&actor, 0, 0, 1) == 0);
    assert(bubsy_actor_update_local_state_for_sequence(&actor, 0x32E, 1, 7) == 7);

    actor.flags_04 = 0;
    assert(bubsy_actor_update_local_state_for_sequence(&actor, 0x32E, 0, 0) == 1);
    assert(bubsy_actor_update_local_state_for_sequence(&actor, 0x32D, 0, 7) == 7);
    assert(bubsy_actor_update_local_state_for_sequence(&actor, 0x32E, 1, 7) == 7);
}

static void test_bubsy_actor_update_mode1_sequence_boundary_gate(void) {
    assert(bubsy_actor_update_should_check_mode1_sequence_boundary(0x9A, 1) == 1);
    assert(bubsy_actor_update_should_check_mode1_sequence_boundary(0x54, 1) == 1);
    assert(bubsy_actor_update_should_check_mode1_sequence_boundary(0x35, 1) == 0);
    assert(bubsy_actor_update_should_check_mode1_sequence_boundary(0x54, 0) == 0);
    assert(bubsy_actor_update_should_check_mode1_sequence_boundary(0x9A, 2) == 0);
}

static void test_bubsy_actor_update_sequence_counter_reset(void) {
    BubsyActorUpdateActorView actor = {0};

    assert(bubsy_actor_update_should_reset_sequence_counter(&actor, 12, 1, 0) == 1);
    assert(bubsy_actor_update_should_reset_sequence_counter(&actor, 11, 1, 0) == 0);
    assert(bubsy_actor_update_should_reset_sequence_counter(&actor, 12, 0, 0) == 0);
    assert(bubsy_actor_update_should_reset_sequence_counter(&actor, 1, 0, 0x14) == 1);

    actor.flags_04 = 0x40;
    assert(bubsy_actor_update_should_reset_sequence_counter(&actor, 1, 0, 0) == 1);
}

static void test_bubsy_actor_update_level14_counter_path(void) {
    BubsyActorUpdateActorView actor = {0};

    assert(bubsy_actor_update_should_enter_level14_counter_path(&actor, 1, 0, 0x14, 0) == 1);
    assert(bubsy_actor_update_should_enter_level14_counter_path(&actor, 0, 0, 0x14, 0) == 0);
    assert(bubsy_actor_update_should_enter_level14_counter_path(&actor, 1, 1, 0x14, 0) == 0);
    assert(bubsy_actor_update_should_enter_level14_counter_path(&actor, 1, 0, 0x13, 0) == 0);
    assert(bubsy_actor_update_should_enter_level14_counter_path(&actor, 1, 0, 0x14, 1) == 0);

    actor.flags_04 = 0x40;
    assert(bubsy_actor_update_should_enter_level14_counter_path(&actor, 1, 0, 0x14, 0) == 0);
}

static void test_bubsy_actor_update_sequence_index_for_local_state(void) {
    BubsyActorUpdateActorView actor = {0};

    assert(bubsy_actor_update_sequence_index_for_local_state(&actor, 0, 0) == -1);
    assert(bubsy_actor_update_sequence_index_for_local_state(&actor, 1, 0) == 0x35);
    assert(bubsy_actor_update_sequence_index_for_local_state(&actor, 1, 1) == 0x54);

    actor.flags_04 = 0x08;
    assert(bubsy_actor_update_sequence_index_for_local_state(&actor, 1, 1) == 0x9A);
    actor.flags_04 = 0x800;
    assert(bubsy_actor_update_sequence_index_for_local_state(&actor, 1, 2) == 0x9A);
}

static void test_bubsy_actor_update_sequence_mode_override(void) {
    int32_t override_value = 0x1234;

    assert(bubsy_actor_update_get_sequence_mode_override(0, 1, &override_value) == 0);
    assert(override_value == 0x1234);
    assert(bubsy_actor_update_get_sequence_mode_override(1, 0, &override_value) == 0);
    assert(override_value == 0x1234);
    assert(bubsy_actor_update_get_sequence_mode_override(1, 2, &override_value) == 1);
    assert(override_value == -0xF40);
}

static void test_bubsy_death_state_progression(void) {
    assert(bubsy_advance_death_state(-1, 0, 0) == 1);
    assert(bubsy_advance_death_state(1, 0, 0) == 1);
    assert(bubsy_advance_death_state(1, 0, 1) == 7);
    assert(bubsy_advance_death_state(1, 0, 2) == 9);
    assert(bubsy_advance_death_state(1, 0, -1) == 1);
    assert(bubsy_advance_death_state(2, 0, 0) == 2);
    assert(bubsy_advance_death_state(2, 0, 1) == 10);
    assert(bubsy_advance_death_state(2, 0, -1) == 2);
    assert(bubsy_advance_death_state(3, 6, 0) == 12);
    assert(bubsy_advance_death_state(4, 4, 0) == 11);
    assert(bubsy_advance_death_state(4, 7, 0) == 6);
    assert(bubsy_advance_death_state(4, 9, 0) == 6);
    assert(bubsy_advance_death_state(4, 18, 0) == 6);
    assert(bubsy_advance_death_state(4, 6, 0) == 11);
    assert(bubsy_advance_death_state(4, 6, 1) == 11);
    assert(bubsy_advance_death_state(4, 8, 0) == 11);
    assert(bubsy_advance_death_state(4, 8, 1) == 11);
    assert(bubsy_advance_death_state(4, 5, 0) == 6);
    assert(bubsy_advance_death_state(11, 4, 0) == 11);
    assert(bubsy_advance_death_state(11, 4, 1) == 11);
    assert(bubsy_advance_death_state(11, 8, 1) == 11);
    assert(bubsy_advance_death_state(7, 5, 0) == 7);
    assert(bubsy_advance_death_state(7, 8, 0) == 12);
    assert(bubsy_advance_death_state(7, 8, 1) == 13);
    assert(bubsy_advance_death_state(1, 8, -1) == 13);
    assert(bubsy_advance_death_state(1, 8, 0) == 12);
    assert(bubsy_advance_death_state(2, 6, -1) == 13);
}

static unsigned int death_assertion_count;

static void record_death_assertion(int32_t failed, const char *condition, const char *source_path, uint32_t source_line) {
    assert(failed == 0);
    assert(strcmp(condition, "gBubsyInfo.deathType >= 0") == 0);
    assert(strcmp(source_path, "../f/bubsy.c") == 0);
    assert(source_line == 0x912);
    death_assertion_count++;
}

static void test_bubsy_death_state_initial_normalization(void) {
    death_assertion_count = 0;
    assert(bubsy_death_state_normalize_initial(3, record_death_assertion) == 3);
    assert(death_assertion_count == 0);
    assert(bubsy_death_state_normalize_initial(-1, record_death_assertion) == 1);
    assert(death_assertion_count == 1);
}

static void test_bubsy_death_state_entry_event_gate(void) {
    assert(bubsy_death_state_should_dispatch_entry_event(-2, 0) == 1);
    assert(bubsy_death_state_should_dispatch_entry_event(-2, 1) == 0);
    assert(bubsy_death_state_should_dispatch_entry_event(0, 0) == 0);
    assert(bubsy_death_state_should_run_level13_cleanup(0x13) == 1);
    assert(bubsy_death_state_should_run_level13_cleanup(0x12) == 0);
    assert(bubsy_death_state_should_run_main_loop(0) == 1);
    assert(bubsy_death_state_should_run_main_loop(1) == 0);
    assert(bubsy_death_state_uses_counter_entry_list(-2) == 1);
    assert(bubsy_death_state_uses_counter_entry_list(-1) == 1);
    assert(bubsy_death_state_uses_counter_entry_list(0) == 0);
    assert(bubsy_death_state_next_global_counter(-2, 1) == -1);
    assert(bubsy_death_state_next_global_counter(-1, 1) == 0);
    assert(bubsy_death_state_next_global_counter(4, 0) == 4);
    assert(bubsy_death_state_next_global_counter(4, 1) == 0);
    assert(bubsy_death_state_should_dispatch_counter_entry_event(-1, 0, 1, 0) == 1);
    assert(bubsy_death_state_should_dispatch_counter_entry_event(-2, 0, 2, 0) == 1);
    assert(bubsy_death_state_should_dispatch_counter_entry_event(0, 0, 1, 0) == 0);
    assert(bubsy_death_state_should_dispatch_counter_entry_event(-1, 1, 1, 0) == 0);
    assert(bubsy_death_state_should_dispatch_counter_entry_event(-1, 0, 0, 0) == 0);
    assert(bubsy_death_state_should_dispatch_counter_entry_event(-1, 0, 1, 1) == 0);
}

static void test_bubsy_death_state_runtime_reset(void) {
    BubsyDeathStateRuntimeView runtime = {0xFFFFFFFF, 0xFFFFFFFF, 1, 1, 0, 1, 1, 1, 1, 0xFFFFFFFF};

    bubsy_death_state_reset_runtime(&runtime, 3);
    assert(runtime.actor_flags_04 == 0 && runtime.actor_state_10 == 0);
    assert(runtime.actor_event_guard_6460 == 0 && runtime.state_6461 == 0);
    assert(runtime.state_6463 == 1 && runtime.state_6479 == 0);
    assert(runtime.state_64B8 == 0 && runtime.state_649D == 0);
    assert(runtime.state_6452 == 0 && runtime.state_64C0 == 0);

    runtime.state_6463 = 1;
    bubsy_death_state_reset_runtime(&runtime, 0);
    assert(runtime.state_6463 == 0);
}

static void test_bubsy_death_state_finalization(void) {
    uint8_t actor_state_2C = 0xFF;
    uint32_t actor_component_state_20 = 0xFFFFFFFF;
    uint32_t nested_state_words[11];
    int32_t global_counter_3A4 = 0;
    uint8_t state_6462 = 0;
    uint8_t state_6464 = 0xFF;
    BubsyDeathStateFinalizationView runtime = {
        .actor_state_2C = &actor_state_2C,
        .actor_component_state_20 = &actor_component_state_20,
        .global_counter_3A4 = &global_counter_3A4,
        .state_6462 = &state_6462,
        .state_6464 = &state_6464,
    };
    unsigned int index;

    for (index = 0; index < 11; index++) {
        nested_state_words[index] = 0xFFFFFFFF;
        runtime.nested_state_words[index] = &nested_state_words[index];
    }

    bubsy_death_state_finalize_runtime(&runtime);
    assert(actor_state_2C == 0 && actor_component_state_20 == 0);
    assert(global_counter_3A4 == -2 && state_6462 == 1 && state_6464 == 0);
    for (index = 0; index < 11; index++) {
        assert(nested_state_words[index] == 0);
    }
}

int main(void) {
    PlayerModelState failed_state = {0};
    LevelModelConfig failed_config = {0};

    test_model_selection(0, 0, "BUB.TZP");
    test_model_selection(0, 1, "BUBSWIM.TZP");
    test_model_selection(1, 0, "PLISKIN.TZP");
    test_model_selection(1, 1, "PLISWIM.TZP");
    test_actor_flag_helpers();
    test_actor_sequence_boundary();
    test_actor_sequence_helpers();
    test_actor_sequence_state_selection();
    test_actor_shared_state_progress_reset();
    test_actor_shared_state_mode1_numeric_update();
    test_actor_shared_state_mode0_progress_clamp();
    test_actor_shared_state_mode23_plan();
    test_actor_state23_event_17_gate();
    test_actor_state_target_data();
    test_move_request_queue();
    test_actor_event_dispatch();
    test_bubsy_actor_update_entry_gate();
    test_bubsy_actor_default_state_route();
    test_bubsy_grounded_dispatch_table();
    test_bubsy_grounded_handler_prelude();
    test_bubsy_grounded_case38_descriptor();
    test_bubsy_grounded_case37();
    test_bubsy_grounded_cases29_30();
    test_bubsy_grounded_cases13_34_36();
    test_bubsy_grounded_case21();
    test_bubsy_grounded_case12();
    test_bubsy_grounded_case17();
    test_bubsy_grounded_case16();
    test_bubsy_grounded_case14();
    test_bubsy_grounded_case15();
    test_bubsy_grounded_case6();
    test_bubsy_grounded_cases33_35();
    test_bubsy_grounded_case20();
    test_bubsy_grounded_case31_counter();
    test_bubsy_grounded_case31_followups();
    test_bubsy_grounded_cases8_10();
    test_bubsy_grounded_cases8_10_counter_path();
    test_bubsy_grounded_case32();
    test_bubsy_grounded_case7();
    test_bubsy_grounded_cases9_11();
    test_fixed_q12_dot_product();
    test_vector_range_scaling();
    test_vector_length_q12();
    test_grounded_vector_transform();
    test_bubsy_actor_update_sequence_309_gate();
    test_bubsy_actor_update_local_state_for_sequence();
    test_bubsy_actor_update_mode1_sequence_boundary_gate();
    test_bubsy_actor_update_sequence_counter_reset();
    test_bubsy_actor_update_level14_counter_path();
    test_bubsy_actor_update_sequence_index_for_local_state();
    test_bubsy_actor_update_sequence_mode_override();
    test_bubsy_death_state_progression();
    test_bubsy_death_state_initial_normalization();
    test_bubsy_death_state_entry_event_gate();
    test_bubsy_death_state_runtime_reset();
    test_bubsy_death_state_finalization();

    failed_state.config = &failed_config;
    failed_state.is_swimming = 0;
    failed_config.flags_2A2A = 0;
    loader_result = 0;
    assertion_count = 0;
    assert(level_select_player_model_resource(&failed_state, fake_load, fake_assert) == 0);
    assert(assertion_count == 1);

    return 0;
}