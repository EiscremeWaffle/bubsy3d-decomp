/* Host model of the common pre-switch part of func_80036018; not a byte match. */
#include "actor_context.h"
#include "bubsy_actor_state_update.h"
#include "heading_callback_gates.h"
#include "heading_common_gates.h"
#include "heading_effects.h"
#include "heading_entry_gates.h"
#include "sequence_queries.h"

extern void *func_80028764(int32_t index);
extern void func_800395E8(BubsyGroundedActor *actor, void *transform);

HeadingCommonDecision heading_common_gates(GameTransitionPayload *event, uint8_t counter_changed) {
    HeadingCommonDecision decision;
    BubsyGroundedActor *actor = func_80028764(event->heading);
    HeadingComponentView *component = actor->component_1C;

    decision.actor = actor;
    decision.transform = component->transform_14;
    decision.shared = actor->shared_28;
    decision.context = func_80026648(0, 0);
    decision.proceed = 1;

    if (event->event_id != 50) {
        func_80039AB4(actor, decision.transform);
    }
    if (g_runtime_mode == 1) {
        func_800395E8(actor, decision.transform);
    }
    if (g_deceleration_active_64B8 != 0) {
        func_80038844(actor);
    }

    if (counter_changed != 0 && actor->component_1C != 0 && g_heading_stop_byte == 0 &&
        (actor->state_10 & 0x80000U) == 0) {
        func_8001B558(4, 1, 0xFFFE, 0xFFFE, &actor->heading_0C, 0, 0, 0);
        if (g_runtime_state_61 != 5 && ((HeadingActorView *)actor)->state_2C == 0) {
            HeadingSharedView *shared = (HeadingSharedView *)actor->shared_28;
            if (shared->state_7C < 0 || g_heading_status_6478 == 1) {
                func_8001B558(5, 3, 0xFFFE, 0xFFFE, &actor->heading_0C, 0, 0, 0);
                g_heading_status_6478 = 0;
            } else {
                func_8001B558(5, 4, 0xFFFE, 0xFFFE, &actor->heading_0C, 0, 0, 0);
            }
        }
    }

    if ((actor->state_10 & 0x80000U) != 0 &&
        event->event_id != 46 && event->event_id != 18 &&
        event->event_id != 19 && event->event_id != 27) {
        decision.proceed = 0;
        return decision;
    }

    if ((actor->state_10 & 0x40U) != 0 && (actor->flags_04 & 0x1000U) != 0) {
        func_8001C700(4, 3);
        g_deceleration_stop_649C = 0;
        if (g_deceleration_state_6464 != 0) {
            actor->state_01 = 1;
        }
        actor->state_10 = 0;
        g_stride_override = 0;
        func_8003539C(actor, 1);
    } else if ((actor->state_10 & 4U) != 0 && (actor->flags_04 & 0x1000U) != 0) {
        g_deceleration_stop_649C = 0;
        actor->state_10 = 0;
        g_stride_override = 0;
        func_8003539C(actor, 1);
    }

    if ((uint8_t)func_8002E824() == 0 && (actor->state_10 & 0x1000U) == 0 &&
        (actor->state_10 & 0x8100U) != 0x100U && g_heading_stop_byte == 0) {
        func_8002E7FC();
    }
    return decision;
}
