#include "texture_font_slots.h"

extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

int32_t func_800482D0(int32_t value, const uint8_t color[3]) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t initial_value __asm__("$16") = value;
    register const uint8_t *loaded __asm__("$17");
    register const uint8_t *incoming_color __asm__("$5") = color;
    register const uint8_t *saved_color __asm__("$19");
    register int32_t index __asm__("$17");
    register int32_t offset __asm__("$16");
    register int32_t tag __asm__("$18");
    register uint32_t shifted __asm__("$2");

    /* Preserve the original argument and saved-register scheduling. */
    __asm__("" : "=r"(initial_value), "=r"(saved_color) : "0"(initial_value));
    loaded = &g_texture_font_loaded;
    __asm__ volatile("" : "=r"(loaded), "=r"(incoming_color)
                     : "0"(loaded), "1"(incoming_color));
#else
    int32_t initial_value = value;
    const uint8_t *loaded = &g_texture_font_loaded;
    const uint8_t *incoming_color = color;
    const uint8_t *saved_color;
    int32_t index;
    int32_t offset;
    int32_t tag;
#endif

    saved_color = incoming_color;
    if (*loaded != 1) {
        func_800556D0(0, g_texture_font_state_condition, g_texture_font_source_path, 0x19C);
    }
    if (*loaded == 0) {
        return 0;
    }
    index = 0;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    shifted = (uint32_t)initial_value << 24;
    __asm__ volatile("" : "=r"(shifted) : "0"(shifted), "r"(initial_value), "r"(saved_color));
    tag = (int32_t)shifted >> 24;
#else
    tag = (int8_t)initial_value;
#endif
    offset = 0;
    do {
        int32_t slot_tag = ((int8_t *)g_texture_font_slots)[offset];
        ++index;
        if (slot_tag == tag) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            register uint8_t *payload __asm__("$4") = (uint8_t *)g_texture_font_slots + 4;
            __asm__ volatile("" : "=r"(payload) : "0"(payload));
            if (func_80055EA8((uint8_t *)((uintptr_t)(uint32_t)offset + (uintptr_t)payload), saved_color) != 0) {
#else
            if (func_80055EA8((uint8_t *)g_texture_font_slots + offset + 4, saved_color) != 0) {
#endif
                func_800556D0(0, g_texture_font_error_condition, g_texture_font_source_path, 0x1A8);
            }
        }
        offset += sizeof(TextureFontSlot);
    } while (index < TEXTURE_FONT_SLOT_COUNT);
    return 1;
}
