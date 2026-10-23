#include <stdint.h>

extern volatile int16_t g_move_request_count;
extern void * volatile g_move_request_entries[];
extern const char g_move_request_count_condition[];
extern const char g_game_source_path[];
extern int32_t func_800556D0(
    int32_t failed,
    const char *condition,
    const char *source_path,
    uint32_t source_line
);

int32_t func_80024EBC(void *request) {
    int16_t current_count = g_move_request_count;
    uint16_t index;
    int32_t slot_offset;

    if (current_count >= 30) {
        func_800556D0(1, g_move_request_count_condition, g_game_source_path, 0xB01);
    }

    index = (uint16_t)g_move_request_count;
    slot_offset = (int32_t)(int16_t)index * 4;
    g_move_request_count = (int16_t)(index + 1);
    *(void * volatile *)((uint8_t *)g_move_request_entries + slot_offset) = request;
    return slot_offset;
}
