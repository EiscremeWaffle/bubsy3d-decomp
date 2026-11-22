#include "texture_font_slots.h"

extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

int32_t func_800481D4(int32_t value, int32_t x, int32_t y) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t initial_value __asm__("$16") = value;
    register const uint8_t *loaded __asm__("$17");
    register int32_t incoming_y __asm__("$6") = y;
    register int32_t saved_y __asm__("$21");
    register int32_t saved_x __asm__("$20") = x;
    register int32_t index __asm__("$18");
    register int32_t offset __asm__("$17");
    register int32_t tag __asm__("$19");
    register uint32_t shifted __asm__("$2");

    /* Preserve argument lifetimes and the original saved-register scheduling. */
    __asm__("" : "=r"(initial_value), "=r"(saved_y) : "0"(initial_value));
    loaded = &g_texture_font_loaded;
    __asm__ volatile("" : "=r"(loaded), "=r"(incoming_y)
                     : "0"(loaded), "1"(incoming_y));
#else
    int32_t initial_value = value;
    const uint8_t *loaded = &g_texture_font_loaded;
    int32_t incoming_y = y, saved_y;
    int32_t saved_x = x;
    int32_t index, offset, tag;
#endif

    saved_y = incoming_y;
    if (*loaded != 1) {
        func_800556D0(0, g_texture_font_state_condition, g_texture_font_source_path, 0x17A);
    }
    if (*loaded == 0) return 0;
    index = 0;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    shifted = (uint32_t)initial_value << 24;
    __asm__ volatile("" : "=r"(shifted) : "0"(shifted), "r"(initial_value), "r"(saved_y));
    tag = (int32_t)shifted >> 24;
#else
    tag = (int8_t)initial_value;
#endif
    offset = 0;
    do {
        int32_t slot_tag = ((int8_t *)g_texture_font_slots)[offset];
        ++index;
        if (slot_tag == tag) {
            TextureFontPositionPayload *payload;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            register uint8_t *base __asm__("$16") = (uint8_t *)g_texture_font_slots + 4;
            __asm__ volatile("" : "=r"(base), "=r"(saved_x) : "0"(base), "1"(saved_x));
            payload = (TextureFontPositionPayload *)((uintptr_t)(uint32_t)offset + (uintptr_t)base);
#else
            payload = &g_texture_font_slots[index - 1].data.position_payload;
#endif
            func_80055DF4(payload, (int16_t)saved_x);
            func_80055E38(payload, (int16_t)saved_y);
        }
        offset += sizeof(TextureFontSlot);
    } while (index < TEXTURE_FONT_SLOT_COUNT);
    return 1;
}
