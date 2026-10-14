
#ifndef BUBSY3D_MOVE_REQUESTS_H
#define BUBSY3D_MOVE_REQUESTS_H

#include <stdint.h>

enum { GAME_MAX_MOVE_REQUESTS = 30 };

typedef struct GameMoveRequestQueue {
    uint16_t count;
    void **entries;
} GameMoveRequestQueue;

typedef void (*GameMoveRequestAssertion)(int32_t failed, const char *condition, const char *source_path, uint32_t source_line);

int32_t game_enqueue_move_request(GameMoveRequestQueue *queue, void *request, GameMoveRequestAssertion assertion);

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
