#include "game_transition.h"
#include "input_events.h"
#include "texture_font_slots.h"
#include "timeout_resources.h"

extern const char g_sequence_query_condition[];
extern const char g_transition_source_path[];
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

int32_t func_8001C04C(void) {
    if (g_timeout_event_id != 0xFE && g_timeout_event_id != 0x16) {
        if (func_80062890() != 0) {
            func_800556D0(0, g_sequence_query_condition, g_transition_source_path, 0x49A);
        }
        func_80048488();
    }

    g_event_input_gate = 0;
    if (g_timeout_event_id != 0xFE) {
        func_8001BC28();
    }

    /* Adding two wraps the suppressed IDs 0xFE and 0xFF to zero and one. */
    if ((uint8_t)(g_timeout_event_id + 2u) >= 2) {
        /* Preserve the original argument-read order and reload the event ID. */
        uint32_t flags = g_timeout_event_flags;
        int32_t heading = g_timeout_event_heading;
        uint32_t event_id = *(volatile uint8_t *)&g_timeout_event_id;
        func_8001B558(event_id, 0, flags, heading, 0, 0, 0, 0);
    }
    return 0;
}
