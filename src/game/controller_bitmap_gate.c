/* Host model of the pre-bitmap gates of func_80023E94, not a byte match. */
#include "controller_bitmap_gate.h"
#include "controller_input_route.h"
#include "input_events.h"

extern uint32_t func_80056678(uint32_t value, uint32_t selector);
extern int32_t func_80047050(int32_t value);
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);
extern void func_80038430(GameCyclicEntry *entry);
extern int32_t func_800222E0(void);
extern void func_80023A84(void);
extern int32_t func_8003AA1C(void **output);

int32_t controller_gate_bitmap(
    ControllerGameContext *context, uint8_t mode, uint32_t buttons, GameCyclicEntry *cycle_entry
) {
    if (context->blocked_12 != 0 || context->blocked_2b != 0) {
        return 0;
    }

    if ((uint8_t)func_80056678(buttons, 11) != 0) {
        uint8_t hold = g_controller_button_hold;
        if (hold != 0) {
            if (hold >= 61) {
                g_controller_lifecycle_state = 2;
                func_80047050(1);
                func_8001B558(1, 5, 0xFFFF, 0xFFFF, 0, 0, 0, 0);
            }
            g_controller_button_hold = (uint8_t)(g_controller_button_hold + 1U);
            return 0;
        }

        if ((g_controller_runtime_flags->buttons_2a2a & 4U) != 0
            && (uint8_t)func_80056678(buttons, 15) != 0) {
            if (func_80023A30(60) == 0) {
                return 0;
            }
            if ((uint8_t)func_8001A280(&g_controller_cycle_cursor, cycle_entry) != 1) {
                func_800556D0(0, g_controller_cycle_condition, g_controller_cycle_source, 0xA19);
            }
            func_80038430(cycle_entry);
        }
        if (func_80023A30(60) == 0 || mode == 1 || context->blocked_29 != 0
            || (uint8_t)func_800222E0() == 0) {
            return 0;
        }
        g_controller_action_latched = 1;
        func_80023A84();
        return 0;
    }

    if ((uint8_t)func_80056678(buttons, 8) != 0) {
        ControllerRuntimeFlags *flags = g_controller_runtime_flags;
        g_controller_button_hold = 1;
        if ((flags->buttons_2a2a & 8U) != 0
            && (uint8_t)func_80056678(buttons, 4) != 0 && func_80023A30(60) != 0) {
            void *output;
            ControllerGameContext *refreshed;
            func_8003AA1C(&output);
            refreshed = output;
            refreshed->toggle_13 = refreshed->toggle_13 == 4 ? 0 : 4;
        }
        return 0;
    }

    g_controller_button_hold = 0;
    {
        uint8_t timeout = g_controller_timeout_ticks;
        if (timeout != 0) {
            g_controller_timeout_ticks = (uint8_t)(timeout - 1U);
            return 0;
        }
    }
    return 1;
}
