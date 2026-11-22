#include "secondary_controller_confirm.h"
#include "controller_bitmap_gate.h"
#include "secondary_controller_helpers.h"
#include "sequence_queries.h"

extern uint32_t func_80056678(uint32_t value, uint32_t selector);
extern int32_t func_8003AA1C(void **output);

SecondaryControllerConfirmResult secondary_controller_confirm_gate(
    uint32_t buttons, ResultCounterContext *context
) {
    if ((uint8_t)func_80056678(buttons, 7) != 0) {
        ResultCounterSlot *slot;
        void *output;

        if (g_secondary_controller_flag_505 != 0) return SECONDARY_CONFIRM_ACTIVE;
        slot = &context->slots[context->selection == 0];
        func_8003AA1C(&output);
        if (slot->value == 0 || ((ControllerGameContext *)output)->unknown_00[0x11] == 5) {
            func_8001C6BC(4, 5, 1);
            return SECONDARY_CONFIRM_EXIT;
        }
        return SECONDARY_CONFIRM_START;
    }
    return g_secondary_controller_flag_505 != 0 ? SECONDARY_CONFIRM_ACTIVE : SECONDARY_CONFIRM_EXIT;
}
