#include "heading_callback_gates.h"

extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

uint8_t func_8002B620(void) {
    if (g_actor_environment != 19) {
        func_800556D0(0, g_heading_gate_condition, g_heading_gate_source_path, 0x8D7);
    }
    return g_heading_context_gate;
}
