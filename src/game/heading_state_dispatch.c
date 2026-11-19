/* Host model of the state switch and default route in func_80036018. */
#include "controller_callback_helpers.h"
#include "heading_effects.h"
#include "heading_entry_gates.h"
#include "heading_mode_one.h"
#include "heading_state_dispatch.h"
#include "heading_timed_records.h"
#include "sequence_limit.h"
#include "sequence_queries.h"

extern void func_8001AE88(uint32_t *output);
extern void func_80055680(const char *message);
extern void func_8003697C(BubsyGroundedActor *actor, void **transform, BubsyGroundedShared *shared);
extern int32_t func_80052AEC(void *component, int32_t value);
extern void func_80052E18(void *component);
extern void func_8003AC60(void);
extern uint8_t func_800283E0(void);
extern uint8_t func_80029548(int32_t kind, const char *resource, int32_t length, int16_t position[2]);
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);
extern void func_80039F34(GameTransitionPayload *event, BubsyGroundedActor *actor, void *transform);
extern void func_8003737C(BubsyGroundedActor *actor, void *transform);
extern void func_8003539C(BubsyGroundedActor *actor, int32_t mode);
extern void func_80037D14(BubsyGroundedActor *actor, void *transform, int32_t flags);
extern void func_8004D26C(int32_t mode, BubsyGroundedActor *actor, void *transform);
extern void func_8004D994(int32_t mode, BubsyGroundedActor *actor, void *transform);
extern void func_800347EC(BubsyGroundedActor *actor, void *transform, int32_t mode);
extern void func_8004B35C(
    GameTransitionPayload *event, BubsyGroundedActor *actor, void *transform,
    void *context, BubsyGroundedShared *shared
);
extern void func_8004BFF8(
    GameTransitionPayload *event, BubsyGroundedActor *actor, void *transform,
    void *context, BubsyGroundedShared *shared
);

void heading_state_dispatch(GameTransitionPayload *event, const HeadingCommonDecision *decision) {
    BubsyGroundedActor *actor = decision->actor;
    HeadingDispatchSharedView *shared = (HeadingDispatchSharedView *)decision->shared;
    void *transform = decision->transform;
    uint16_t state = event->event_id;

    switch (state) {
    case 49:
        func_8001AE88(&g_heading_dispatch_stop_tick);
        g_heading_saved_effect5 = -1;
        if (g_heading_active_byte == 2 || g_heading_active_byte == 3) {
            g_heading_saved_effect5 = (int16_t)(uint16_t)g_unknown_8009F58C;
        }
        if (g_runtime_mode != 0) {
            g_heading_saved_effect15 = (int16_t)(uint16_t)g_unknown_8009F7BC;
        }
        g_heading_sequence_started = 0;
        if (func_8001C7C4(5, 1) == 0) {
            func_8005E708(5, 1);
            g_heading_sequence_started = 1;
        }
        return;
    case 50: {
        uint32_t now;
        func_8001AE88(&now);
        if (g_heading_effect_started != 0) {
            g_heading_effect_started = now - g_heading_dispatch_stop_tick + g_heading_effect_started;
        }
        if (g_heading_sequence_started != 0) {
            g_heading_sequence_started = 0;
            func_8005E760(5, 1);
        }
        if (g_heading_saved_effect5 >= 0) {
            func_8002CA88(5, g_heading_saved_effect5);
        }
        if (g_heading_saved_effect15 >= 0) {
            func_8002CA88(15, g_heading_saved_effect15);
        }
        return;
    }
    case 48:
        func_80055680(g_heading_diagnostic_48);
        return;
    case 2:
        func_8003697C(actor, &transform, decision->shared);
        return;
    case 43: {
        uint8_t mode = g_runtime_mode;
        actor->unknown_08 = 0;
        actor->state_10 = 0;
        shared->shared.state_04 = -2;
        shared->vector_78[2] = 0;
        shared->vector_78[1] = 0;
        shared->vector_78[0] = 0;
        shared->shared.stride_30 = 0;
        shared->shared.stride_delta_34 = 0;
        shared->shared.pending_38[2] = 0;
        shared->shared.pending_38[1] = 0;
        shared->shared.pending_38[0] = 0;
        shared->vector_48[2] = 0;
        shared->vector_48[1] = 0;
        shared->vector_48[0] = 0;
        g_interaction_previous_phase = -1;
        g_heading_context_6450 = 0;
        g_heading_context_6452 = 0;
        g_stride_override = 0;
        g_heading_state_6462 = 0;
        g_heading_active_byte = 0;
        g_runtime_state_61 = 0;
        g_heading_flag_647A = 0;
        g_deceleration_state_6464 = 0;
        g_heading_status_6478 = 0;
        g_heading_context_645C = -1;
        func_80052AEC(actor->component_1C, mode != 0 ? 0x9A : 0x35);
        func_80052E18(actor->component_1C);
        func_8001B9C8();
        return;
    }
    case 40:
        shared->shared.pending_38[2] = 0;
        shared->shared.pending_38[1] = 0;
        shared->shared.pending_38[0] = 0;
        shared->vector_78[2] = 0;
        shared->vector_78[1] = 0;
        shared->vector_78[0] = 0;
        shared->vector_98[2] = 0;
        shared->vector_98[1] = 0;
        shared->vector_98[0] = 0;
        shared->shared.stride_30 = 0;
        shared->shared.stride_delta_34 = 0;
        g_deceleration_active_64B8 = 0;
        g_deceleration_state_6464 = 0;
        g_deceleration_stop_649C = 0;
        g_stride_override = 0;
        actor->state_01 = 2;
        func_8001B558(40, 5, 0xFFFE, 0xFFFE, 0, 0, 0, 0);
        return;
    case 42: {
        int16_t position[2] = {0x340, 0x100};
        const char *resource;
        int32_t length;
        if ((g_heading_level_config->flags_2A2A & 0x10) != 0) {
            func_8003AC60();
            return;
        }
        if (func_800283E0() == 0) {
            resource = g_heading_resource_mode_zero;
            length = 0x11;
        } else {
            resource = g_heading_resource_mode_other;
            length = 0x0F;
        }
        if (func_80029548(2, resource, length, position) != 1) {
            func_800556D0(0, g_heading_resource_condition, g_heading_resource_source, 0x6EE);
        }
        return;
    }
    case 46:
        func_80039F34(event, actor, transform);
        return;
    case 27:
        func_8003737C(actor, transform);
        return;
    case 22:
        if (actor->state_10 == 0) {
            func_8003539C(actor, 0);
        } else if (g_runtime_state_61 == 5) {
            func_8001B558(22, 4, actor->heading_0C, (int32_t)actor->heading_0C, 0, 0, 0, 0);
        }
        return;
    case 23:
        if ((actor->state_10 & 0x00100000U) == 0) {
            func_80037D14(actor, transform, event->flags);
        }
        return;
    case 18:
    case 19:
        if (g_runtime_mode == 0) {
            func_8004D26C(state == 18, actor, transform);
        } else {
            func_8004D994(state == 18, actor, transform);
        }
        return;
    case 39:
        func_800347EC(actor, transform, 1);
        return;
    default:
        break;
    }

    if ((actor->state_10 & 0x00100000U) != 0) {
        return;
    }
    if (g_runtime_state_61 == 5 &&
        state != 0x25 && !(state == 0x26 && event->flags != 0xFFFE)) {
        return;
    }
    if (g_heading_lock_6479 != 0 || g_heading_context_gate_6454 != 0 ||
        (actor->flags_04 & 0x100U) != 0) {
        return;
    }
    if (g_runtime_mode == 0) {
        func_8004B35C(event, actor, transform, decision->context, decision->shared);
    } else {
        func_8004BFF8(event, actor, transform, decision->context, decision->shared);
    }
}
