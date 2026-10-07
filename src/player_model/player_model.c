#include "player_model.h"

#define PLAYER_MODEL_FORM_FLAG 0x10u

int32_t level_select_player_model_resource(
    PlayerModelState *state,
    PlayerModelLoadResource load_resource,
    PlayerModelAssertFailure assert_failure
) {
    const int is_swimming = (state->config->flags_2A2A & PLAYER_MODEL_FORM_FLAG) != 0;
    const char *path;

    if (state->is_bubsy != 0) {
        path = is_swimming ? "BUBSWIM.TZP" : "BUB.TZP";
    } else {
        path = is_swimming ? "PLISWIM.TZP" : "PLISKIN.TZP";
    }

    state->asset_size = load_resource(path, &state->asset, state->asset_size);
    if (state->asset_size == 0) {
        assert_failure(1, "errorFlag == EGSBoolTrue", "../f/level.c", 0x495);
    }

    return state->asset_size;
}

void level_get_player_model(const PlayerModelState *state, void **asset_out) {
    *asset_out = state->asset;
}
