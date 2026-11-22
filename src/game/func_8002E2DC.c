#include "actor_context.h"
#include "controller_result.h"
#include "grounded_event_records.h"

extern GroundedEventRecord *g_result_expiration_event;
extern int16_t g_result_expiration_selection;
extern void func_8002E348(int32_t delta);

void func_8002E2DC(void) {
    ResultCounterContext *context;
    ResultCounterSlot *slot;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    /* Preserve the incoming arguments passed to the context getter. */
    register void *incoming_actor __asm__("$4");
    register void *incoming_context __asm__("$5");
    __asm__("" : "=r"(incoming_actor), "=r"(incoming_context));
    context = func_80026648(incoming_actor, incoming_context);
#else
    context = func_80026648(0, 0);
#endif
    slot = &context->slots[context->selection == 0];
    if (slot->value == 0 && context->selection == g_result_expiration_selection) {
        func_8002E348(1);
    }
    func_8004FFEC(g_result_expiration_event);
    g_result_expiration_event = 0;
}
