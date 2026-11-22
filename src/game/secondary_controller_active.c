#include "secondary_controller_active.h"
#include "controller_callback_helpers.h"
#include "level_records.h"
#include "secondary_controller_entry.h"
#include "secondary_controller_helpers.h"
#include "sequence_queries.h"

extern uint32_t func_80056678(uint32_t value, uint32_t selector);

SecondaryControllerActiveResult secondary_controller_active_gate(
    uint32_t buttons, GroundedCase37ComponentView **object, void **component
) {
    GroundedCase37ComponentView *current;

    if ((uint8_t)func_80056678(buttons, 7) != 0 && g_unknown_8009F58C != 0) {
        return SECONDARY_ACTIVE_EXIT;
    }
    if (g_level_record_count <= 0) {
        current = func_80040FC0(0);
    } else {
        current = g_level_records->object_1C;
    }
    *object = current;
    *component = current->owner_1C->transform_14;
    if (g_unknown_8009F58C == 0) {
        func_8001C6BC(4, 5, 1);
        g_secondary_controller_flag_506 = 1;
    }
    return g_secondary_controller_flag_507 == 1 ? SECONDARY_ACTIVE_CONTINUE : SECONDARY_ACTIVE_EXIT;
}
