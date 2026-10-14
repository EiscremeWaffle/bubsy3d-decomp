
#ifndef BUBSY3D_MOVE_REQUESTS_H
#define BUBSY3D_MOVE_REQUESTS_H

#include <stdint.h>

enum { GAME_MAX_MOVE_REQUESTS = 30 };

enum {
    GAME_MOVE_CALLER_ENQUEUE = 1u << 0,
    GAME_MOVE_CALLER_CALL_8001FD6C = 1u << 1,
    GAME_MOVE_CALLER_CALL_80022674 = 1u << 2,
    GAME_MOVE_CALLER_CALL_80022944 = 1u << 3,
    GAME_MOVE_CALLER_SET_ACTOR_BYTE_01 = 1u << 4,
    GAME_MOVE_CALLER_UPDATE_LINKED_STATE = 1u << 5,
    GAME_MOVE_CALLER_TRANSITION_26_4 = 1u << 6,
    GAME_MOVE_CALLER_TRANSITION_2F_1 = 1u << 7,
    GAME_MOVE_CALLER_TRANSITION_RELATED = 1u << 8,
    GAME_MOVE_CALLER_CALL_80052AEC = 1u << 9,
    GAME_MOVE_CALLER_CLEAR_SAVED_S2 = 1u << 10,
    GAME_MOVE_CALLER_CLEAR_GP_14 = 1u << 11,
    GAME_MOVE_CALLER_CALL_80022C94 = 1u << 12,
    GAME_MOVE_CALLER_CALL_80022C94_WITH_380 = 1u << 13,
    GAME_MOVE_CALLER_CALL_800691B8 = 1u << 14,
    GAME_MOVE_CALLER_SET_ACTOR_FLAG_04 = 1u << 15,
    GAME_MOVE_CALLER_CONTINUE_POST_ENQUEUE = 1u << 16,
    GAME_MOVE_CALLER_CALL_80028764 = 1u << 17,
    GAME_MOVE_CALLER_CALL_80053270 = 1u << 18,
    GAME_MOVE_CALLER_CALL_800509A4 = 1u << 19,
    GAME_MOVE_CALLER_CALL_8002205C = 1u << 20,
    GAME_MOVE_CALLER_TRANSITION_1B_3 = 1u << 21,
    GAME_MOVE_CALLER_TRANSITION_5_3 = 1u << 22,
    GAME_MOVE_CALLER_TRANSITION_6_1 = 1u << 23,
    GAME_MOVE_CALLER_TRANSITION_6_4 = 1u << 24,
    GAME_MOVE_CALLER_TRANSITION_0C_4 = 1u << 25,
};

uint32_t game_move_request_plan_15a54_state_12(
    uint32_t actor_state_before_22c94,
    uint32_t actor_state_after_22c94,
    uint8_t component_flags_04
);
uint32_t game_move_request_plan_3d5d8(int32_t positive_check_result, uint8_t actor_state_byte_01);
uint32_t game_move_request_plan_3d718(
    uint8_t actor_state_byte_01_before,
    int32_t query_22674_result,
    uint8_t actor_state_byte_01_after
);
uint32_t game_move_request_plan_4193c(uint8_t helper_22604_result);
uint32_t game_move_request_plan_41b08(uint16_t current_sequence_key, uint8_t saved_s2);

typedef struct GameMoveRequestQueue {
    uint16_t count;
    void **entries;
} GameMoveRequestQueue;

typedef void (*GameMoveRequestAssertion)(int32_t failed, const char *condition, const char *source_path, uint32_t source_line);

int32_t game_enqueue_move_request(GameMoveRequestQueue *queue, void *request, GameMoveRequestAssertion assertion);

int32_t game_enqueue_move_request_if_translation_pending(
    GameMoveRequestQueue *queue,
    void *request,
    const int32_t translation[3],
    GameMoveRequestAssertion assertion
);

void game_enqueue_move_request_if_actor_state_bit_2(
    GameMoveRequestQueue *queue,
    void *actor,
    uint32_t actor_state_10,
    GameMoveRequestAssertion assertion
);

enum {
    GAME_MOVE_INFO_BYTES = 0xA4,
    GAME_MOVE_INFO_WORDS = GAME_MOVE_INFO_BYTES / sizeof(int32_t),
};

typedef struct GameMoveInfoView {
    int32_t words[GAME_MOVE_INFO_WORDS];
} GameMoveInfoView;

_Static_assert(sizeof(GameMoveInfoView) == GAME_MOVE_INFO_BYTES, "move-info view size must match the observed accesses");

typedef struct GameMovePreparationObjectView {
    void *move_context;
    void *component;
    void *component_basis;
    GameMoveInfoView *move_info;
} GameMovePreparationObjectView;

typedef struct GameMovePreparationTableEntry {
    uint8_t tag;
    uint8_t selector;
    uint16_t reserved;
    int32_t value_04;
    const int32_t *payload_08;
} GameMovePreparationTableEntry;

typedef int32_t (*GameMovePreparationInvoke)(
    void *context,
    uint32_t function_address,
    uintptr_t argument_0,
    uintptr_t argument_1,
    uintptr_t argument_2,
    uintptr_t argument_3
);

typedef struct GameMovePreparationOps {
    void *context;
    GameMoveRequestAssertion assertion;
    GameMovePreparationInvoke invoke;
    void *global_transform;
} GameMovePreparationOps;

int32_t game_prepare_move_request(
    GameMoveRequestQueue *queue,
    void *actor,
    GameMovePreparationObjectView *object,
    const int32_t *basis_input,
    uint32_t operation_flags,
    uint8_t *completion_flag,
    const GameMovePreparationOps *ops
);

typedef struct GameMoveConsumerObjectView {
    void *component_transform;
    GameMoveInfoView *move_info;
} GameMoveConsumerObjectView;

typedef struct GameMoveConsumerOps {
    void *context;
    GameMoveRequestAssertion assertion;
    void (*call_800548A0)(void *context, void *component_transform, const int32_t *values_48);
    void (*call_800536F4)(void *context, const int32_t *values_98, int32_t *scratch);
    void (*call_80010038)(void *context, const int32_t *a0, const int32_t *a1, int32_t *a2);
    void (*call_80054B20)(void *context, void *component_transform, int32_t mode, const int32_t *values_68);
    void (*call_80054C7C)(void *context, void *component_transform, int32_t mode, const int32_t *translation);
} GameMoveConsumerOps;

void game_apply_queued_move_request(
    GameMoveConsumerObjectView *object,
    const GameMoveConsumerOps *ops
);

#endif
