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
    const int32_t entries[] = {250, -1, 350, 50};
    PlayerActorSequenceData sequence = {.entries = entries};
    PlayerActorSequenceView actor = {0};
    uint8_t result = 0xFF;

    actor.cursor_08 = 250;
    actor.threshold_0E = 2;
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
    const int32_t entries[] = {250, -101, 350};
    PlayerActorSequenceData sequence = {.entries = entries};
    PlayerActorSequenceView actor = {0};
    int32_t result = 0;

    actor.sequence_18 = &sequence;
    actor.cursor_08 = 0;
    assert(player_actor_read_sequence_remainder(&actor, &result) == 0);
    assert(result == 50);
    actor.cursor_08 = 1;
    assert(player_actor_read_sequence_remainder(&actor, &result) == 0);
    assert(result == -1);

    sequence.flags_2C = 0x04;
    result = -1;
    assert(player_actor_read_sequence_remainder(&actor, &result) == 0);
    assert(result == 0);

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

static EventCallLog run_actor_event(uint8_t update_mode, uint8_t runtime_mode, uint16_t event_id, uint8_t state_6463) {
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
    assert(bubsy_process_actor_event(update_mode, &ops) == (update_mode > 1 ? 1 : 0x42));
    return log;
}

static void test_actor_event_dispatch(void) {
    EventCallLog log = run_actor_event(0, 0, BUBSY_EVENT_PROPERTY_KIND_3E0, 0);
    assert(log.selected_count == 1 && log.property_id == BUBSY_EVENT_PROPERTY_NORMAL);
    assert(log.flag_count == 1 && log.dispatch_count == 1 && log.dispatch_flags == 0);

    log = run_actor_event(0, 1, BUBSY_EVENT_PROPERTY_KIND_157, 0);
    assert(log.selected_count == 1 && log.property_id == BUBSY_EVENT_PROPERTY_SWIM_MODE);
    assert(log.flag_count == 1 && log.dispatch_count == 1 && log.dispatch_flags == 0);

    log = run_actor_event(0, 0, BUBSY_EVENT_PROPERTY_KIND_3E0, 3);
    assert(log.selected_count == 1 && log.property_id == BUBSY_EVENT_PROPERTY_KIND_3E0);
    assert(log.dispatch_flags == BUBSY_EVENT_SPECIAL_FLAG);

    log = run_actor_event(1, 1, BUBSY_EVENT_PROPERTY_KIND_157, 0);
    assert(log.selected_count == 1 && log.property_id == BUBSY_EVENT_PROPERTY_KIND_157);
    assert(log.dispatch_flags == BUBSY_EVENT_SPECIAL_FLAG);

    log = run_actor_event(2, 0, BUBSY_EVENT_PROPERTY_KIND_3E0, 0);
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
    assert(bubsy_advance_death_state(3, 6, 0) == 3);
    assert(bubsy_advance_death_state(4, 4, 0) == 6);
    assert(bubsy_advance_death_state(4, 7, 0) == 6);
    assert(bubsy_advance_death_state(4, 9, 0) == 6);
    assert(bubsy_advance_death_state(4, 18, 0) == 6);
    assert(bubsy_advance_death_state(4, 6, 0) == 8);
    assert(bubsy_advance_death_state(4, 8, 0) == 12);
    assert(bubsy_advance_death_state(4, 8, 1) == 13);
    assert(bubsy_advance_death_state(4, 5, 0) == 4);
    assert(bubsy_advance_death_state(11, 4, 0) == 12);
    assert(bubsy_advance_death_state(11, 4, 1) == 13);
    assert(bubsy_advance_death_state(7, 5, 0) == 11);
    assert(bubsy_advance_death_state(7, 8, 0) == 12);
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
    test_actor_state_target_data();
    test_move_request_queue();
    test_actor_event_dispatch();
    test_bubsy_actor_update_entry_gate();
    test_bubsy_actor_update_sequence_309_gate();
    test_bubsy_actor_update_local_state_for_sequence();
    test_bubsy_actor_update_mode1_sequence_boundary_gate();
    test_bubsy_actor_update_sequence_counter_reset();
    test_bubsy_actor_update_level14_counter_path();
    test_bubsy_actor_update_sequence_index_for_local_state();
    test_bubsy_actor_update_sequence_mode_override();
    test_bubsy_death_state_progression();

    failed_state.config = &failed_config;
    failed_state.is_swimming = 0;
    failed_config.flags_2A2A = 0;
    loader_result = 0;
    assertion_count = 0;
    assert(level_select_player_model_resource(&failed_state, fake_load, fake_assert) == 0);
    assert(assertion_count == 1);

    return 0;
}