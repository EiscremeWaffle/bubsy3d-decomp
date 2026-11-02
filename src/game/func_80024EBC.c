#include <stdint.h>

extern uint16_t g_move_request_count;
extern void *g_move_request_entries[];
extern const char g_move_request_count_condition[];
extern const char g_game_source_path[];
extern int32_t func_800556D0(
    int32_t failed,
    const char *condition,
    const char *source_path,
    uint32_t source_line
);

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
int32_t func_80024EBC(void *request) __asm__("game_enqueue_move_request");
#endif

int32_t func_80024EBC(void *request) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t index __asm__("$2");
#else
    uint32_t index;
#endif

    if ((int16_t)g_move_request_count >= 30) {
        func_800556D0(1, g_move_request_count_condition, g_game_source_path, 0xB01);
    }

    index = (uint16_t)g_move_request_count;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(index) : "0"(index));
#endif
    g_move_request_count = (uint16_t)(index + 1);
    g_move_request_entries[(int16_t)index] = request;
    return (int32_t)(int16_t)index * 4;
}
