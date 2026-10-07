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

#endif
