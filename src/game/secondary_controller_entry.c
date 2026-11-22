#include "secondary_controller_entry.h"
#include "actor_context.h"
#include "component_records.h"
#include "game_transition.h"
#include "heading_timed_records.h"
#include "input_events.h"

extern void *func_80028764(int32_t index);
extern uint32_t func_8004753C(void);
extern int32_t func_8002BD0C(int32_t value);
extern int32_t func_8003AA1C(void **output);
extern void func_8002DF24(const char *text);
extern uint32_t func_80056678(uint32_t value, uint32_t selector);

SecondaryControllerEntryResult secondary_controller_entry(uint32_t buttons, void **context) {
    int16_t timer;

    *context = func_80026648((void *)(uintptr_t)buttons, 0);
    if (g_secondary_controller_initialized == 0) {
        uint32_t record[8];

        func_8002CA4C(17, 240);
        g_secondary_controller_initialized = 1;
        func_80052094(g_extra_model_handle, 0, record);
        record[0] = (uint32_t)-40;
        record[1] = (uint32_t)-40;
        record[4] = 40;
        record[5] = 40;
        func_800528A4(g_extra_model_handle, 0, record);
    }

    timer = g_secondary_controller_timer;
    if (timer >= 0) {
        timer = (int16_t)(uint16_t)(timer + 1);
        g_secondary_controller_timer = timer;
        if (timer == 90) {
            SecondaryControllerActor *actor = func_80028764(0);
            uint32_t selection;

            actor->owner_1c->status_20 = 0;
            selection = func_8004753C();
            func_8001B558(23, 5, 0, (int32_t)selection, 0, 0, 0, 0);
            func_8001BC28();
        }
        return SECONDARY_CONTROLLER_ENTRY_EXIT;
    }

    if (func_8002BD0C(17) == 0) {
        ControllerGameContext *game_context;
        void *output;

        func_8003AA1C(&output);
        game_context = output;
        game_context->blocked_2b = 1;
        game_context->mode_6d = 2;
        g_secondary_controller_timer = 0;
        func_8002DF24(g_secondary_controller_timeout_text);
        func_8001BC08();
        return SECONDARY_CONTROLLER_ENTRY_EXIT;
    }

    if (func_8001BCE4() != 0) {
        return SECONDARY_CONTROLLER_ENTRY_EXIT;
    }
    if (g_secondary_controller_flag_506 == 1) {
        if ((uint8_t)func_80056678(buttons, 7) != 0) {
            return SECONDARY_CONTROLLER_ENTRY_EXIT;
        }
        g_secondary_controller_flag_506 = 0;
    }
    return SECONDARY_CONTROLLER_ENTRY_CONTINUE;
}
