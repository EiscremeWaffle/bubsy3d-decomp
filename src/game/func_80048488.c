#include "texture_font_slots.h"

void func_80048488(void) {
    int32_t index;
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
        int32_t tag = g_texture_font_slots[index].tag;
        if (tag != -1) {
            func_80048504(tag);
            func_80048C48(tag);
        }
    }
}
