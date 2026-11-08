#include "texture_font_slots.h"

extern int32_t func_80050534(void *payload);
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

int32_t func_800483C0(int32_t value) {
    int32_t index;
    int8_t tag;

    if (g_texture_font_loaded != 1) {
        func_800556D0(0, g_texture_font_state_condition, g_texture_font_source_path, 0x1BF);
    }
    tag = (int8_t)value;
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
        if (g_texture_font_slots[index].tag == tag) {
            if (func_80050534(g_texture_font_slots[index].payload) != 0) {
                func_800556D0(0, g_texture_font_error_condition, g_texture_font_source_path, 0x1C8);
            }
        }
    }
    return 1;
}
