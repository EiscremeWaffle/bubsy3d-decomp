#include "actor_context.h"
#include "controller_result.h"
#include "grounded_event_records.h"

extern void func_8002BB64(int32_t value);
extern void func_8002E2DC(void);
extern GroundedEventRecord *g_result_expiration_event;
extern int16_t g_result_expiration_selection;

void func_8002E348(int32_t delta) {
    ResultCounterContext *context;
    ResultCounterSlot *slot;
    int16_t value;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    /* The original getter receives the otherwise unused incoming second argument. */
    register void *incoming_context __asm__("$5");
    __asm__("" : "=r"(incoming_context));
    context = func_80026648((void *)(uintptr_t)(uint32_t)delta, incoming_context);
#else
    context = func_80026648((void *)(uintptr_t)(uint32_t)delta, 0);
#endif
    slot = &context->slots[context->selection == 0];
    value = (int16_t)((uint32_t)delta + (uint16_t)slot->value);
    if (value < 0) value = 0;
    slot->value = value;
    if (value == 0 && g_result_expiration_event == 0) {
        g_result_expiration_selection = context->selection;
        func_8004FF50((void *)func_8002E2DC, (void *)(uintptr_t)600, &g_result_expiration_event);
    }
    {
        int32_t selected = 10;
        if (context->selection == 0) selected = 11;
        func_8002BB64(selected);
    }
}
