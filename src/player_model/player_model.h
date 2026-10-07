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
    uint8_t is_bubsy;
    LevelModelConfig *config;
    void *asset;
    int32_t asset_size;
} PlayerModelState;

typedef int32_t (*PlayerModelLoadResource)(const char *path, void **asset_out, int32_t previous_size);
typedef void (*PlayerModelAssertFailure)(int32_t failed, const char *condition, const char *source_path, uint32_t source_line);

int32_t level_select_player_model_resource(PlayerModelState *state, PlayerModelLoadResource load_resource, PlayerModelAssertFailure assert_failure);
void level_get_player_model(const PlayerModelState *state, void **asset_out);
void *level_get_player_model_global(void **asset_out);

#endif
