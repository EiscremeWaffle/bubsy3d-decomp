/* Host model of the entry gates in func_80036018; not a byte match. */
#include "controller_callback_helpers.h"
#include "heading_callback_gates.h"
#include "heading_entry_gates.h"

extern void func_8004733C(int32_t mode);
extern int32_t func_80052BDC(void *handle, int32_t mode);

HeadingEntryDecision heading_entry_gates(GameTransitionPayload *event) {
    HeadingEntryDecision decision = {0, 1};
    uint32_t counter = global_0x800BDE98;

    if (counter != g_heading_previous_counter) {
        g_heading_previous_counter = counter;
        decision.counter_changed = 1;
    }

    if (g_actor_environment == 19 && func_8002B620() == 1 && g_heading_stop_byte == 0) {
        uint32_t gate = g_unknown_8009F58C;
        if (gate == 0 || gate >= 0x80000000U) {
            func_8004733C(1);
        }
    }

    if (g_heading_stop_byte != 0) {
        if (g_heading_active_byte != 0) {
            func_80052BDC(g_actor_handles[0], 1);
            func_80052BDC(g_actor_handles[1], 1);
            g_heading_active_byte = 0;
        }
        if (event->event_id != 46) {
            decision.proceed = 0;
        }
    }
    return decision;
}
