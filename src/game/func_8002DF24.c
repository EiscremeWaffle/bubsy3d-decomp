#include "result_display.h"
#include "texture_font_slots.h"

extern void func_800486D0(void *context, int32_t first, int32_t second, int8_t *tag);

void func_8002DF24(const char *text) {
    if (g_result_display_phase != 0) {
        return;
    }

    g_result_display_phase = 1;
    g_result_display_scale = 0;
    func_800486D0((void *)text, -1, 0xF0, &g_result_display_tag);
    func_800481D4(g_result_display_tag, 0, 0);
    func_800483C0(g_result_display_tag);
    func_8004FF50((void *)func_8002DE34, (void *)(uintptr_t)4, &g_result_display_event);
}
