
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
};

uint32_t game_move_request_plan_3d5d8(int32_t positive_check_result, uint8_t actor_state_byte_01);
uint32_t game_move_request_plan_3d718(uint8_t actor_state_byte_01, int32_t query_22674_result);
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
