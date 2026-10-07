#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "bubsy_actor_events.h"
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
    test_move_request_queue();
    test_actor_event_dispatch();

    failed_state.config = &failed_config;
    failed_state.is_swimming = 0;
    failed_config.flags_2A2A = 0;
    loader_result = 0;
    assertion_count = 0;
    assert(level_select_player_model_resource(&failed_state, fake_load, fake_assert) == 0);
    assert(assertion_count == 1);

    return 0;
}