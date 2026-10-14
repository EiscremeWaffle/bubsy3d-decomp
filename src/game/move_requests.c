#include "move_requests.h"

#include <string.h>

int32_t game_enqueue_move_request(GameMoveRequestQueue *queue, void *request, GameMoveRequestAssertion assertion) {
    const int16_t signed_count = (int16_t)queue->count;
    if (signed_count >= GAME_MAX_MOVE_REQUESTS) {
        assertion(1, "gMoveRequestCount < MAX_MOVE_REQUEST_COUNT", "../f/game.c", 0xB01);
    }

    const uint16_t index = queue->count;
    queue->count = (uint16_t)(index + 1);
    queue->entries[index] = request;
    return (int32_t)(int16_t)index * 4;
}

static int32_t move_vector_has_nonzero_component(const int32_t *vector) {
    return vector[0] != 0 || vector[1] != 0 || vector[2] != 0;
}

int32_t game_enqueue_move_request_if_translation_pending(
    GameMoveRequestQueue *queue,
    void *request,
    const int32_t translation[3],
    GameMoveRequestAssertion assertion
) {
    if (move_vector_has_nonzero_component(translation)) {
        game_enqueue_move_request(queue, request, assertion);
    }
    return 1;
}

uint32_t game_move_request_plan_3d5d8(int32_t positive_check_result, uint8_t actor_state_byte_01) {
    if (positive_check_result <= 0 || actor_state_byte_01 != 2) {
        return 0;
    }
    return GAME_MOVE_CALLER_SET_ACTOR_BYTE_01 |
        GAME_MOVE_CALLER_UPDATE_LINKED_STATE |
        GAME_MOVE_CALLER_TRANSITION_26_4 |
        GAME_MOVE_CALLER_TRANSITION_2F_1 |
        GAME_MOVE_CALLER_ENQUEUE;
}

uint32_t game_move_request_plan_3d718(uint8_t actor_state_byte_01, int32_t query_22674_result) {
    uint32_t actions = 0;

    if (actor_state_byte_01 != 1) {
        actions = GAME_MOVE_CALLER_CALL_8001FD6C | GAME_MOVE_CALLER_CALL_80022674;
        if (((uint32_t)query_22674_result & 0xFFu) == 1) {
            return actions;
        }
    }
    return actions | GAME_MOVE_CALLER_TRANSITION_2F_1 | GAME_MOVE_CALLER_ENQUEUE;
}

uint32_t game_move_request_plan_4193c(uint8_t helper_22604_result) {
    if (helper_22604_result == 0) {
        return GAME_MOVE_CALLER_CALL_80022944 | GAME_MOVE_CALLER_ENQUEUE;
    }
    return GAME_MOVE_CALLER_TRANSITION_RELATED | GAME_MOVE_CALLER_ENQUEUE;
}

uint32_t game_move_request_plan_41b08(uint16_t current_sequence_key, uint8_t saved_s2) {
    uint32_t actions = GAME_MOVE_CALLER_ENQUEUE;

    if (current_sequence_key == 0x15) {
        return actions | GAME_MOVE_CALLER_CLEAR_SAVED_S2;
    }

    actions |= GAME_MOVE_CALLER_CALL_80052AEC;
    if (saved_s2 != 0) {
        actions |= GAME_MOVE_CALLER_CALL_80022944;
    }
    return actions;
}

void game_enqueue_move_request_if_actor_state_bit_2(
    GameMoveRequestQueue *queue,
    void *actor,
    uint32_t actor_state_10,
    GameMoveRequestAssertion assertion
) {
    if ((actor_state_10 & 0x4u) != 0) {
        game_enqueue_move_request(queue, actor, assertion);
    }
}

void game_apply_queued_move_request(
    GameMoveConsumerObjectView *object,
    const GameMoveConsumerOps *ops
) {
    GameMoveInfoView *move_info = object->move_info;
    int32_t rotation_scratch[4] = {0, 0, 0, 0};
    int32_t translation_scratch[4] = {0, 0, 0, 0};
    int32_t *rotation_48;
    int32_t *translation_38;
    int32_t *state_68;
    int32_t *rotation_98;

    if (move_info == NULL) {
        if (ops->assertion != NULL) {
            ops->assertion(0, "mi", "../f/game.c", 0xB12);
        }
        return;
    }

    rotation_48 = move_info->words + 0x48 / sizeof(int32_t);
    translation_38 = move_info->words + 0x38 / sizeof(int32_t);
    state_68 = move_info->words + 0x68 / sizeof(int32_t);
    rotation_98 = move_info->words + 0x98 / sizeof(int32_t);

    if (move_vector_has_nonzero_component(rotation_48)) {
        ops->call_800548A0(ops->context, object->component_transform, rotation_48);
    }

    if (move_vector_has_nonzero_component(rotation_98)) {
        ops->call_800536F4(ops->context, rotation_98, rotation_scratch);
        ops->call_80010038(ops->context, translation_38, rotation_scratch, translation_38);
        memset(rotation_98, 0, 3 * sizeof(*rotation_98));
    }

    ops->call_80054B20(ops->context, object->component_transform, 0, state_68);

    if (move_vector_has_nonzero_component(translation_38)) {
        ops->call_80010038(ops->context, state_68, translation_38, translation_scratch);
        ops->call_80054C7C(ops->context, object->component_transform, 0, translation_scratch);
    }

    memcpy(move_info->words + 0x78 / sizeof(int32_t), translation_38, 4 * sizeof(*translation_38));
    memset(translation_38, 0, 3 * sizeof(*translation_38));
    memset(rotation_48, 0, 3 * sizeof(*rotation_48));
}
