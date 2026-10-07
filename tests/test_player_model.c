#include <assert.h>
#include <stdint.h>
#include <string.h>

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

int main(void) {
    PlayerModelState failed_state = {0};
    LevelModelConfig failed_config = {0};

    test_model_selection(0, 0, "BUB.TZP");
    test_model_selection(0, 1, "BUBSWIM.TZP");
    test_model_selection(1, 0, "PLISKIN.TZP");
    test_model_selection(1, 1, "PLISWIM.TZP");
    test_actor_flag_helpers();

    failed_state.config = &failed_config;
    failed_state.is_swimming = 0;
    failed_config.flags_2A2A = 0;
    loader_result = 0;
    assertion_count = 0;
    assert(level_select_player_model_resource(&failed_state, fake_load, fake_assert) == 0);
    assert(assertion_count == 1);

    return 0;
}