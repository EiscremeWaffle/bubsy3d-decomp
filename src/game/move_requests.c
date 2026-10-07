#include "move_requests.h"

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
