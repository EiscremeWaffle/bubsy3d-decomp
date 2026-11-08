#include "texture_font_slots.h"

int32_t func_80048C48(int32_t value) {
    int32_t removed = 0;
    int32_t index;
    int8_t tag;

    if (g_texture_font_loaded == 0) {
        return 1;
    }

    tag = (int8_t)value;
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
        if (g_texture_font_slots[index].tag == tag) {
            g_texture_font_slots[index].tag = -1;
            ++removed;
        }
    }
    g_texture_font_count -= (uint32_t)removed;
    return 1;
}
