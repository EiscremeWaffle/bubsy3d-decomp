#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "bubsy_actor_update.h"
#include "bubsy_actor_events.h"
#include "bubsy_death_state.h"
#include "component_transform.h"
#include "move_requests.h"
#include "player_model.h"

extern int32_t func_800536F4(const int32_t *input, int32_t *output);
extern int32_t func_8002262C(void *actor, uint8_t selector);
extern int32_t func_8002269C(void *actor, uint8_t selector);
extern int32_t func_80022650(void *actor, uint8_t selector);
extern int32_t func_8003AA1C(void **output);
extern uint32_t func_8001DCE0(void);
extern int32_t func_800516F8(void **output);
extern int32_t func_80054414(const GameComponentBasisView *input, GameComponentBasisView *output);
extern int32_t func_80054488(const GameComponentBasisView *input, GameComponentBasisView *output);
extern int32_t func_800546F4(const GameComponentBasisView *input, GameComponentBasisView *output);
extern int32_t func_8005476C(const GameComponentBasisView *input, GameComponentBasisView *output);
extern int32_t func_8005406C(const int32_t input[4], const GameComponentBasisView *basis, int32_t output[4]);
extern int32_t func_80054120(const int32_t input[4], const GameComponentBasisView *basis, int32_t output[4]);
uint32_t g_l0_gp_170;
void *g_l0_gp_888;
extern uint32_t func_80056678(uint32_t value, uint32_t selector);
extern int32_t func_8005668C(uint32_t *state, uint8_t selector);
extern int32_t func_800566E8(uint32_t *state, uint8_t selector);

static uint16_t g_inverse_basis_matrix[9];
static int32_t g_inverse_basis_translation[3];
static int32_t g_inverse_basis_input[4];
static unsigned int g_inverse_basis_call_count;
static unsigned int g_point_transform_word_count = 4;

int32_t func_80010000(const int32_t left[3], const int32_t right[3], int32_t output[3]) {
    unsigned int index;

    for (index = 0; index < 3; index++) {
        output[index] = left[index] - right[index];
    }
    return 0;
}

int32_t func_80053FA4(const int32_t input[4], const volatile GameComponentBasisView *basis, int32_t output[4]) {
    unsigned int index;

    for (index = 0; index < 9; index++) {
        g_inverse_basis_matrix[index] = basis->matrix_halfwords[index];
    }
    for (index = 0; index < 3; index++) {
        g_inverse_basis_translation[index] = basis->translation[index];
    }
    for (index = 0; index < g_point_transform_word_count; index++) {
        g_inverse_basis_input[index] = input[index];
        output[index] = input[index] + (int32_t)index;
    }
    g_inverse_basis_call_count++;
    return -7;
}

static void test_grounded_movement_helpers(void) {
    static const int32_t inputs[] = {
        0, 1, -1, 4095, -4095, 4096, -4096, 4097, -4097,
        INT32_MAX, INT32_MIN
    };
    static const int32_t expected[] = {
        0, 0, 0, 0, 0, 1, -1, 1, -1, 524287, 524288
    };
    static const uint32_t states[] = {0, UINT32_MAX, 0x55555555u, 0xAAAAAAAAu};
    uint32_t actor[8] = {0};
    unsigned int index;
    unsigned int state_index;

    for (index = 0; index < sizeof(inputs) / sizeof(inputs[0]); index++) {
        int32_t vector[3] = {inputs[index], inputs[index], inputs[index]};
        int32_t output[4] = {42, 42, 42, 99};
        assert(func_800536F4(vector, output) == 0);
        assert(output[0] == expected[index]);
        assert(output[1] == expected[index]);
        assert(output[2] == expected[index]);
        assert(output[3] == 99);
        assert(func_800536F4(vector, vector) == 0);
        assert(vector[0] == expected[index]);
        assert(vector[1] == expected[index]);
        assert(vector[2] == expected[index]);
    }
    for (index = 0; index < 256; index++) {
        uint32_t mask = 1u << (index & 31u);
        for (state_index = 0; state_index < sizeof(states) / sizeof(states[0]); state_index++) {
            uint32_t original = states[state_index];
            uint32_t present = (original & mask) != 0;
            uint32_t state = original;
            uint32_t set_result = present ? 1u : original | mask;
            uint32_t clear_result = present ? original & ~mask : 0u;

            assert(func_80056678(original, index) == present);
            assert(func_80056678(original, index + 0x10000u) == present);
            assert((uint32_t)func_8005668C(&state, (uint8_t)index) == set_result);
            assert(state == (original | mask));
            assert(func_8005668C(&state, (uint8_t)index) == 1);
            assert(state == (original | mask));
            state = original;
            assert((uint32_t)func_800566E8(&state, (uint8_t)index) == clear_result);
            assert(state == (original & ~mask));
            assert(func_800566E8(&state, (uint8_t)index) == 0);
            assert(state == (original & ~mask));

            actor[4] = original;
            assert((uint32_t)func_80022650(actor, (uint8_t)index) == set_result);
            assert(actor[4] == (original | mask));
            actor[4] = original;
            assert((uint32_t)func_8002262C(actor, (uint8_t)index) == clear_result);
            assert(actor[4] == (original & ~mask));
            actor[1] = original;
            assert((uint32_t)func_8002269C(actor, (uint8_t)index) == clear_result);
            assert(actor[1] == (original & ~mask));
        }
    }
    assert(func_80056678(0x80000000u, UINT32_MAX) == 1);
    for (index = 0; index < sizeof(actor) / sizeof(actor[0]); index++) {
        if (index != 1 && index != 4) {
            assert(actor[index] == 0);
        }
    }
    {
        void *output = NULL;
        assert(func_8003AA1C(&output) == 1);
        assert((uintptr_t)output == 0x80186450u);
    }
    g_l0_gp_170 = 0x13579BDFu;
    assert(func_8001DCE0() == 0x13579BDFu);
    g_l0_gp_888 = (void *)(uintptr_t)0x12345678u;
    {
        void *output = NULL;
        assert(func_800516F8(&output) == 0);
        assert(output == g_l0_gp_888);
    }
}

static void test_component_basis_copy_helpers(void) {
    static const uint16_t transposed[9] = {1, 4, 7, 2, 5, 8, 3, 6, 9};
    GameComponentBasisView input = {
        .matrix_halfwords = {1, 2, 3, 4, 5, 6, 7, 8, 9},
        .ignored_halfword_12 = 0xBEEF,
        .translation = {0x12345678, -0x234567, 0x3456789},
    };
    GameComponentBasisView output;
    size_t index;

    memset(&output, 0xA5, sizeof(output));
    assert(func_80054414(&input, &output) == 0);
    for (index = 0; index < 9; index++) {
        assert(output.matrix_halfwords[index] == transposed[index]);
    }
    assert(output.ignored_halfword_12 == 0xA5A5);
    assert(memcmp(output.translation, input.translation, sizeof(input.translation)) == 0);

    memset(&output, 0x5A, sizeof(output));
    assert(func_80054488(&input, &output) == 0);
    assert(memcmp(output.matrix_halfwords, input.matrix_halfwords, sizeof(input.matrix_halfwords)) == 0);
    assert(output.ignored_halfword_12 == 0x5A5A);
    for (index = 0; index < sizeof(output.translation); index++) {
        assert(((const uint8_t *)output.translation)[index] == 0x5A);
    }
}

static void test_component_basis_normalization_helpers(void) {
    static const uint16_t normalized[9] = {
        0x1234, 0x7FFF, 0x2345, 0x7FFE, 0x3456,
        0x7FFD, 0x4567, 0x7FFC, 0x5678
    };
    const GameComponentBasisView input = {
        .matrix_halfwords = {
            0x1234, 0x8001, 0x2345, 0x8002, 0x3456,
            0x8003, 0x4567, 0x8004, 0x5678
        },
        .ignored_halfword_12 = 0xBEEF,
        .translation = {123, -456, 789},
    };
    GameComponentBasisView output;
    size_t index;

    memset(&output, 0x5A, sizeof(output));
    assert(func_800546F4(&input, &output) == 0);
    assert(memcmp(output.matrix_halfwords, normalized, sizeof(normalized)) == 0);
    assert(output.ignored_halfword_12 == 0x5A5A);
    assert(output.translation[0] == 0);
    assert(output.translation[1] == 0);
    assert(output.translation[2] == 0);

    memset(&output, 0xA5, sizeof(output));
    assert(func_800546F4(NULL, &output) == 0x18);
    for (index = 0; index < sizeof(output); index++) {
        assert(((const uint8_t *)&output)[index] == 0xA5);
    }

    memset(&output, 0x5A, sizeof(output));
    assert(func_8005476C(&input, &output) == 0);
    assert(memcmp(output.matrix_halfwords, normalized, sizeof(normalized)) == 0);
    assert(output.ignored_halfword_12 == 0x5A5A);
    assert(output.translation[0] == input.translation[0]);
    assert(output.translation[1] == 456);
    assert(output.translation[2] == input.translation[2]);
}

static void test_component_inverse_basis_wrapper(void) {
    static const uint16_t transposed[9] = {1, 4, 7, 2, 5, 8, 3, 6, 9};
    GameComponentBasisView input = {
        .matrix_halfwords = {1, 2, 3, 4, 5, 6, 7, 8, 9},
        .ignored_halfword_12 = 0xABCD,
        .translation = {101, -202, 303},
    };
    const int32_t point[4] = {11, 22, 33, 44};
    int32_t output[4] = {0};
    unsigned int index;

    g_inverse_basis_call_count = 0;
    assert(func_8005406C(point, &input, output) == 0);
    assert(g_inverse_basis_call_count == 1);
    assert(memcmp(g_inverse_basis_matrix, transposed, sizeof(transposed)) == 0);
    assert(g_inverse_basis_translation[0] == input.translation[0]);
    assert(g_inverse_basis_translation[1] == input.translation[1]);
    assert(g_inverse_basis_translation[2] == input.translation[2]);
    for (index = 0; index < 4; index++) {
        assert(g_inverse_basis_input[index] == point[index]);
        assert(output[index] == point[index] + (int32_t)index);
    }

    {
        const int32_t point_to_convert[4] = {100, 200, 300, 400};
        const int32_t expected_input[3] = {90, 220, 270};
        int32_t converted[4] = {0, 0, 0, 0x5A5A5A5A};

        input.translation[0] = 10;
        input.translation[1] = -20;
        input.translation[2] = 30;
        g_point_transform_word_count = 3;
        g_inverse_basis_call_count = 0;
        assert(func_80054120(point_to_convert, &input, converted) == 0);
        assert(g_inverse_basis_call_count == 1);
        assert(memcmp(g_inverse_basis_input, expected_input, sizeof(expected_input)) == 0);
        assert(converted[0] == 90);
        assert(converted[1] == 221);
        assert(converted[2] == 272);
        assert(converted[3] == 0x5A5A5A5A);
        g_point_transform_word_count = 4;
    }
}

static void test_component_basis_transform_point_q12(void) {
    GameComponentBasisView basis = {
        .matrix_halfwords = {
            0x1000, 0, 0,
            0, 0x1000, 0,
            0, 0, 0x1000,
        },
        .translation = {900, -800, 700},
    };
    const int32_t point[4] = {123, -456, 789, 0x11223344};
    const int32_t sentinel = (int32_t)0xA5A5A5A5u;
    int32_t output[4] = {0};

    assert(game_component_basis_transform_point_q12(&basis, point, output) == 0);
    assert(memcmp(output, point, sizeof(point)) == 0);

    basis.matrix_halfwords[0] = 0;
    basis.matrix_halfwords[2] = 0x1000;
    basis.matrix_halfwords[3] = 0;
    basis.matrix_halfwords[8] = 0;
    basis.matrix_halfwords[6] = 0xF000;
    basis.matrix_halfwords[1] = 0;
    basis.matrix_halfwords[4] = 0x1000;
    {
        const int32_t rotated_point[4] = {0x1000, 0x2000, -0x3000, 0x55667788};
        output[3] = sentinel;
        assert(game_component_basis_transform_point_q12(&basis, rotated_point, output) == 0);
        assert(output[0] == -0x3000);
        assert(output[1] == 0x2000);
        assert(output[2] == -0x1000);
        assert(output[3] == sentinel);
    }

    basis.matrix_halfwords[0] = 0x0800;
    basis.matrix_halfwords[1] = 0;
    basis.matrix_halfwords[2] = 0;
    basis.matrix_halfwords[3] = 0;
    basis.matrix_halfwords[4] = 0x0800;
    basis.matrix_halfwords[5] = 0;
    basis.matrix_halfwords[6] = 0;
    basis.matrix_halfwords[7] = 0;
    basis.matrix_halfwords[8] = 0x0800;
    {
        const int32_t fractional_point[4] = {1, -1, -3, 4};
        assert(game_component_basis_transform_point_q12(&basis, fractional_point, output) == 0);
        assert(output[0] == 0);
        assert(output[1] == -1);
        assert(output[2] == -2);
        assert(output[3] == sentinel);
    }

    basis.matrix_halfwords[0] = 0xA04F;
    basis.matrix_halfwords[1] = 0;
    basis.matrix_halfwords[2] = 0;
    basis.matrix_halfwords[3] = 0;
    basis.matrix_halfwords[4] = 0x1000;
    basis.matrix_halfwords[5] = 0;
    basis.matrix_halfwords[6] = 0;
    basis.matrix_halfwords[7] = 0;
    basis.matrix_halfwords[8] = 0x1000;
    {
        const int32_t large_point[4] = {-1898136935, 0, 0, 4};
        assert(game_component_basis_transform_point_q12(&basis, large_point, output) == 0);
        assert(output[0] == -1491271101);
        assert(output[1] == 0);
        assert(output[2] == 0);
        assert(output[3] == sentinel);
    }

    basis.matrix_halfwords[0] = 0x1001;
    {
        const int32_t inputs[] = {
            32767, 32768, -32768, -32769, INT32_MAX, INT32_MIN,
        };
        const int32_t expected[] = {
            32774, 32776, -32776, -32778, -2, 0,
        };
        int index;

        for (index = 0; index < (int)(sizeof(inputs) / sizeof(inputs[0])); index++) {
            const int32_t boundary_point[4] = {inputs[index], 0, 0, 0x11223344};
            output[3] = sentinel;
            assert(game_component_basis_transform_point_q12(&basis, boundary_point, output) == 0);
            assert(output[0] == expected[index]);
            assert(output[1] == 0);
            assert(output[2] == 0);
            assert(output[3] == sentinel);
        }
    }

    assert(game_component_basis_transform_point_q12(NULL, point, output) == -1);
    assert(game_component_basis_transform_point_q12(&basis, NULL, output) == -1);
    assert(game_component_basis_transform_point_q12(&basis, point, NULL) == -1);
}

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

static void test_move_request_enqueue_if_translation_pending(void) {
    int request;
    void *entries[2] = {0};
    int32_t translation[3] = {0, 0, 0};
    GameMoveRequestQueue queue = {0, entries};

    assert(game_enqueue_move_request_if_translation_pending(
        &queue, &request, translation, record_move_limit_assertion
    ) == 1);
    assert(queue.count == 0 && entries[0] == NULL);

    translation[2] = -1;
    assert(game_enqueue_move_request_if_translation_pending(
        &queue, &request, translation, record_move_limit_assertion
    ) == 1);
    assert(queue.count == 1 && entries[0] == &request);
}

static void test_move_request_enqueue_if_actor_state_bit_2(void) {
    int actor;
    void *entries[1] = {0};
    GameMoveRequestQueue queue = {0, entries};

    game_enqueue_move_request_if_actor_state_bit_2(
        &queue, &actor, 0x2u, record_move_limit_assertion
    );
    assert(queue.count == 0 && entries[0] == NULL);

    game_enqueue_move_request_if_actor_state_bit_2(
        &queue, &actor, 0x4u, record_move_limit_assertion
    );
    assert(queue.count == 1 && entries[0] == &actor);
}

static void test_move_request_caller_plans(void) {
    assert(game_move_request_plan_15a54_state_12(0, 0, 0) ==
        (GAME_MOVE_CALLER_CLEAR_GP_14 | GAME_MOVE_CALLER_CALL_80052AEC |
            GAME_MOVE_CALLER_SET_ACTOR_FLAG_04 | GAME_MOVE_CALLER_CALL_80022C94_WITH_380 |
            GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_TRANSITION_0C_4));
    assert(game_move_request_plan_15a54_state_12(4, 0, 1) ==
        (GAME_MOVE_CALLER_CLEAR_GP_14 | GAME_MOVE_CALLER_CALL_80022C94 |
            GAME_MOVE_CALLER_CALL_800691B8 | GAME_MOVE_CALLER_CONTINUE_POST_ENQUEUE));
    assert(game_move_request_plan_15a54_state_12(4, 4, 0) ==
        (GAME_MOVE_CALLER_CLEAR_GP_14 | GAME_MOVE_CALLER_CALL_80022C94 |
            GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_TRANSITION_0C_4));

    assert(game_move_request_plan_3d5d8(1, 2, 1) ==
        (GAME_MOVE_CALLER_SET_ACTOR_BYTE_01 | GAME_MOVE_CALLER_UPDATE_LINKED_STATE |
            GAME_MOVE_CALLER_TRANSITION_26_4 | GAME_MOVE_CALLER_TRANSITION_2F_1 |
            GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_CALL_80028764 |
            GAME_MOVE_CALLER_CALL_80053270 | GAME_MOVE_CALLER_CALL_800509A4));
    assert(game_move_request_plan_3d5d8(0, 2, 1) == 0);
    assert(game_move_request_plan_3d5d8(1, 1, 0) == 0);
    assert(game_move_request_plan_3d5d8(1, 2, 2) == 0);

    assert(game_move_request_plan_3d718(1, 0, 1) ==
        (GAME_MOVE_CALLER_CALL_80022944 | GAME_MOVE_CALLER_TRANSITION_2F_1 |
            GAME_MOVE_CALLER_ENQUEUE));
    assert(game_move_request_plan_3d718(1, 0, 2) ==
        (GAME_MOVE_CALLER_CALL_80022944 | GAME_MOVE_CALLER_TRANSITION_2F_1 |
            GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_TRANSITION_5_3));
    assert(game_move_request_plan_3d718(2, 1, 0) ==
        (GAME_MOVE_CALLER_CALL_80022944 | GAME_MOVE_CALLER_CALL_8001FD6C |
            GAME_MOVE_CALLER_CALL_80022674 |
            GAME_MOVE_CALLER_TRANSITION_1B_3 | GAME_MOVE_CALLER_CALL_8002205C));
    assert(game_move_request_plan_3d718(2, 0, 1) ==
        (GAME_MOVE_CALLER_CALL_80022944 | GAME_MOVE_CALLER_CALL_8001FD6C |
            GAME_MOVE_CALLER_CALL_80022674 |
            GAME_MOVE_CALLER_TRANSITION_2F_1 | GAME_MOVE_CALLER_ENQUEUE));
    assert(game_move_request_plan_3d718(2, 0, 2) ==
        (GAME_MOVE_CALLER_CALL_80022944 | GAME_MOVE_CALLER_CALL_8001FD6C |
            GAME_MOVE_CALLER_CALL_80022674 |
            GAME_MOVE_CALLER_TRANSITION_2F_1 | GAME_MOVE_CALLER_ENQUEUE |
            GAME_MOVE_CALLER_TRANSITION_5_3));
    assert(game_move_request_plan_4193c(0) ==
        (GAME_MOVE_CALLER_CALL_80022944 | GAME_MOVE_CALLER_ENQUEUE |
            GAME_MOVE_CALLER_TRANSITION_6_1));
    assert(game_move_request_plan_4193c(1) ==
        (GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_TRANSITION_6_1));

    assert(game_move_request_plan_41b08(0x15, 1) ==
        (GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_CLEAR_SAVED_S2 |
            GAME_MOVE_CALLER_TRANSITION_6_4));
    assert(game_move_request_plan_41b08(0x14, 0) ==
        (GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_CALL_80052AEC |
            GAME_MOVE_CALLER_TRANSITION_6_4));
    assert(game_move_request_plan_41b08(0x14, 1) ==
        (GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_CALL_80052AEC |
            GAME_MOVE_CALLER_CALL_80022944 | GAME_MOVE_CALLER_TRANSITION_6_4));
}

typedef struct MoveCallerTestLog {
    uint32_t calls[12];
    uintptr_t arguments[12][8];
    size_t call_count;
    int32_t helper_22674_result;
    int32_t helper_2205C_result;
    int32_t transition_result;
    uint8_t *actor_state_byte_01;
    int mutate_actor_state_after_22674;
} MoveCallerTestLog;

static int32_t record_move_caller_invoke(void *context, uint32_t function_address, const uintptr_t arguments[8]) {
    MoveCallerTestLog *log = context;
    size_t index;

    assert(log->call_count < sizeof(log->calls) / sizeof(log->calls[0]));
    log->calls[log->call_count] = function_address;
    for (index = 0; index < 8; index++) {
        log->arguments[log->call_count][index] = arguments[index];
    }
    log->call_count++;

    if (function_address == 0x80022674) {
        if (log->mutate_actor_state_after_22674) {
            *log->actor_state_byte_01 = 1;
        }
        return log->helper_22674_result;
    }
    if (function_address == 0x8002205C) {
        return log->helper_2205C_result;
    }
    if (function_address == 0x8001B558) {
        return log->transition_result;
    }
    return 0;
}

static void test_game_run_remaining_move_request_paths(void) {
    GameMoveRequestQueue queue = {0};
    MoveCallerTestLog log = {0};
    GameMoveState12Ops ops = {&log, record_move_limit_assertion, record_move_caller_invoke, NULL};
    uint8_t actor_state_byte_01 = 1;
    int actor;
    int move_context;
    void *entries[4] = {0};

    queue.entries = entries;
    log.actor_state_byte_01 = &actor_state_byte_01;
    log.transition_result = 0x51;
    log.helper_2205C_result = 0x60;

    assert(game_run_move_request_path_3d718(
        &queue, &actor, &move_context, &actor_state_byte_01, &ops
    ) == 0);
    assert(queue.count == 1 && entries[0] == &actor);
    assert(log.call_count == 2);
    assert(log.calls[0] == 0x80022944 && log.calls[1] == 0x8001B558);
    assert(log.arguments[1][0] == 0x2F && log.arguments[1][1] == 1);

    actor_state_byte_01 = 2;
    queue.count = 0;
    log.call_count = 0;
    log.helper_22674_result = 0x101;
    assert(game_run_move_request_path_3d718(
        &queue, &actor, &move_context, &actor_state_byte_01, &ops
    ) == 0x60);
    assert(queue.count == 0);
    assert(log.call_count == 5);
    assert(log.calls[0] == 0x80022944 && log.calls[1] == 0x8001FD6C);
    assert(log.calls[2] == 0x80022674 && log.calls[3] == 0x8001B558);
    assert(log.calls[4] == 0x8002205C);
    assert(log.arguments[3][0] == 0x1B && log.arguments[3][1] == 3);

    log.call_count = 0;
    log.helper_22674_result = 0;
    log.transition_result = 0x52;
    assert(game_run_move_request_path_3d718(
        &queue, &actor, &move_context, &actor_state_byte_01, &ops
    ) == 0x52);
    assert(queue.count == 1 && entries[0] == &actor);
    assert(log.call_count == 5 && log.calls[3] == 0x8001B558 && log.calls[4] == 0x8001B558);
    assert(log.arguments[3][0] == 0x2F && log.arguments[3][1] == 1);
    assert(log.arguments[4][0] == 5 && log.arguments[4][1] == 3);

    actor_state_byte_01 = 1;
    queue.count = 0;
    log.call_count = 0;
    log.helper_22674_result = 0;
    log.transition_result = 0x61;
    assert(game_run_move_request_path_4193c(
        &queue, &actor, &move_context, 0, &ops
    ) == 0x61);
    assert(queue.count == 1 && entries[0] == &actor);
    assert(log.call_count == 2 && log.calls[0] == 0x80022944 && log.calls[1] == 0x8001B558);
    assert(log.arguments[1][0] == 6 && log.arguments[1][1] == 1);

    queue.count = 0;
    log.call_count = 0;
    assert(game_run_move_request_path_41b08(
        &queue, &actor, &move_context, &move_context, 0x15, 1, &ops
    ) == 0x61);
    assert(queue.count == 1 && log.call_count == 1 && log.calls[0] == 0x8001B558);
    assert(log.arguments[0][0] == 6 && log.arguments[0][1] == 4);

    queue.count = 0;
    log.call_count = 0;
    assert(game_run_move_request_path_41b08(
        &queue, &actor, &move_context, &move_context, 0x14, 1, &ops
    ) == 0x61);
    assert(queue.count == 1 && log.call_count == 3);
    assert(log.calls[0] == 0x80052AEC && log.calls[1] == 0x80022944 && log.calls[2] == 0x8001B558);
}

static void test_game_run_move_request_path_3d5d8(void) {
    GameMoveRequestQueue queue = {0};
    MoveCallerTestLog log = {0};
    GameMoveState12Ops ops = {&log, record_move_limit_assertion, record_move_caller_invoke, NULL};
    GameMoveRequestGlobalObject3D5D8View global_object = {0, 11, INT32_MIN};
    uint8_t actor_byte_01 = 2;
    int32_t actor_word_14 = 0;
    uint8_t global_byte_3E4 = 1;
    void *global_actor_3F8 = NULL;
    int actor;
    int actor_context;
    int linked_object;
    int global_509A4_argument;
    void *entries[1] = {0};
    GameMoveRequestState3D5D8View state = {
        &actor,
        &actor_byte_01,
        &actor_word_14,
        0x1234,
        &actor_context,
        &linked_object,
        0x5678,
        &global_object,
        &global_byte_3E4,
        &global_actor_3F8,
        &global_509A4_argument,
    };

    queue.entries = entries;
    log.transition_result = 0x77;
    assert(game_run_move_request_path_3d5d8(&queue, &state, 0, 0, &ops) == 0);
    assert(game_run_move_request_path_3d5d8(&queue, &state, 1, 2, &ops) == 0);
    assert(queue.count == 0 && log.call_count == 0);

    actor_byte_01 = 1;
    assert(game_run_move_request_path_3d5d8(&queue, &state, 1, 0, &ops) == 0);
    assert(queue.count == 0 && log.call_count == 0);

    actor_byte_01 = 2;
    assert(game_run_move_request_path_3d5d8(&queue, &state, 1, 1, &ops) == 0);
    assert(queue.count == 1 && entries[0] == &actor);
    assert(actor_byte_01 == 1 && actor_word_14 == 0x1234);
    assert(global_object.word_30 == 0x60 && global_object.word_88 == -11);
    assert(global_object.word_90 == INT32_MIN);
    assert(linked_object == 0x5678);
    assert(global_byte_3E4 == 0 && global_actor_3F8 == &actor);
    assert(log.call_count == 5);
    assert(log.calls[0] == 0x8001B558 && log.calls[1] == 0x8001B558);
    assert(log.arguments[0][0] == 0x26 && log.arguments[0][1] == 4);
    assert(log.arguments[0][2] == (uintptr_t)&actor_context && log.arguments[0][3] == 0);
    assert(log.arguments[1][0] == 0x2F && log.arguments[1][1] == 1);
    assert(log.arguments[1][2] == (uintptr_t)&actor_context);
    assert(log.arguments[1][3] == (uintptr_t)&actor_context);
    assert(log.calls[2] == 0x80028764 && log.arguments[2][0] == 0);
    assert(log.calls[3] == 0x80053270 && log.arguments[3][0] == 0x20);
    assert(log.calls[4] == 0x800509A4 && log.arguments[4][0] == 1);
    assert(log.arguments[4][1] == (uintptr_t)&global_509A4_argument);
    assert(log.arguments[4][2] == 1);
}

typedef struct MovePreparationTestLog {
    unsigned int notify_count;
    unsigned int component_update_count;
    uint32_t squared_sums[2];
    uint32_t calls[12];
    size_t call_count;
} MovePreparationTestLog;

static void record_move_preparation_magnitude(void *context, uint32_t squared_sum, int32_t *magnitude_out) {
    MovePreparationTestLog *log = context;

    assert(log->component_update_count < sizeof(log->squared_sums) / sizeof(log->squared_sums[0]));
    assert(log->call_count < sizeof(log->calls) / sizeof(log->calls[0]));
    log->squared_sums[log->component_update_count] = squared_sum;
    log->calls[log->call_count++] = 0x800535CC;
    log->component_update_count++;
    *magnitude_out = log->component_update_count == 2 ? 1 : 0;
}

static int32_t record_move_preparation_call(
    void *context,
    uint32_t function_address,
    uintptr_t argument_0,
    uintptr_t argument_1,
    uintptr_t argument_2,
    uintptr_t argument_3
) {
    MovePreparationTestLog *log = context;

    (void)argument_1;
    (void)argument_2;
    (void)argument_3;
    assert(log->call_count < sizeof(log->calls) / sizeof(log->calls[0]));
    log->calls[log->call_count++] = function_address;
    if (function_address == 0x80022944) {
        assert(argument_0 != 0);
        log->notify_count++;
        return 0;
    }
    if (function_address == 0x800546F4 || function_address == 0x800550A4 ||
        function_address == 0x8005541C) {
        return 0;
    }
    assert(0);
    return 0;
}

static void test_game_prepare_move_request(void) {
    GameMoveInfoView move_info = {0};
    GameMovePreparationObjectView object = {0};
    GameMovePreparationOps ops = {0};
    GameComponentVectorMagnitudeOps magnitude_ops = {0};
    MovePreparationTestLog log = {0};
    GameMoveRequestQueue queue = {0};
    GameComponentTransformView global_transform = {0};
    void *entries[1] = {0};
    int actor;
    uint8_t completion_flag = 0xFF;
    GameMoveRequestDirectoryEntry directory_entries[2] = {0};
    GameMoveRequestDirectoryView directory = {2, 2, directory_entries};

    ops.context = &log;
    ops.assertion = record_move_limit_assertion;
    ops.invoke = record_move_preparation_call;
    ops.global_transform = &global_transform;
    magnitude_ops.context = &log;
    magnitude_ops.convert_squared_sum = record_move_preparation_magnitude;
    ops.vector_magnitude_ops = &magnitude_ops;
    ops.directory = &directory;
    object.move_context = (void *)(intptr_t)1;
    object.component = &actor;
    object.move_info = &move_info;
    queue.entries = entries;

    move_info.words[0x04 / 4] = -2;
    assert(game_prepare_move_request(&queue, &actor, &object, NULL, 0, &completion_flag, &ops) == 1);
    assert(completion_flag == 0 && log.notify_count == 0);

    move_info.words[0x04 / 4] = 0;
    move_info.words[0x18 / 4] = 1;
    assert(game_prepare_move_request(&queue, &actor, &object, NULL, 0, &completion_flag, &ops) == 1);
    assert(log.notify_count == 1 && queue.count == 0);

    move_info.words[0x38 / 4] = -1;
    assert(game_prepare_move_request(&queue, &actor, &object, NULL, 0, &completion_flag, &ops) == 1);
    assert(log.notify_count == 2 && queue.count == 1 && entries[0] == &actor);

    move_info.words[0x18 / 4] = 0;
    move_info.words[0] = 0;
    move_info.words[0x04 / 4] = 0;
    directory_entries[1].table_10 = NULL;
    directory_entries[1].table_limit_14 = 0;
    object.component = NULL;
    queue.count = 0;
    entries[0] = NULL;
    assert(game_prepare_move_request(&queue, &actor, &object, NULL, 0, &completion_flag, &ops) == 0);
    assert(move_info.words[0] == 0);
    assert(queue.count == 0 && entries[0] == NULL);

    {
        static const uint32_t expected_calls[] = {
            0x800535CC, 0x800535CC, 0x800546F4,
            0x800550A4, 0x8005541C, 0x80022944,
        };
        static const int32_t movement_payload[8] = {2, -3, 4, 0xAA, 5, 6, 7, 0xBB};
        static const GameMovePreparationTableEntry movement_table[] = {
            {0x0A, 1, 0, 2, movement_payload},
        };
        GameComponentTransformView component = {0};
        int basis_marker;
        size_t call_index;

        memset(&move_info, 0, sizeof(move_info));
        memset(&log, 0, sizeof(log));
        move_info.words[0x38 / 4] = 1;
        directory_entries[1] = (GameMoveRequestDirectoryEntry){1, movement_table, 1};
        object.component = &component;
        object.component_basis = &basis_marker;
        queue.count = 0;
        entries[0] = NULL;

        assert(game_prepare_move_request(&queue, &actor, &object, &basis_marker, 0, &completion_flag, &ops) == 1);
        assert(completion_flag == 0);
        assert(move_info.words[0] == 1 && move_info.words[1] == 0);
        assert(((uint8_t *)move_info.words)[0x1C] == 1);
        assert(move_info.words[2] == 7 && move_info.words[3] == 3 && move_info.words[4] == 11);
        assert(global_transform.translation[0] == 7);
        assert(global_transform.translation[1] == -3);
        assert(global_transform.translation[2] == 11);
        assert(move_info.words[0x30 / 4] == 2);
        assert(log.component_update_count == 2);
        assert(log.squared_sums[0] == 29 && log.squared_sums[1] == 179);
        assert(log.call_count == sizeof(expected_calls) / sizeof(expected_calls[0]));
        for (call_index = 0; call_index < log.call_count; call_index++) {
            assert(log.calls[call_index] == expected_calls[call_index]);
        }
        assert(queue.count == 1 && entries[0] == &actor);
    }
}

static void test_move_request_directory_helpers(void) {
    static const GameMovePreparationTableEntry table[] = {{0x0A, 3, 0, 12, NULL}};
    const GameMoveRequestDirectoryEntry entries[] = {
        {2, NULL, 0},
        {7, table, 1},
    };
    const GameMoveRequestDirectoryView directory = {2, 2, entries};
    const GameMovePreparationTableEntry *table_out = NULL;
    uint32_t table_limit = 99;
    uint8_t selector = 0;

    assert(game_move_request_lookup_table(1, &directory, &table_out, &table_limit) == 1);
    assert(table_out == table && table_limit == 1);
    assert(game_move_request_read_selector(1, &directory, &selector) == 1);
    assert(selector == 7);

    table_out = NULL;
    table_limit = 99;
    assert(game_move_request_lookup_table(-1, &directory, &table_out, &table_limit) == 0);
    assert(table_out == NULL && table_limit == 99);
    assert(game_move_request_lookup_table(2, &directory, &table_out, &table_limit) == 0);
    assert(game_move_request_read_selector(-1, &directory, &selector) == 0);
    assert(game_move_request_read_selector(2, &directory, &selector) == 0);
}

typedef struct MoveState12TestLog {
    uint32_t calls[8];
    size_t call_count;
    uint32_t *actor_state_10;
    int clear_actor_state_after_22c94;
    unsigned int continuation_count;
} MoveState12TestLog;

static int32_t record_move_state12_call(void *context, uint32_t function_address, const uintptr_t arguments[8]) {
    MoveState12TestLog *log = context;

    assert(log->call_count < sizeof(log->calls) / sizeof(log->calls[0]));
    log->calls[log->call_count++] = function_address;
    if (function_address == 0x80052AEC || function_address == 0x80052E18) {
        assert(arguments[0] != 0);
        return 0;
    }
    if (function_address == 0x800691B8) {
        assert(arguments[0] != 0 && arguments[1] == 0);
        return 0;
    }
    if (function_address == 0x80022C94) {
        assert(arguments[0] != 0);
        if (arguments[1] == 0x380) {
            assert(arguments[4] == 0x5A && arguments[5] == 0x61 && arguments[6] == 0x68);
        } else {
            assert(arguments[1] == 0 && arguments[4] == 0x5A);
            if (log->clear_actor_state_after_22c94) {
                *log->actor_state_10 = 0;
            }
        }
        return 0;
    }
    if (function_address == 0x8001B558) {
        assert(arguments[0] == 0x0C && arguments[1] == 4);
        assert(arguments[2] != 0 && arguments[2] == arguments[3]);
        return 0x42;
    }
    assert(0);
    return 0;
}

static int32_t record_move_state12_continuation(void *context, GameMoveState12View *state) {
    MoveState12TestLog *log = context;
    assert(state->actor != NULL);
    log->continuation_count++;
    return 0x66;
}

static void test_game_process_move_request_state_12(void) {
    int actor;
    int move_context;
    int shared_context;
    uint32_t actor_state_10 = 0;
    uint8_t global_byte_14 = 1;
    void *entries[2] = {0};
    GameMoveRequestQueue queue = {0, entries};
    MoveState12TestLog log = {0};
    GameMoveState12View state = {
        &actor, &move_context, &actor_state_10, 1, &global_byte_14, &shared_context,
    };
    GameMoveState12Ops ops = {
        &log, record_move_limit_assertion, record_move_state12_call,
        record_move_state12_continuation,
    };

    log.actor_state_10 = &actor_state_10;
    assert(game_process_move_request_state_12(&queue, &state, &ops) == 0x42);
    assert(global_byte_14 == 0 && queue.count == 1 && entries[0] == &actor);
    assert(log.call_count == 4);
    assert(log.calls[0] == 0x80052AEC && log.calls[1] == 0x80052E18);
    assert(log.calls[2] == 0x80022C94 && log.calls[3] == 0x8001B558);

    actor_state_10 = 4;
    log.call_count = 0;
    assert(game_process_move_request_state_12(&queue, &state, &ops) == 0x42);
    assert(queue.count == 2 && entries[1] == &actor);
    assert(log.calls[0] == 0x800691B8 && log.calls[1] == 0x80022C94);
    assert(log.calls[2] == 0x8001B558);

    actor_state_10 = 4;
    log.clear_actor_state_after_22c94 = 1;
    log.call_count = 0;
    assert(game_process_move_request_state_12(&queue, &state, &ops) == 0x66);
    assert(queue.count == 2 && log.continuation_count == 1);
    assert(log.call_count == 2 && log.calls[0] == 0x800691B8 && log.calls[1] == 0x80022C94);
}

enum {
    MOVE_CONSUMER_CALL_548A0 = 1,
    MOVE_CONSUMER_CALL_536F4 = 2,
    MOVE_CONSUMER_CALL_10038 = 3,
    MOVE_CONSUMER_CALL_54B20 = 4,
    MOVE_CONSUMER_CALL_54C7C = 5,
};

typedef struct GameMoveConsumerTestLog {
    GameMoveInfoView *move_info;
    void *component_transform;
    const int32_t *rotation_scratch;
    int32_t calls[8];
    size_t call_count;
    unsigned int vector_combine_count;
    unsigned int transform_apply_count;
} GameMoveConsumerTestLog;

static unsigned int move_info_assertion_count;

static void log_move_consumer_call(GameMoveConsumerTestLog *log, int32_t call) {
    assert(log->call_count < sizeof(log->calls) / sizeof(log->calls[0]));
    log->calls[log->call_count++] = call;
}

static void record_move_consumer_assertion(
    int32_t condition_value,
    const char *condition,
    const char *source_path,
    uint32_t source_line
) {
    assert(condition_value == 0);
    assert(strcmp(condition, "mi") == 0);
    assert(strcmp(source_path, "../f/game.c") == 0);
    assert(source_line == 0xB12);
    move_info_assertion_count++;
}

static void record_move_consumer_rotate(
    void *context,
    void *component_transform,
    const int32_t *values_48
) {
    GameMoveConsumerTestLog *log = context;

    assert(component_transform == log->component_transform);
    assert(values_48 == log->move_info->words + 0x48 / 4);
    log_move_consumer_call(log, MOVE_CONSUMER_CALL_548A0);
}

static void record_move_consumer_prepare(
    void *context,
    const int32_t *values_98,
    int32_t *scratch
) {
    GameMoveConsumerTestLog *log = context;

    assert(values_98 == log->move_info->words + 0x98 / 4);
    log->rotation_scratch = scratch;
    scratch[0] = 0xA0;
    scratch[1] = 0xA1;
    scratch[2] = 0xA2;
    scratch[3] = 0xA3;
    log_move_consumer_call(log, MOVE_CONSUMER_CALL_536F4);
}

static void record_move_consumer_combine(
    void *context,
    const int32_t *a0,
    const int32_t *a1,
    int32_t *a2
) {
    GameMoveConsumerTestLog *log = context;

    log->vector_combine_count++;
    log_move_consumer_call(log, MOVE_CONSUMER_CALL_10038);
    if (log->vector_combine_count == 1) {
        assert(a0 == log->move_info->words + 0x38 / 4);
        assert(a1 == log->rotation_scratch);
        assert(a2 == a0);
        a2[0] = 5;
        a2[1] = 6;
        a2[2] = 7;
    } else {
        assert(a0 == log->move_info->words + 0x68 / 4);
        assert(a1 == log->move_info->words + 0x38 / 4);
        assert(a2 != a0 && a2 != a1);
        a2[0] = 0xB0;
        a2[1] = 0xB1;
        a2[2] = 0xB2;
        a2[3] = 0xB3;
    }
}

static void record_move_consumer_update(
    void *context,
    void *component_transform,
    int32_t mode,
    const int32_t *values_68
) {
    GameMoveConsumerTestLog *log = context;

    assert(component_transform == log->component_transform);
    assert(mode == 0);
    assert(values_68 == log->move_info->words + 0x68 / 4);
    log_move_consumer_call(log, MOVE_CONSUMER_CALL_54B20);
}

static void record_move_consumer_apply(
    void *context,
    void *component_transform,
    int32_t mode,
    const int32_t *translation
) {
    GameMoveConsumerTestLog *log = context;

    assert(component_transform == log->component_transform);
    assert(mode == 0);
    assert(translation[0] == 0xB0 && translation[1] == 0xB1);
    assert(translation[2] == 0xB2 && translation[3] == 0xB3);
    log->transform_apply_count++;
    log_move_consumer_call(log, MOVE_CONSUMER_CALL_54C7C);
}

static void test_game_apply_queued_move_request(void) {
    GameMoveInfoView move_info = {0};
    int transform_marker;
    GameMoveConsumerTestLog log = {0};
    GameMoveConsumerObjectView object = {&transform_marker, &move_info};
    GameMoveConsumerOps ops = {
        &log,
        record_move_consumer_assertion,
        record_move_consumer_rotate,
        record_move_consumer_prepare,
        record_move_consumer_combine,
        record_move_consumer_update,
        record_move_consumer_apply,
    };
    static const int32_t expected_calls[] = {
        MOVE_CONSUMER_CALL_548A0,
        MOVE_CONSUMER_CALL_536F4,
        MOVE_CONSUMER_CALL_10038,
        MOVE_CONSUMER_CALL_54B20,
        MOVE_CONSUMER_CALL_10038,
        MOVE_CONSUMER_CALL_54C7C,
    };
    size_t index;

    log.move_info = &move_info;
    log.component_transform = &transform_marker;
    move_info.words[0x38 / 4] = 3;
    move_info.words[0x44 / 4] = 0x44;
    move_info.words[0x48 / 4] = 1;
    move_info.words[0x68 / 4] = 2;
    move_info.words[0x98 / 4] = 4;

    game_apply_queued_move_request(&object, &ops);

    assert(log.call_count == sizeof(expected_calls) / sizeof(expected_calls[0]));
    for (index = 0; index < log.call_count; index++) {
        assert(log.calls[index] == expected_calls[index]);
    }
    assert(log.transform_apply_count == 1 && log.vector_combine_count == 2);
    assert(move_info.words[0x78 / 4] == 5);
    assert(move_info.words[0x7C / 4] == 6);
    assert(move_info.words[0x80 / 4] == 7);
    assert(move_info.words[0x84 / 4] == 0x44);
    assert(move_info.words[0x38 / 4] == 0 && move_info.words[0x3C / 4] == 0);
    assert(move_info.words[0x40 / 4] == 0 && move_info.words[0x44 / 4] == 0x44);
    assert(move_info.words[0x48 / 4] == 0 && move_info.words[0x4C / 4] == 0);
    assert(move_info.words[0x50 / 4] == 0 && move_info.words[0x98 / 4] == 0);

    {
        GameMoveInfoView empty_move_info = {0};

        object.move_info = &empty_move_info;
        log.move_info = &empty_move_info;
        log.call_count = 0;
        log.vector_combine_count = 0;
        log.transform_apply_count = 0;
        game_apply_queued_move_request(&object, &ops);
        assert(log.call_count == 1);
        assert(log.calls[0] == MOVE_CONSUMER_CALL_54B20);
        assert(log.vector_combine_count == 0 && log.transform_apply_count == 0);
    }

    object.move_info = NULL;
    log.call_count = 0;
    log.vector_combine_count = 0;
    log.transform_apply_count = 0;
    move_info_assertion_count = 0;
    game_apply_queued_move_request(&object, &ops);
    assert(log.call_count == 0 && log.vector_combine_count == 0);
    assert(move_info_assertion_count == 1);
}

typedef struct ComponentTransformTestLog {
    unsigned int call_count;
} ComponentTransformTestLog;

static void record_component_point_transform(
    void *context,
    const GameComponentBasisView *basis,
    const int32_t input[3],
    int32_t output[3]
) {
    ComponentTransformTestLog *log = context;

    assert(basis->matrix_halfwords[0] == 1);
    assert(basis->matrix_halfwords[1] == (uint16_t)-2);
    assert(basis->matrix_halfwords[2] == 3);
    assert(basis->matrix_halfwords[3] == (uint16_t)-4);
    assert(basis->translation[0] == 0x10);
    assert(basis->translation[1] == 0x20);
    assert(basis->translation[2] == 0x30);
    assert(input[0] == 8 && input[1] == -9 && input[2] == 10);
    output[0] = 0x111;
    output[1] = 0x222;
    output[2] = -0x333;
    log->call_count++;
}

static void test_component_transform_set_translation(void) {
    GameComponentTransformView component = {0};
    GameComponentBasisView global_basis = {0};
    GameComponentBasisView basis;
    int32_t translation[3] = {13, INT32_MIN, -27};
    ComponentTransformTestLog log = {0};
    GameComponentTransformOps ops = {&log, record_component_point_transform, NULL};

    component.matrix_halfwords[0] = 0xA;
    component.matrix_halfwords[8] = 0xB;
    component.unused_halfword_12 = 0xC;
    component.translation[0] = 0xD;
    component.translation[1] = 0xE;
    component.translation[2] = 0xF;
    assert(game_component_transform_set_translation(&component, NULL, NULL, translation, NULL) == 0);
    assert(component.matrix_halfwords[0] == 0xA && component.matrix_halfwords[8] == 0xB);
    assert(component.unused_halfword_12 == 0xC);
    assert(component.translation[0] == 13 && component.translation[1] == INT32_MIN && component.translation[2] == -27);

    basis = global_basis;
    basis.ignored_halfword_12 = 0x1234;
    translation[0] = 4;
    translation[1] = -5;
    translation[2] = 6;
    assert(game_component_transform_set_translation(&component, &basis, &global_basis, translation, NULL) == 0);
    assert(component.translation[0] == 4 && component.translation[1] == 5 && component.translation[2] == 6);

    basis.matrix_halfwords[0] = 1;
    basis.matrix_halfwords[1] = 2;
    basis.matrix_halfwords[2] = 3;
    basis.matrix_halfwords[3] = 4;
    basis.translation[0] = 0x10;
    basis.translation[1] = -0x20;
    basis.translation[2] = 0x30;
    translation[0] = 8;
    translation[1] = -9;
    translation[2] = 10;
    assert(game_component_transform_set_translation(&component, &basis, &global_basis, translation, &ops) == 0);
    assert(log.call_count == 1);
    assert(component.translation[0] == 0x111 && component.translation[1] == -0x222 && component.translation[2] == -0x333);

    assert(game_component_transform_set_translation(NULL, NULL, NULL, NULL, NULL) == 0x18);
}

static void record_component_point_to_basis(
    void *context,
    const GameComponentBasisView *basis,
    const int32_t input[3],
    int32_t output[3]
) {
    ComponentTransformTestLog *log = context;

    assert(basis->matrix_halfwords[1] == (uint16_t)-2);
    assert(basis->matrix_halfwords[3] == (uint16_t)-4);
    assert(basis->translation[0] == 0x10);
    assert(basis->translation[1] == 0x20);
    assert(basis->translation[2] == 0x30);
    assert(input[0] == 100 && input[1] == -200 && input[2] == 300);
    output[0] = -11;
    output[1] = 22;
    output[2] = -33;
    log->call_count++;
}

static void test_component_transform_read_translation(void) {
    GameComponentTransformView component = {0};
    GameComponentBasisView global_basis = {0};
    GameComponentBasisView basis;
    ComponentTransformTestLog log = {0};
    GameComponentTransformOps ops = {&log, NULL, record_component_point_to_basis};
    int32_t output[3] = {0};

    component.translation[0] = 100;
    component.translation[1] = 200;
    component.translation[2] = 300;
    assert(game_component_transform_read_translation(&component, NULL, NULL, output, NULL) == 0);
    assert(output[0] == 100 && output[1] == -200 && output[2] == 300);

    basis = global_basis;
    basis.ignored_halfword_12 = 0x1234;
    assert(game_component_transform_read_translation(&component, &basis, &global_basis, output, NULL) == 0);
    assert(output[0] == 100 && output[1] == -200 && output[2] == 300);

    basis.matrix_halfwords[0] = 1;
    basis.matrix_halfwords[1] = 2;
    basis.matrix_halfwords[2] = 3;
    basis.matrix_halfwords[3] = 4;
    basis.translation[0] = 0x10;
    basis.translation[1] = -0x20;
    basis.translation[2] = 0x30;
    assert(game_component_transform_read_translation(&component, &basis, &global_basis, output, &ops) == 0);
    assert(log.call_count == 1);
    assert(output[0] == -11 && output[1] == 22 && output[2] == -33);

    assert(game_component_transform_read_translation(NULL, NULL, NULL, NULL, NULL) == 0x18);
}

typedef struct ComponentMagnitudeTestLog {
    uint32_t squared_sum;
    unsigned int call_count;
} ComponentMagnitudeTestLog;

static void record_component_magnitude(
    void *context,
    uint32_t squared_sum,
    int32_t *magnitude_out
) {
    ComponentMagnitudeTestLog *log = context;

    log->squared_sum = squared_sum;
    log->call_count++;
    *magnitude_out = 15;
}

static void test_component_transform_helper_models(void) {
    GameComponentTransformView component = {0};
    int32_t matrix_vector[3] = {0};
    int32_t left[3] = {10, -4, 8};
    int32_t right[3] = {1, 2, -2};
    int32_t magnitude = -1;
    ComponentMagnitudeTestLog log = {0};
    GameComponentVectorMagnitudeOps ops = {&log, record_component_magnitude};

    component.matrix_halfwords[2] = (uint16_t)-12;
    component.matrix_halfwords[5] = (uint16_t)-7;
    component.matrix_halfwords[8] = 31;
    assert(game_component_transform_read_matrix_vector(&component, matrix_vector) == 0);
    assert(matrix_vector[0] == -12 && matrix_vector[1] == 7 && matrix_vector[2] == 31);

    assert(game_component_vector_separation(left, right, &magnitude, &ops) == 0);
    assert(log.call_count == 1 && log.squared_sum == 217);
    assert(magnitude == 15);
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

static void test_bubsy_actor_shared_update_entry(void) {
    assert(bubsy_actor_shared_update_entry_action(0x4, 0) ==
        BUBSY_ACTOR_SHARED_UPDATE_ENTRY_RETURN);
    assert(bubsy_actor_shared_update_entry_action(0, 1) ==
        BUBSY_ACTOR_SHARED_UPDATE_ENTRY_RETURN);
    assert(bubsy_actor_shared_update_entry_action(0x100, 0) ==
        BUBSY_ACTOR_SHARED_UPDATE_ENTRY_SET_PROPERTIES_2_6);
    assert(bubsy_actor_shared_update_entry_action(0x1000, 0) ==
        BUBSY_ACTOR_SHARED_UPDATE_ENTRY_SET_PROPERTIES_2_6);
    assert(bubsy_actor_shared_update_entry_action(0, 0) ==
        BUBSY_ACTOR_SHARED_UPDATE_ENTRY_CONTINUE);
}

static void test_bubsy_actor_347ec_mode_entry(void) {
    uint8_t actor_state_00 = 0xA5;

    assert(bubsy_actor_347ec_select_mode_entry(0, 0, &actor_state_00) ==
        BUBSY_ACTOR_347EC_MODE_ZERO_PATH);
    assert(actor_state_00 == 0xA5);
    assert(bubsy_actor_347ec_select_mode_entry(1, 0x44, &actor_state_00) ==
        BUBSY_ACTOR_347EC_NONZERO_ALTERNATE_PATH);
    assert(actor_state_00 == 0xA5);
    assert(bubsy_actor_347ec_select_mode_entry(1, 0, &actor_state_00) ==
        BUBSY_ACTOR_347EC_CALL_2269C_01);
    assert(actor_state_00 == 0);
}

static void test_bubsy_actor_347ec_argument_plan(void) {
    BubsyActor347ECArgumentPlan plan = {0};

    bubsy_actor_347ec_build_nonzero_mode_plan(0x9000, 1, 0x5A, &plan);
    assert((plan.actions & (BUBSY_ACTOR_347EC_NONZERO_CALL_22C94 |
        BUBSY_ACTOR_347EC_NONZERO_CLEAR_ACTOR_00 |
        BUBSY_ACTOR_347EC_NONZERO_CALL_2269C_01 |
        BUBSY_ACTOR_347EC_NONZERO_CALL_54B20 |
        BUBSY_ACTOR_347EC_NONZERO_CALL_1B558_27_04)) ==
        (BUBSY_ACTOR_347EC_NONZERO_CALL_22C94 |
            BUBSY_ACTOR_347EC_NONZERO_CLEAR_ACTOR_00 |
            BUBSY_ACTOR_347EC_NONZERO_CALL_2269C_01 |
            BUBSY_ACTOR_347EC_NONZERO_CALL_54B20 |
            BUBSY_ACTOR_347EC_NONZERO_CALL_1B558_27_04));
    assert(plan.argument_a1 == 0x29D && plan.stack_argument_10 == 0x2E3);
    assert(plan.stack_argument_14 == 0x2EA && plan.stack_argument_18 == 0x2EF);
    assert(plan.argument_a2 == 1 && plan.argument_a3 == 0);
    assert(plan.set_global_37D == 0 && plan.update_properties_3_4 == 0 && plan.actor_state_44_path == 0);

    bubsy_actor_347ec_build_nonzero_mode_plan(0x10018, 0, 0x5A, &plan);
    assert((plan.actions & BUBSY_ACTOR_347EC_NONZERO_PROPERTIES_3_4) != 0);
    assert(plan.argument_a1 == 0x29D && plan.stack_argument_10 == 0x2F8);
    assert(plan.stack_argument_14 == 0x2FF && plan.stack_argument_18 == 0x302);
    assert(plan.set_global_37D == 1 && plan.update_properties_3_4 == 1 && plan.actor_state_44_path == 0);

    bubsy_actor_347ec_build_nonzero_mode_plan(0, 0, 0x5A, &plan);
    assert(plan.argument_a1 == 0x7D8 && plan.stack_argument_10 == 0x29B);
    assert(plan.stack_argument_14 == 0x2A0 && plan.stack_argument_18 == 0x2A7);
    assert(plan.set_global_37D == 0 && plan.update_properties_3_4 == 0 && plan.actor_state_44_path == 0);
    bubsy_actor_347ec_build_nonzero_mode_plan(0, 1, 0x5A, &plan);
    assert(plan.stack_argument_14 == 0x33A);

    bubsy_actor_347ec_build_nonzero_mode_plan(0x18, 0, 0x5A, &plan);
    assert(plan.update_properties_3_4 == 1);
    assert(plan.stack_argument_10 == 0x294);

    bubsy_actor_347ec_build_nonzero_mode_plan(0x44, 0, 7, &plan);
    assert(plan.actor_state_44_path == 1 && plan.argument_a1 == 0);
    assert(plan.argument_a2 == 0 && plan.argument_a3 == 7);
    assert(plan.actions == BUBSY_ACTOR_347EC_NONZERO_CALL_22C94);
    assert(plan.stack_argument_10 == 0 && plan.stack_argument_14 == 0x33A);
    assert(plan.stack_argument_18 == 0x2A7 && plan.update_properties_3_4 == 0);
    bubsy_actor_347ec_build_nonzero_mode_plan(0x9044, 0, 3, &plan);
    assert(plan.stack_argument_14 == 0x2EA && plan.stack_argument_18 == 0x2EF);
    bubsy_actor_347ec_build_nonzero_mode_plan(0x10044, 0, 4, &plan);
    assert(plan.stack_argument_14 == 0x2FF && plan.stack_argument_18 == 0x302);
}

static void test_bubsy_actor_347ec_mode_zero(void) {
    uint8_t actor_state_00 = 0xA5;
    uint8_t expected = BUBSY_ACTOR_347EC_MODE_ZERO_SET_ACTOR_STATE_00 |
        BUBSY_ACTOR_347EC_MODE_ZERO_CALL_22C94 |
        BUBSY_ACTOR_347EC_MODE_ZERO_REQUEST_MOVE |
        BUBSY_ACTOR_347EC_MODE_ZERO_CALL_2925C |
        BUBSY_ACTOR_347EC_MODE_ZERO_CALL_26A0C;

    assert(bubsy_actor_347ec_select_mode_zero_actions(0, &actor_state_00) == expected);
    assert(actor_state_00 == 1);
    actor_state_00 = 0xA5;
    assert(bubsy_actor_347ec_select_mode_zero_actions(0x18, &actor_state_00) ==
        (expected | BUBSY_ACTOR_347EC_MODE_ZERO_PROPERTIES_3_4));
    assert(actor_state_00 == 1);
    actor_state_00 = 0xA5;
    assert(bubsy_actor_347ec_select_mode_zero_actions(0x4, &actor_state_00) ==
        BUBSY_ACTOR_347EC_MODE_ZERO_SET_ACTOR_00_AND_RETURN);
    assert(actor_state_00 == 1);
    actor_state_00 = 0xA5;
    assert(bubsy_actor_347ec_select_mode_zero_actions(0x40, &actor_state_00) ==
        BUBSY_ACTOR_347EC_MODE_ZERO_ALTERNATE);
    assert(actor_state_00 == 0xA5);
}

static void test_bubsy_actor_347ec_mode_zero_call_plan(void) {
    BubsyActor347ECModeZeroPlan plan = bubsy_actor_347ec_build_mode_zero_plan(0);

    assert(plan.actions == (BUBSY_ACTOR_347EC_MODE_ZERO_SET_ACTOR_STATE_00 |
        BUBSY_ACTOR_347EC_MODE_ZERO_CALL_22C94 |
        BUBSY_ACTOR_347EC_MODE_ZERO_REQUEST_MOVE |
        BUBSY_ACTOR_347EC_MODE_ZERO_CALL_2925C |
        BUBSY_ACTOR_347EC_MODE_ZERO_CALL_26A0C));
    assert(plan.actor_state_00 == 1);
    assert(plan.selection_a1 == 0x680 && plan.selection_a2 == 1 && plan.selection_a3 == 0);
    assert(plan.selection_stack_10 == 0x29B && plan.selection_stack_14 == 0x33A);
    assert(plan.selection_stack_18 == 0x2A7);
    assert(plan.move_a0 == 0x27 && plan.move_a1 == 4);
    assert(plan.property_a0 == 4 && plan.property_a1 == 3);
    assert(plan.property_a2 == 1 && plan.property_a3 == -1);

    plan = bubsy_actor_347ec_build_mode_zero_plan(0x18);
    assert((plan.actions & BUBSY_ACTOR_347EC_MODE_ZERO_PROPERTIES_3_4) != 0);
}

static void test_bubsy_actor_347ec_mode_zero_bit40_plan(void) {
    BubsyActor347ECModeZeroBit40Plan plan =
        bubsy_actor_347ec_build_mode_zero_bit40_plan(0, 0x2, 1, 10, 41);

    assert(plan.actions == (BUBSY_ACTOR_347EC_ZERO40_CLEAR_GLOBALS_64C0_37C_37E |
        BUBSY_ACTOR_347EC_ZERO40_CALL_PROPERTIES_5_8_A |
        BUBSY_ACTOR_347EC_ZERO40_SET_GLOBAL_380 |
        BUBSY_ACTOR_347EC_ZERO40_SET_ACTOR_STATE_01 |
        BUBSY_ACTOR_347EC_ZERO40_REQUEST_MOVE_05 |
        BUBSY_ACTOR_347EC_ZERO40_SET_GLOBAL_37C |
        BUBSY_ACTOR_347EC_ZERO40_CLEAR_ACTOR_FLAGS_03 |
        BUBSY_ACTOR_347EC_ZERO40_CLEAR_GLOBAL_6464 |
        BUBSY_ACTOR_347EC_ZERO40_SET_GLOBAL_64B8));
    assert(plan.updated_global_380 == 41 && plan.actor_state_01 == 2);
    assert(plan.actor_flags_04 == 0);

    plan = bubsy_actor_347ec_build_mode_zero_bit40_plan(0x1000, 0, 1, 10, 40);
    assert((plan.actions & (BUBSY_ACTOR_347EC_ZERO40_CALL_PROPERTY_C |
        BUBSY_ACTOR_347EC_ZERO40_CLEAR_GLOBAL_6452 |
        BUBSY_ACTOR_347EC_ZERO40_CALL_2E7FC)) ==
        (BUBSY_ACTOR_347EC_ZERO40_CALL_PROPERTY_C |
            BUBSY_ACTOR_347EC_ZERO40_CLEAR_GLOBAL_6452 |
            BUBSY_ACTOR_347EC_ZERO40_CALL_2E7FC));
    assert((plan.actions & BUBSY_ACTOR_347EC_ZERO40_CALL_3539C_MODE_1) != 0);

    plan = bubsy_actor_347ec_build_mode_zero_bit40_plan(0x18000, 0x2, 0, 10, 40);
    assert((plan.actions & BUBSY_ACTOR_347EC_ZERO40_CALL_PROPERTIES_F_10) != 0);
    assert((plan.actions & (BUBSY_ACTOR_347EC_ZERO40_CLEAR_GLOBAL_6464 |
        BUBSY_ACTOR_347EC_ZERO40_SET_GLOBAL_64B8)) == 0);

    plan = bubsy_actor_347ec_build_mode_zero_bit40_plan(0, 0x2, 0, 10, 40);
    assert((plan.actions & BUBSY_ACTOR_347EC_ZERO40_ASSERT_BOUND) != 0);
    assert((plan.actions & BUBSY_ACTOR_347EC_ZERO40_REQUEST_MOVE_05) == 0);
    assert(plan.updated_global_380 == 10 && plan.actor_flags_04 == 0x2);
    plan = bubsy_actor_347ec_build_mode_zero_bit40_plan(0, 0, 0, 10, 41);
    assert((plan.actions & BUBSY_ACTOR_347EC_ZERO40_CLEAR_GLOBAL_6464) != 0);
}

static void test_bubsy_actor_347ec_mode_zero_alternate(void) {
    BubsyActor347ECModeZeroAlternatePlan plan =
        bubsy_actor_347ec_plan_mode_zero_alternate(0, 0, 0, 0xA5, 0, 0, 10, 40);

    assert(plan.actions == (BUBSY_ACTOR_347EC_ZERO_ALT_CALL_22C94 |
        BUBSY_ACTOR_347EC_ZERO_ALT_RESET_GLOBALS |
        BUBSY_ACTOR_347EC_ZERO_ALT_CALL_2944C));
    assert(plan.argument_a1 == 0x88 && plan.argument_a2 == 0 && plan.argument_a3 == 0);
    assert(plan.stack_argument_10 == 0 && plan.stack_argument_14 == 0x33A);
    assert(plan.stack_argument_18 == 0x2A7 && plan.updated_global_380 == 10);
    assert(plan.actor_state_00 == 0xA5 && plan.actor_state_01 == 2);

    plan = bubsy_actor_347ec_plan_mode_zero_alternate(0, 0, 1, 0xA5, 0, 0, 10, 40);
    assert(plan.actions == BUBSY_ACTOR_347EC_ZERO_ALT_CALL_22C94);
    assert(plan.actor_state_00 == 0xA5 && plan.updated_global_380 == 10);

    plan = bubsy_actor_347ec_plan_mode_zero_alternate(1, -1, 2, 0xA5, 0, 0, 10, 40);
    assert(plan.argument_a1 == 0x108);
    assert((plan.actions & (BUBSY_ACTOR_347EC_ZERO_ALT_CLEAR_64BC |
        BUBSY_ACTOR_347EC_ZERO_ALT_RESET_GLOBALS |
        BUBSY_ACTOR_347EC_ZERO_ALT_CALL_2944C)) ==
        (BUBSY_ACTOR_347EC_ZERO_ALT_CLEAR_64BC |
            BUBSY_ACTOR_347EC_ZERO_ALT_RESET_GLOBALS |
            BUBSY_ACTOR_347EC_ZERO_ALT_CALL_2944C));
    assert(plan.actor_state_01 == 2 && plan.updated_global_380 == 10);

    plan = bubsy_actor_347ec_plan_mode_zero_alternate(1, 0, 2, 0xA5, 1, 0x2, 10, 40);
    assert(plan.argument_a1 == 0xC8);
    assert((plan.actions & BUBSY_ACTOR_347EC_ZERO_ALT_ASSERT) != 0);
    assert((plan.actions & BUBSY_ACTOR_347EC_ZERO_ALT_CALL_1B558_27_04) == 0);
    assert(plan.actor_state_01 == 1 && plan.updated_global_380 == 10);

    plan = bubsy_actor_347ec_plan_mode_zero_alternate(0, 0, 2, 0xA5, 1, 0x2, 10, 41);
    assert((plan.actions & (BUBSY_ACTOR_347EC_ZERO_ALT_CALL_1B558_27_04 |
        BUBSY_ACTOR_347EC_ZERO_ALT_CALL_2269C_01)) ==
        (BUBSY_ACTOR_347EC_ZERO_ALT_CALL_1B558_27_04 |
            BUBSY_ACTOR_347EC_ZERO_ALT_CALL_2269C_01));
    assert(plan.actor_state_01 == 2 && plan.updated_global_380 == 41);

    plan = bubsy_actor_347ec_plan_mode_zero_alternate(0, 0, 2, 0xA5, 0, 0, 10, 41);
    assert(plan.actor_state_01 == 2 && plan.updated_global_380 == 10);
    assert((plan.actions & BUBSY_ACTOR_347EC_ZERO_ALT_ASSERT) == 0);
    assert(plan.actor_state_00 == 0xA5);
}

static void test_bubsy_actor_347ec_followup_plan(void) {
    BubsyActor347ECFollowupPlan plan = {0};

    bubsy_actor_347ec_build_followup_plan(0, 0, 5, 1, 0xFF, &plan);
    assert(plan.actions == BUBSY_ACTOR_347EC_FOLLOWUP_ENTER_STATE_BIT4_CLEAR_BLOCK);

    bubsy_actor_347ec_build_followup_plan(0x8004, 0x2EF, 5, 1, 0, &plan);
    assert(plan.actions == BUBSY_ACTOR_347EC_FOLLOWUP_MOVE_REQUEST);
    assert(plan.dispatch_mode == 0);
    bubsy_actor_347ec_build_followup_plan(0x8004, 0x2EF, 5, 0, 0, &plan);
    assert(plan.actions == BUBSY_ACTOR_347EC_FOLLOWUP_MOVE_REQUEST);
    bubsy_actor_347ec_build_followup_plan(0x8004, 0x2EE, 2, 0, 0, &plan);
    assert((plan.actions & BUBSY_ACTOR_347EC_FOLLOWUP_DISPATCH_SHARED) != 0);
    assert(plan.dispatch_mode == 2);
    bubsy_actor_347ec_build_followup_plan(0x8004, 0x2EE, 0, 0, 0, &plan);
    assert(plan.actions == BUBSY_ACTOR_347EC_FOLLOWUP_MOVE_REQUEST);

    bubsy_actor_347ec_build_followup_plan(0x10004, 0x2FF, 0, 1, 0, &plan);
    assert((plan.actions & (BUBSY_ACTOR_347EC_FOLLOWUP_DISPATCH_SHARED |
        BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_GLOBAL_37D)) ==
        (BUBSY_ACTOR_347EC_FOLLOWUP_DISPATCH_SHARED | BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_GLOBAL_37D));
    assert(plan.dispatch_mode == 1);
    bubsy_actor_347ec_build_followup_plan(0x10004, 0x2EA, 0, 1, 0, &plan);
    assert(plan.actions == BUBSY_ACTOR_347EC_FOLLOWUP_MOVE_REQUEST);
    bubsy_actor_347ec_build_followup_plan(0x10004, 0x2FF, 0, 0, 0, &plan);
    assert(plan.actions == BUBSY_ACTOR_347EC_FOLLOWUP_MOVE_REQUEST);
    bubsy_actor_347ec_build_followup_plan(0x1004, 0x2EA, 0, 0, 0, &plan);
    assert((plan.actions & (BUBSY_ACTOR_347EC_FOLLOWUP_TRANSFORM_CONTEXT |
        BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_ACTOR_1000 |
        BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_GLOBAL_6452)) ==
        (BUBSY_ACTOR_347EC_FOLLOWUP_TRANSFORM_CONTEXT |
            BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_ACTOR_1000 |
            BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_GLOBAL_6452));

    bubsy_actor_347ec_build_followup_plan(0x104, 0, 0, 0, 0xFD, &plan);
    assert(plan.actions == BUBSY_ACTOR_347EC_FOLLOWUP_MOVE_REQUEST);
    bubsy_actor_347ec_build_followup_plan(0x104, 0, 0, 0, 0xFE, &plan);
    assert((plan.actions & (BUBSY_ACTOR_347EC_FOLLOWUP_UPDATE_LINKED_4C |
        BUBSY_ACTOR_347EC_FOLLOWUP_RESET_GLOBAL_64BA |
        BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_ACTOR_100)) ==
        (BUBSY_ACTOR_347EC_FOLLOWUP_UPDATE_LINKED_4C |
            BUBSY_ACTOR_347EC_FOLLOWUP_RESET_GLOBAL_64BA |
            BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_ACTOR_100));
    assert(plan.linked_value_4C == -0x400);
    bubsy_actor_347ec_build_followup_plan(0x104, 0, 0, 0, 0xFF, &plan);
    assert(plan.linked_value_4C == 0x400);
}

static void test_bubsy_actor_347ec_state_bit4_clear_plan(void) {
    BubsyActor347ECStateBit4ClearPlan plan;

    bubsy_actor_347ec_build_state_bit4_clear_plan(0x40, 1, 0, 0, 10, 40, &plan);
    assert(plan.actions == (BUBSY_ACTOR_347EC_STATE4_CLEAR_SELECT_2B0 |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_ACTOR_00 |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_SET_ACTOR_FLAG_04 |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_CALL_1C6BC));
    assert(plan.actor_state_00 == 0 && plan.actor_flags_04 == 0x04);

    bubsy_actor_347ec_build_state_bit4_clear_plan(0, 0, 0, 0x02, 10, 41, &plan);
    assert((plan.actions & (BUBSY_ACTOR_347EC_STATE4_CLEAR_PROPERTIES_5_8_A |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_UPDATE_COUNTER |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_MOVE_05 |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_SET_GLOBAL_37C |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_ACTOR_FLAGS_03 |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_RESET_6464_64B8)) ==
        (BUBSY_ACTOR_347EC_STATE4_CLEAR_PROPERTIES_5_8_A |
            BUBSY_ACTOR_347EC_STATE4_CLEAR_UPDATE_COUNTER |
            BUBSY_ACTOR_347EC_STATE4_CLEAR_MOVE_05 |
            BUBSY_ACTOR_347EC_STATE4_CLEAR_SET_GLOBAL_37C |
            BUBSY_ACTOR_347EC_STATE4_CLEAR_ACTOR_FLAGS_03 |
            BUBSY_ACTOR_347EC_STATE4_CLEAR_RESET_6464_64B8));
    assert(plan.global_380 == 41 && plan.actor_state_01 == 2 && plan.actor_flags_04 == 0);

    bubsy_actor_347ec_build_state_bit4_clear_plan(0x1000, 0, 0, 0, 10, 40, &plan);
    assert((plan.actions & (BUBSY_ACTOR_347EC_STATE4_CLEAR_PROPERTY_C |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_GLOBAL_6452 |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_CALL_2E7FC |
        BUBSY_ACTOR_347EC_STATE4_CLEAR_UPDATE_ACTOR_MODE_1)) ==
        (BUBSY_ACTOR_347EC_STATE4_CLEAR_PROPERTY_C |
            BUBSY_ACTOR_347EC_STATE4_CLEAR_GLOBAL_6452 |
            BUBSY_ACTOR_347EC_STATE4_CLEAR_CALL_2E7FC |
            BUBSY_ACTOR_347EC_STATE4_CLEAR_UPDATE_ACTOR_MODE_1));

    bubsy_actor_347ec_build_state_bit4_clear_plan(0x18000, 0, 0, 0x02, 10, 40, &plan);
    assert((plan.actions & BUBSY_ACTOR_347EC_STATE4_CLEAR_ASSERT_BOUND) != 0);
    assert((plan.actions & BUBSY_ACTOR_347EC_STATE4_CLEAR_PROPERTIES_F_10) != 0);
    assert((plan.actions & BUBSY_ACTOR_347EC_STATE4_CLEAR_RESET_6464_64B8) == 0);
    assert(plan.global_380 == 10 && plan.actor_flags_04 == 0x02);
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

static void test_bubsy_grounded_handler_model(void) {
    BubsyGroundedHandlerModelState state = {0};
    BubsyGroundedHandlerModelPlan plan;
    uint16_t state_id;

    for (state_id = 6; state_id <= 38; state_id++) {
        state = (BubsyGroundedHandlerModelState){0};
        state.state_id = state_id;
        bubsy_grounded_handler_build_model_plan(&state, &plan);
        assert(plan.switch_dispatched == 1);
        assert(plan.target_address == bubsy_grounded_handler_case_target(state_id));
    }

    state = (BubsyGroundedHandlerModelState){0};
    state.state_id = 14;
    state.descriptor_lookup_word_04 = 3;
    state.actor_state_10 = 0x1000;
    bubsy_grounded_handler_build_model_plan(&state, &plan);
    assert(plan.state_id == 14);
    assert(plan.switch_dispatched == 0);
    assert(plan.target_address == 0x8004BFD0);
    assert(state.descriptor_state_02 == 1);

    state = (BubsyGroundedHandlerModelState){0};
    state.state_id = 37;
    bubsy_grounded_handler_build_model_plan(&state, &plan);
    assert(plan.switch_dispatched == 1);
    assert(plan.target_address == 0x8004B66C);
    assert(plan.actions_primary == BUBSY_GROUNDED_CASE37_INITIALIZE);
    assert(state.descriptor_state_11 == 1);

    state = (BubsyGroundedHandlerModelState){0};
    state.state_id = 31;
    state.descriptor_counter_4D = 11;
    state.current_sequence_key = 0x2B8;
    bubsy_grounded_handler_build_model_plan(&state, &plan);
    assert(plan.target_address == 0x8004BA34);
    assert(plan.actions_primary == BUBSY_GROUNDED_CASE31_COUNTER_SEQUENCE_PATH);
    assert((plan.actions_secondary & BUBSY_GROUNDED_CASE31_CALL_22944) != 0);
    assert(state.descriptor_counter_4D == 12 && state.linked_state_30 == -0x11);
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

    assert(vector_range_condition_q12(input, output) == 0);
    assert(output[0] == 0x1000 && output[1] == -0x2000 && output[2] == 0x3000);

    input[0] = 0x1001;
    input[1] = -2;
    input[2] = 3;
    assert(vector_range_condition_q12(input, output) == 0);
    assert(output[0] == 0x4004 && output[1] == -8 && output[2] == 12);

    input[0] = 0x40000;
    input[1] = -32;
    input[2] = 16;
    assert(vector_range_condition_q12(input, output) == 0);
    assert(output[0] == 0x100000 && output[1] == -128 && output[2] == 64);

    input[0] = 0x40001;
    assert(vector_range_condition_q12(input, output) == 0);
    assert(output[0] == 0x4000 && output[1] == -2 && output[2] == 1);

    input[0] = 0x01000001;
    input[1] = -0x1000;
    input[2] = 0x800;
    assert(vector_range_condition_q12(input, output) == 0);
    assert(output[0] == 0x2000 && output[1] == -2 && output[2] == 1);
}

static void test_vector_length_q12(void) {
    const int32_t input[3] = {3 * 0x1000, 4 * 0x1000, 12 * 0x1000};
    const int32_t sparse[3] = {3 * 0x1000, 0, 4 * 0x1000};
    const int32_t zero[3] = {0, 0, 0};
    int32_t output = 0;

    vector_length_result = 13 * 0x1000;
    vector_length_input = 0;
    vector_length_call_count = 0;
    vector_square_call_count = 0;
    assert(vector_length_q12(input, &output) == 0);
    assert(vector_length_input == 169 * 0x1000);
    assert(output == 13 * 0x1000);
    assert(vector_square_call_count == 3 && vector_length_call_count == 1);

    vector_length_result = 5 * 0x1000;
    vector_length_input = 0;
    vector_length_call_count = 0;
    vector_square_call_count = 0;
    assert(vector_length_q12(sparse, &output) == 0);
    assert(vector_length_input == 25 * 0x1000);
    assert(output == 5 * 0x1000);
    assert(vector_square_call_count == 2 && vector_length_call_count == 1);

    vector_length_result = 0;
    vector_length_call_count = 0;
    vector_square_call_count = 0;
    assert(vector_length_q12(zero, &output) == 0);
    assert(vector_length_input == 0 && output == 0);
    assert(vector_square_call_count == 0 && vector_length_call_count == 1);
}

static void test_grounded_vector_transform(void) {
    const int32_t input[3] = {16, -32, 48};
    int32_t output[3] = {1, 2, 3};

    vector_length_result = 0;
    assert(vector_range_scale_q12(input, output) == 7);
    assert(output[0] == 0 && output[1] == 0 && output[2] == 0);

    vector_length_result = 0x1000;
    assert(vector_range_scale_q12(input, output) == 0);
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
    test_grounded_movement_helpers();
    test_component_basis_copy_helpers();
    test_component_basis_normalization_helpers();
    test_component_inverse_basis_wrapper();
    test_component_basis_transform_point_q12();
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
    test_move_request_enqueue_if_translation_pending();
    test_move_request_enqueue_if_actor_state_bit_2();
    test_move_request_caller_plans();
    test_move_request_directory_helpers();
    test_game_prepare_move_request();
    test_game_process_move_request_state_12();
    test_game_run_remaining_move_request_paths();
    test_game_run_move_request_path_3d5d8();
    test_game_apply_queued_move_request();
    test_component_transform_set_translation();
    test_component_transform_read_translation();
    test_component_transform_helper_models();
    test_actor_event_dispatch();
    test_bubsy_actor_update_entry_gate();
    test_bubsy_actor_update_level14_counter_path();
    test_bubsy_actor_default_state_route();
    test_bubsy_actor_shared_update_entry();
    test_bubsy_actor_347ec_mode_entry();
    test_bubsy_actor_347ec_argument_plan();
    test_bubsy_actor_347ec_mode_zero();
    test_bubsy_actor_347ec_mode_zero_call_plan();
    test_bubsy_actor_347ec_mode_zero_bit40_plan();
    test_bubsy_actor_347ec_mode_zero_alternate();
    test_bubsy_actor_347ec_followup_plan();
    test_bubsy_actor_347ec_state_bit4_clear_plan();
    test_bubsy_grounded_dispatch_table();
    test_bubsy_grounded_handler_model();
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