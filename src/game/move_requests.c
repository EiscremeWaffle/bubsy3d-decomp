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

static int32_t move_request_add_wrap(int32_t left, int32_t right) {
    const uint32_t bits = (uint32_t)left + (uint32_t)right;
    int32_t result;

    memcpy(&result, &bits, sizeof(result));
    return result;
}

static int32_t move_request_call(
    const GameMovePreparationOps *ops,
    uint32_t function_address,
    uintptr_t argument_0,
    uintptr_t argument_1,
    uintptr_t argument_2,
    uintptr_t argument_3
) {
    return ops->invoke(
        ops->context,
        function_address,
        argument_0,
        argument_1,
        argument_2,
        argument_3
    );
}

static void move_request_assert(
    const GameMovePreparationOps *ops,
    uint32_t source_line
) {
    if (ops->assertion != NULL) {
        ops->assertion(1, "errorFlag == EGSBoolTrue", "../f/game.c", source_line);
    }
}

static int32_t move_vector_has_nonzero_component(const int32_t *vector);

int32_t game_prepare_move_request(
    GameMoveRequestQueue *queue,
    void *actor,
    GameMovePreparationObjectView *object,
    const int32_t *basis_input,
    uint32_t operation_flags,
    uint8_t *completion_flag,
    const GameMovePreparationOps *ops
) {
    GameMoveInfoView *move_info;
    int32_t *words;
    const GameMovePreparationTableEntry *table = NULL;
    uint32_t table_limit = 0;
    int32_t counter;
    int32_t index;
    uint8_t table_mode;
    uint8_t selector = 0;
    int32_t component_translation[3] = {0, 0, 0};
    int32_t transformed_basis[16] = {0};
    int32_t transform_values[4] = {0, 0, 0, 0};
    int32_t helper_result;
    size_t element;

    if (object == NULL) {
        return 0;
    }

    move_info = object->move_info;
    if (object->move_context == NULL) {
        move_request_assert(ops, 0xD27);
    }
    if (move_info == NULL) {
        move_request_assert(ops, 0xD2A);
        return 0;
    }
    if (completion_flag != NULL) {
        *completion_flag = 0;
    }
    if (ops == NULL || ops->invoke == NULL) {
        return 0;
    }

    words = move_info->words;
    counter = words[0];
    index = words[0x04 / sizeof(int32_t)];
    table_mode = ((const uint8_t *)words)[0x1C];

    if ((uint32_t)index + 2u < 2u) {
        return 1;
    }

    if (words[0x18 / sizeof(int32_t)] <= 0) {
        uintptr_t table_result = (uintptr_t)&table;
        uintptr_t limit_result = (uintptr_t)&table_limit;

        helper_result = move_request_call(
            ops,
            0x80028C18,
            (uintptr_t)object->move_context,
            limit_result,
            table_result,
            0
        );
        if (((uint32_t)helper_result & 0xFFu) != 1) {
            move_request_assert(ops, 0xD3A);
        }
        if (table == NULL || index < 0 || (uint32_t)index >= table_limit) {
            return 0;
        }

        if (table[index].tag == 0x0A) {
            counter = table[index].value_04;
        } else {
            counter = move_request_add_wrap(counter, 1);
        }
        words[0] = counter;

        if (counter >= table[index].value_04) {
            if (table_mode == 1) {
                const int32_t next_index = move_request_add_wrap(index, 1);
                if ((uint32_t)next_index == table_limit) {
                    if (completion_flag != NULL) {
                        *completion_flag = 1;
                    }
                    ((uint8_t *)words)[0x1C] = 0;
                    words[0x04 / sizeof(int32_t)] = next_index - 1;
                    return 1;
                }
                index = next_index;
                counter = 0;
                words[0] = 0;
                words[0x04 / sizeof(int32_t)] = index;
            }

            if (table_mode == 0 || table_mode == 2) {
                for (;;) {
                    index--;
                    words[0x04 / sizeof(int32_t)] = index;
                    if (index < 0) {
                        table_mode = 1;
                        ((uint8_t *)words)[0x1C] = table_mode;
                        words[0x04 / sizeof(int32_t)] = 0;
                        words[0] = 0;
                        break;
                    }
                    if ((uint32_t)index >= table_limit) {
                        return 0;
                    }
                    if (table[index].tag == 0x0A) {
                        words[0] = table[index].value_04 - 1;
                        break;
                    }
                }
            }
        }
    }

    counter = words[0];
    index = words[0x04 / sizeof(int32_t)];
    table_mode = ((const uint8_t *)words)[0x1C];
    if (words[0x18 / sizeof(int32_t)] > 0) {
        goto notify_and_enqueue;
    }
    if (table == NULL || index < 0 || (uint32_t)index >= table_limit) {
        return 0;
    }

    while (words[0x18 / sizeof(int32_t)] == 0) {
        const int32_t *payload;

        index = words[0x04 / sizeof(int32_t)];
        counter = words[0];
        if (object->component == NULL || table == NULL || index < 0 ||
            (uint32_t)index >= table_limit || counter < 0 ||
            table[index].payload_08 == NULL) {
            return 0;
        }
        payload = table[index].payload_08 + (size_t)counter * 4;
        for (element = 0; element < 3; element++) {
            words[0x08 / sizeof(int32_t) + element] = move_request_add_wrap(
                words[0x08 / sizeof(int32_t) + element], payload[element]
            );
        }

        selector = 0;
        move_request_call(
            ops,
            0x800286F8,
            (uintptr_t)object->move_context,
            (uintptr_t)&selector,
            0,
            0
        );
        if (selector == 1) {
            words[0x30 / sizeof(int32_t)] = (int32_t)table[index].selector * 2;
        }

        helper_result = move_request_call(
            ops,
            0x80054B20,
            (uintptr_t)object->component,
            0,
            (uintptr_t)component_translation,
            0
        );
        if (helper_result != 0) {
            return 0;
        }

        helper_result = move_request_call(
            ops,
            0x80062D4C,
            (uintptr_t)component_translation,
            (uintptr_t)&words[0x08 / sizeof(int32_t)],
            (uintptr_t)&words[0x18 / sizeof(int32_t)],
            0
        );
        if (helper_result != 0) {
            move_request_assert(ops, 0xDA7);
        }

        if (words[0x18 / sizeof(int32_t)] == 0) {
            if (table[index].tag != 0x0A) {
                move_request_assert(ops, 0xDAD);
            }
            words[0] = words[0] == 0 ? 1 : 0;
        }
    }

    helper_result = move_request_call(
        ops,
        0x80054C7C,
        (uintptr_t)ops->global_transform,
        0,
        (uintptr_t)&words[0x08 / sizeof(int32_t)],
        0
    );
    if (helper_result != 0) {
        return 0;
    }

    helper_result = move_request_call(
        ops,
        0x800546F4,
        (uintptr_t)object->component_basis,
        (uintptr_t)transformed_basis,
        0,
        0
    );
    if (helper_result != 0) {
        return 0;
    }
    helper_result = move_request_call(
        ops,
        0x800550A4,
        (uintptr_t)object->component_basis,
        (uintptr_t)ops->global_transform,
        0,
        0
    );
    if (helper_result != 0) {
        return 0;
    }
    helper_result = move_request_call(
        ops,
        0x8005541C,
        (uintptr_t)object->component_basis,
        (uintptr_t)basis_input,
        0,
        0
    );
    if (helper_result != 0) {
        return 0;
    }

    if ((operation_flags & 1u) == 0 && (operation_flags & 4u) == 0) {
        goto clear_transform_values;
    }
    helper_result = move_request_call(
        ops,
        0x80054AF0,
        (uintptr_t)object->component_basis,
        (uintptr_t)transform_values,
        0,
        0
    );
    if (helper_result != 0) {
        return 0;
    }

    if ((uint32_t)transform_values[1] + 0xCCBu >= 0x1997u ||
        (operation_flags & 1u) != 0) {
        for (element = 0; element < 4; element++) {
            words[0x88 / sizeof(int32_t) + element] = transform_values[element];
        }
        helper_result = move_request_call(
            ops,
            0x800549E8,
            (uintptr_t)object->component_basis,
            (uintptr_t)transformed_basis,
            0,
            0
        );
        if (helper_result != 0) {
            return 0;
        }
    } else {
clear_transform_values:
        words[0x88 / sizeof(int32_t)] = 0;
        words[0x8C / sizeof(int32_t)] = 0;
        words[0x90 / sizeof(int32_t)] = 0;
    }

notify_and_enqueue:
    move_request_call(ops, 0x80022944, (uintptr_t)actor, 0, 0, 0);
    return game_enqueue_move_request_if_translation_pending(
        queue,
        actor,
        &words[0x38 / sizeof(int32_t)],
        ops->assertion
    );
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
        GAME_MOVE_CALLER_ENQUEUE |
        GAME_MOVE_CALLER_CALL_80028764 |
        GAME_MOVE_CALLER_CALL_80053270 |
        GAME_MOVE_CALLER_CALL_800509A4;
}

uint32_t game_move_request_plan_15a54_state_12(
    uint32_t actor_state_before_22c94,
    uint32_t actor_state_after_22c94,
    uint8_t component_flags_04
) {
    uint32_t actions = GAME_MOVE_CALLER_CLEAR_GP_14;

    if ((actor_state_before_22c94 & 0x4u) == 0) {
        return actions |
            GAME_MOVE_CALLER_CALL_80052AEC |
            GAME_MOVE_CALLER_SET_ACTOR_FLAG_04 |
            GAME_MOVE_CALLER_CALL_80022C94_WITH_380 |
            GAME_MOVE_CALLER_ENQUEUE |
            GAME_MOVE_CALLER_TRANSITION_0C_4;
    }

    actions |= GAME_MOVE_CALLER_CALL_80022C94;
    if ((component_flags_04 & 0x3u) == 1) {
        actions |= GAME_MOVE_CALLER_CALL_800691B8;
    }
    if ((actor_state_after_22c94 & 0x4u) != 0) {
        return actions | GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_TRANSITION_0C_4;
    }
    return actions | GAME_MOVE_CALLER_CONTINUE_POST_ENQUEUE;
}

static int32_t move_state12_call(
    const GameMoveState12Ops *ops,
    uint32_t function_address,
    uintptr_t argument_0,
    uintptr_t argument_1,
    uintptr_t argument_2,
    uintptr_t argument_3,
    uintptr_t stack_0,
    uintptr_t stack_1,
    uintptr_t stack_2,
    uintptr_t stack_3
) {
    const uintptr_t arguments[8] = {
        argument_0, argument_1, argument_2, argument_3,
        stack_0, stack_1, stack_2, stack_3,
    };

    return ops->invoke(ops->context, function_address, arguments);
}

static int32_t move_request_negate_wrap(int32_t value) {
    const uint32_t bits = 0U - (uint32_t)value;
    int32_t result;

    memcpy(&result, &bits, sizeof(result));
    return result;
}

int32_t game_run_move_request_path_3d5d8(
    GameMoveRequestQueue *queue,
    GameMoveRequestState3D5D8View *state,
    int32_t positive_check_result,
    uint8_t comparison_actor_byte_01,
    const GameMoveState12Ops *ops
) {
    int32_t helper_result;
    uintptr_t returned_object;

    if (state == NULL || state->actor == NULL || state->actor_byte_01 == NULL ||
        state->actor_word_14 == NULL || state->actor_context == NULL ||
        state->linked_object_word_24 == NULL ||
        state->global_object_3E8 == NULL || state->global_byte_3E4 == NULL ||
        state->global_actor_3F8 == NULL || ops == NULL || ops->invoke == NULL ||
        positive_check_result <= 0 || *state->actor_byte_01 == comparison_actor_byte_01 ||
        *state->actor_byte_01 != 2) {
        return 0;
    }

    *state->actor_byte_01 = 1;
    *state->actor_word_14 = state->actor_frame_word_b8;
    *state->linked_object_word_24 = (int32_t)state->linked_state_value_24;
    state->global_object_3E8->word_30 = 0x60;
    state->global_object_3E8->word_88 = move_request_negate_wrap(state->global_object_3E8->word_88);
    state->global_object_3E8->word_90 = move_request_negate_wrap(state->global_object_3E8->word_90);
    state->global_byte_3E4[0] = 0;
    *state->global_actor_3F8 = state->actor;

    move_state12_call(ops, 0x8001B558, 0x26, 4, (uintptr_t)state->actor_context, 0, 0, 0, 0, 0);
    move_state12_call(
        ops,
        0x8001B558,
        0x2F,
        1,
        (uintptr_t)state->actor_context,
        (uintptr_t)state->actor_context,
        0,
        0,
        0,
        0
    );
    game_enqueue_move_request(queue, state->actor, ops->assertion);

    returned_object = (uintptr_t)(uint32_t)move_state12_call(ops, 0x80028764, 0, 0, 0, 0, 0, 0, 0, 0);
    move_state12_call(ops, 0x80053270, returned_object + 0x20, 0, 0, 0, 0, 0, 0, 0);
    helper_result = move_state12_call(ops, 0x800509A4, 1, (uintptr_t)state->argument_509A4_1, 1, 0, 0, 0, 0, 0);
    return helper_result;
}

int32_t game_process_move_request_state_12(
    GameMoveRequestQueue *queue,
    GameMoveState12View *state,
    const GameMoveState12Ops *ops
) {
    int should_enqueue;

    if (state == NULL || state->actor == NULL || state->move_context == NULL ||
        state->actor_state_10 == NULL || state->global_byte_14 == NULL ||
        ops == NULL || ops->invoke == NULL) {
        return 0;
    }

    *state->global_byte_14 = 0;
    if ((*state->actor_state_10 & 0x4u) == 0) {
        move_state12_call(ops, 0x80052AEC, (uintptr_t)state->move_context, 0, 0, 0, 0, 0, 0, 0);
        move_state12_call(ops, 0x80052E18, (uintptr_t)state->move_context, 0, 0, 0, 0, 0, 0, 0);
        move_state12_call(ops, 0x80022C94, (uintptr_t)state->actor, 0x380, 0, 0, 0x5A, 0x61, 0x68, 0);
        should_enqueue = 1;
    } else {
        if ((state->component_flags_04 & 0x3u) == 1) {
            move_state12_call(ops, 0x800691B8, (uintptr_t)state->shared_691b8_context, 0, 0, 0, 0, 0, 0, 0);
        }
        move_state12_call(ops, 0x80022C94, (uintptr_t)state->actor, 0, 0, 0, 0x5A, 0x61, 0x68, 0);
        should_enqueue = (int)((*state->actor_state_10 >> 2) & 1u);
    }

    if (!should_enqueue) {
        return ops->continue_after_no_enqueue != NULL
            ? ops->continue_after_no_enqueue(ops->context, state)
            : 0;
    }

    game_enqueue_move_request(queue, state->actor, ops->assertion);
    return move_state12_call(
        ops,
        0x8001B558,
        0x0C,
        4,
        (uintptr_t)state->move_context,
        (uintptr_t)state->move_context,
        0,
        0,
        0,
        0
    );
}

int32_t game_run_move_request_path_3d718(
    GameMoveRequestQueue *queue,
    void *actor,
    void *actor_context,
    uint8_t *actor_state_byte_01,
    const GameMoveState12Ops *ops
) {
    int32_t helper_result = 0;
    int32_t enqueue_result;
    int byte_was_one;

    if (actor == NULL || actor_context == NULL || actor_state_byte_01 == NULL ||
        ops == NULL || ops->invoke == NULL) {
        return 0;
    }

    move_state12_call(ops, 0x80022944, (uintptr_t)actor, 0, 0, 0, 0, 0, 0, 0);
    byte_was_one = *actor_state_byte_01 == 1;
    if (!byte_was_one) {
        move_state12_call(ops, 0x8001FD6C, (uintptr_t)actor, 0, 0, 0, 0, 0, 0, 0);
        helper_result = move_state12_call(ops, 0x80022674, (uintptr_t)actor, 0x0B, 0, 0, 0, 0, 0, 0);
        if (((uint32_t)helper_result & 0xFFu) == 1) {
            move_state12_call(ops, 0x8001B558, 0x1B, 3, 0, (uintptr_t)actor_context, 0, 0, 0, 0);
            return move_state12_call(ops, 0x8002205C, (uintptr_t)actor, 0, 0, 0, 0, 0, 0, 0);
        }
    }

    move_state12_call(ops, 0x8001B558, 0x2F, 1, (uintptr_t)actor_context, (uintptr_t)actor_context, 0, 0, 0, 0);
    enqueue_result = game_enqueue_move_request(queue, actor, ops->assertion);
    if (*actor_state_byte_01 != 1) {
        return move_state12_call(ops, 0x8001B558, 5, 3, (uintptr_t)(intptr_t)-2, (uintptr_t)(intptr_t)-2, (uintptr_t)actor_context, 0, 0, 0);
    }
    return enqueue_result;
}

int32_t game_run_move_request_path_4193c(
    GameMoveRequestQueue *queue,
    void *actor,
    void *actor_context,
    int32_t helper_22604_result,
    const GameMoveState12Ops *ops
) {
    if (actor == NULL || actor_context == NULL || ops == NULL || ops->invoke == NULL) {
        return 0;
    }
    if (((uint32_t)helper_22604_result & 0xFFu) == 0) {
        move_state12_call(ops, 0x80022944, (uintptr_t)actor, 0, 0, 0, 0, 0, 0, 0);
    }
    game_enqueue_move_request(queue, actor, ops->assertion);
    return move_state12_call(ops, 0x8001B558, 6, 1, (uintptr_t)actor_context, (uintptr_t)actor_context, 0, 0, 0, 0);
}

int32_t game_run_move_request_path_41b08(
    GameMoveRequestQueue *queue,
    void *actor,
    void *move_context,
    void *actor_context,
    uint16_t current_sequence_key,
    uint8_t saved_s2,
    const GameMoveState12Ops *ops
) {
    if (actor == NULL || move_context == NULL || actor_context == NULL || ops == NULL || ops->invoke == NULL) {
        return 0;
    }
    if (current_sequence_key == 0x15) {
        saved_s2 = 0;
    } else {
        move_state12_call(ops, 0x80052AEC, (uintptr_t)move_context, 0x15, 0, 0, 0, 0, 0, 0);
    }
    if (saved_s2 != 0) {
        move_state12_call(ops, 0x80022944, (uintptr_t)actor, 0, 0, 0, 0, 0, 0, 0);
    }
    game_enqueue_move_request(queue, actor, ops->assertion);
    return move_state12_call(ops, 0x8001B558, 6, 4, (uintptr_t)actor_context, (uintptr_t)actor_context, 0, 0, 0, 0);
}

uint32_t game_move_request_plan_3d718(
    uint8_t actor_state_byte_01_before,
    int32_t query_22674_result,
    uint8_t actor_state_byte_01_after
) {
    uint32_t actions = GAME_MOVE_CALLER_CALL_80022944;

    if (actor_state_byte_01_before != 1) {
        actions |= GAME_MOVE_CALLER_CALL_8001FD6C | GAME_MOVE_CALLER_CALL_80022674;
        if (((uint32_t)query_22674_result & 0xFFu) == 1) {
            return actions | GAME_MOVE_CALLER_TRANSITION_1B_3 | GAME_MOVE_CALLER_CALL_8002205C;
        }
    }
    actions |= GAME_MOVE_CALLER_TRANSITION_2F_1 | GAME_MOVE_CALLER_ENQUEUE;
    if (actor_state_byte_01_after != 1) {
        actions |= GAME_MOVE_CALLER_TRANSITION_5_3;
    }
    return actions;
}

uint32_t game_move_request_plan_4193c(uint8_t helper_22604_result) {
    uint32_t actions = GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_TRANSITION_6_1;

    if (helper_22604_result == 0) {
        actions |= GAME_MOVE_CALLER_CALL_80022944;
    }
    return actions;
}

uint32_t game_move_request_plan_41b08(uint16_t current_sequence_key, uint8_t saved_s2) {
    uint32_t actions = GAME_MOVE_CALLER_ENQUEUE | GAME_MOVE_CALLER_TRANSITION_6_4;

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
