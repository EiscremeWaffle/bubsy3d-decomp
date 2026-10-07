#ifndef BUBSY3D_PLAYER_MODEL_H
#define BUBSY3D_PLAYER_MODEL_H

#include <stddef.h>
#include <stdint.h>

typedef struct LevelModelConfig {
    uint8_t unknown_0000_to_2A29[0x2A2A];
    uint8_t flags_2A2A;
} LevelModelConfig;

_Static_assert(offsetof(LevelModelConfig, flags_2A2A) == 0x2A2A, "config flag offset must match the executable");

typedef struct PlayerModelState {
    uint8_t is_swimming;
    LevelModelConfig *config;
    void *asset;
    int32_t asset_size;
} PlayerModelState;

typedef struct PlayerActorFlagByte {
    uint8_t unknown_00_to_03[4];
    uint8_t flags_04;
} PlayerActorFlagByte;

_Static_assert(offsetof(PlayerActorFlagByte, flags_04) == 0x04, "actor flag byte offset must match the executable");

typedef struct PlayerActorSequenceData {
    uint8_t unknown_00_to_03[4];
    const int32_t *entries;
} PlayerActorSequenceData;

typedef struct PlayerActorSequenceView {
    uint8_t unknown_00_to_03[4];
    uint8_t flags_04;
    int8_t state_05;
    uint8_t unknown_06;
    uint8_t advance_cursor_07;
    int16_t cursor_08;
    uint8_t unknown_0A_to_0D[4];
    int16_t threshold_0E;
    uint8_t unknown_10_to_17[8];
    const PlayerActorSequenceData *sequence_18;
} PlayerActorSequenceView;

_Static_assert(offsetof(PlayerActorSequenceView, flags_04) == 0x04, "sequence flag offset must match the executable");
_Static_assert(offsetof(PlayerActorSequenceView, state_05) == 0x05, "sequence state offset must match the executable");
_Static_assert(offsetof(PlayerActorSequenceView, advance_cursor_07) == 0x07, "sequence direction offset must match the executable");
_Static_assert(offsetof(PlayerActorSequenceView, cursor_08) == 0x08, "sequence cursor offset must match the executable");
_Static_assert(offsetof(PlayerActorSequenceView, threshold_0E) == 0x0E, "sequence threshold offset must match the executable");
_Static_assert(offsetof(PlayerActorSequenceView, sequence_18) == 0x18, "sequence pointer offset must match the executable");

typedef int32_t (*PlayerModelLoadResource)(const char *path, void **asset_out, int32_t previous_size);
typedef void (*PlayerModelAssertFailure)(int32_t failed, const char *condition, const char *source_path, uint32_t source_line);

int32_t level_select_player_model_resource(PlayerModelState *state, PlayerModelLoadResource load_resource, PlayerModelAssertFailure assert_failure);
uint8_t level_get_player_model_mode(void);
void level_get_player_model(const PlayerModelState *state, void **asset_out);
void *level_get_player_model_global(void **asset_out);
int32_t player_actor_set_flag_04(PlayerActorFlagByte *actor);
int32_t player_actor_clear_flag_04(PlayerActorFlagByte *actor);
int32_t player_actor_sequence_boundary_reached(const PlayerActorSequenceView *actor, uint8_t *result);

#endif
