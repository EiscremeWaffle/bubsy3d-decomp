#include "result_display.h"
#include "texture_font_slots.h"

void func_8002DE34(void) {
    uint32_t phase = g_result_display_phase;
    int32_t scale, updated_scale;

    if (phase >= 0x23) {
        g_result_display_scale = (int32_t)((uint32_t)g_result_display_scale - 0x400U);
    } else if (phase == 1) {
        g_result_display_scale = (int32_t)((uint32_t)g_result_display_scale + 0x400U);
    }

    scale = g_result_display_scale;
    if (scale >= 0x1000) {
        func_800481D4(g_result_display_tag, 0x1000, scale);
    } else {
        func_800481D4(g_result_display_tag, scale, scale);
    }
    updated_scale = g_result_display_scale;
    if (updated_scale > 0x2800) {
        ++g_result_display_phase;
    } else if (updated_scale <= 0) {
        func_80048504(g_result_display_tag);
        func_800481D4(g_result_display_tag, 0x1000, 0x1000);
        func_80048C48(g_result_display_tag);
        g_result_display_scale = 0x1000;
        g_result_display_phase = 0;
        func_8004FFEC(g_result_display_event);
        g_result_display_event = 0;
    }
}
