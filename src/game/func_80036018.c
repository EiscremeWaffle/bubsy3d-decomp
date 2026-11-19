#include "actor_context.h"
#include "bubsy_actor_state_update.h"
#include "controller_callback_helpers.h"
#include "heading_callback_gates.h"
#include "heading_common_gates.h"
#include "heading_effects.h"
#include "heading_entry_gates.h"
#include "heading_mode_one.h"
#include "heading_state_dispatch.h"
#include "heading_timed_records.h"
#include "sequence_limit.h"
#include "sequence_queries.h"

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
/* These aliases address the second handle and the timer's low halfword on PSX. */
extern void *g_heading_second_handle;
extern uint16_t g_heading_budget15_low;
#define HEADING_SECOND_HANDLE g_heading_second_handle
#define HEADING_BUDGET15_LOW g_heading_budget15_low
#else
#define HEADING_SECOND_HANDLE g_actor_handles[1]
#define HEADING_BUDGET15_LOW ((uint16_t)g_unknown_8009F7BC)
#endif
extern void func_8004733C(int32_t mode);
extern int32_t func_80052BDC(void *handle, int32_t mode);
extern void *func_80028764(int32_t heading);
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
extern void func_80037D14(BubsyGroundedActor *actor, void *transform, int32_t flags);
extern void func_8004D26C(int32_t mode, BubsyGroundedActor *actor, void *transform);
extern void func_8004D994(int32_t mode, BubsyGroundedActor *actor, void *transform);
extern void func_800347EC(BubsyGroundedActor *actor, void *transform, int32_t mode);
extern void func_8004B35C(GameTransitionPayload *event, BubsyGroundedActor *actor, void *transform,
    void *context, BubsyGroundedShared *shared);
extern void func_8004BFF8(GameTransitionPayload *event, BubsyGroundedActor *actor, void *transform,
    void *context, BubsyGroundedShared *shared);

typedef struct HeadingCallbackLocals {
    uint32_t now;
    void *transform;
    int16_t position[2];
} HeadingCallbackLocals;

void func_80036018(GameTransitionPayload *event) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint8_t handled __asm__("$22") = 1;
    register uint8_t changed __asm__("$17") = 0;
    register BubsyGroundedActor *actor __asm__("$16");
    register HeadingDispatchSharedView *shared __asm__("$18");
    register void *context __asm__("$21");
    register uint32_t changed_gate __asm__("$2");
    register uint32_t handled_value __asm__("$2");
    register uint32_t one __asm__("$20");
#else
    BubsyGroundedActor *actor;
    HeadingDispatchSharedView *shared;
    void *context;
    uint8_t changed = 0, handled = 1;
    uint32_t one;
#endif
    HeadingCallbackLocals locals;
    uint32_t counter = global_0x800BDE98;

    locals.transform = 0;
    if (counter != g_heading_previous_counter) {
        g_heading_previous_counter = counter;
        changed = 1;
    }
    if (g_actor_environment == 19 && func_8002B620() == 1 && g_heading_stop_byte == 0 &&
        (int32_t)g_unknown_8009F58C <= 0) {
        func_8004733C(1);
    }
    if (g_heading_stop_byte != 0) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register uint8_t *active __asm__("$16") = &g_heading_active_byte;
        __asm__ volatile("" : "=r"(active) : "0"(active));
#else
        uint8_t *active = &g_heading_active_byte;
#endif
        if (*active != 0) {
            func_80052BDC(g_actor_handles[0], 1);
            func_80052BDC(HEADING_SECOND_HANDLE, 1);
            *active = 0;
        }
        if (event->event_id != 46) {
            return;
        }
    }

    actor = func_80028764(event->heading);
    locals.transform = ((HeadingComponentView *)actor->component_1C)->transform_14;
    shared = (HeadingDispatchSharedView *)actor->shared_28;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    {
        /* The context getter ignores its arguments; preserve the original call registers. */
        register void *ignored_actor __asm__("$4");
        register void *ignored_context __asm__("$5");
        __asm__ volatile("" : "=r"(ignored_actor), "=r"(ignored_context));
        context = func_80026648(ignored_actor, ignored_context);
    }
    __asm__ volatile("" : "=r"(context) : "0"(context));
#else
    context = func_80026648(0, 0);
#endif
    one = 1;
    if (event->event_id != 50) {
        func_80039AB4(actor, locals.transform);
    }
    if (g_runtime_mode == one) {
        func_800395E8(actor, locals.transform);
    }
    if (g_deceleration_active_64B8 != 0) {
        func_80038844(actor);
    }
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    changed_gate = changed;
    __asm__ volatile("" : "=r"(changed_gate) : "0"(changed_gate));
    if (changed_gate != 0 && actor->component_1C != 0 && g_heading_stop_byte == 0 &&
#else
    if (changed != 0 && actor->component_1C != 0 && g_heading_stop_byte == 0 &&
#endif
        (actor->state_10 & 0x80000U) == 0) {
        func_8001B558(4, 1, 0xFFFE, 0xFFFE, &actor->heading_0C, 0, 0, 0);
        if (g_runtime_state_61 != 5 && ((HeadingActorView *)actor)->state_2C == 0) {
            if (((HeadingSharedView *)actor->shared_28)->state_7C < 0 ||
                g_heading_status_6478 == one) {
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
        return;
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

    switch (event->event_id) {
    case 49:
        func_8001AE88(&g_heading_dispatch_stop_tick);
        g_heading_saved_effect5 = -1;
        if (g_heading_active_byte == 2 || g_heading_active_byte == 3) {
            g_heading_saved_effect5 = (int16_t)(uint16_t)g_unknown_8009F58C;
        }
        if (g_runtime_mode != 0) {
            g_heading_saved_effect15 = (int16_t)HEADING_BUDGET15_LOW;
        }
        g_heading_sequence_started = 0;
        if (func_8001C7C4(5, 1) == 0) {
            func_8005E708(5, 1);
            g_heading_sequence_started = 1;
        }
        break;
    case 50:
        func_8001AE88(&locals.now);
        if (g_heading_effect_started != 0) {
            g_heading_effect_started = locals.now - g_heading_dispatch_stop_tick + g_heading_effect_started;
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
        break;
    case 48:
        func_80055680(g_heading_diagnostic_48);
        break;
    case 2:
        func_8003697C(actor, &locals.transform, &shared->shared);
        break;
    case 43: {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register uint32_t mode __asm__("$3") = g_runtime_mode;
#else
        uint32_t mode = g_runtime_mode;
#endif
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
        if (mode == 0) {
            func_80052AEC(actor->component_1C, 0x35);
        } else {
            func_80052AEC(actor->component_1C, 0x9A);
        }
        func_80052E18(actor->component_1C);
        func_8001B9C8();
        break;
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
        break;
    case 42: {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register int32_t kind __asm__("$4");
        register const char *resource __asm__("$5");
        register int32_t length __asm__("$6");
        register uint32_t result __asm__("$3");
#else
        int32_t kind;
        const char *resource;
        int32_t length;
        uint32_t result;
#endif
        locals.position[0] = 0x340;
        locals.position[1] = 0x100;
        if ((g_heading_level_config->flags_2A2A & 0x10) != 0) {
            func_8003AC60();
        } else {
            uint32_t query = func_800283E0();
            kind = 2;
            if (query == 0) {
                resource = g_heading_resource_mode_zero;
                length = 0x11;
            } else {
                resource = g_heading_resource_mode_other;
                length = 0x0F;
            }
            result = func_80029548(kind, resource, length, locals.position);
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            __asm__ volatile("" : "=r"(result) : "0"(result));
#endif
            if (result != 1) {
                func_800556D0(0, g_heading_resource_condition, g_heading_resource_source, 0x6EE);
            }
        }
        break;
    }
    case 46:
        func_80039F34(event, actor, locals.transform);
        break;
    case 27:
        func_8003737C(actor, locals.transform);
        break;
    case 22:
        if (actor->state_10 == 0) {
            func_8003539C(actor, 0);
        } else if (g_runtime_state_61 == 5) {
            func_8001B558(22, 4, actor->heading_0C, (int32_t)actor->heading_0C, 0, 0, 0, 0);
        }
        break;
    case 23:
        if ((actor->state_10 & 0x00100000U) == 0) {
            func_80037D14(actor, locals.transform, event->flags);
        }
        break;
    case 18:
        if (g_runtime_mode == 0) {
            func_8004D26C(1, actor, locals.transform);
        } else {
            func_8004D994(1, actor, locals.transform);
        }
        break;
    case 19:
        if (g_runtime_mode == 0) {
            func_8004D26C(0, actor, locals.transform);
        } else {
            func_8004D994(0, actor, locals.transform);
        }
        break;
    case 39:
        func_800347EC(actor, locals.transform, 1);
        break;
    default:
        handled = 0;
        break;
    }
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    handled_value = handled;
    __asm__ volatile("" : "=r"(handled_value) : "0"(handled_value));
    if (handled_value != 0 || (actor->state_10 & 0x00100000U) != 0) {
#else
    if (handled != 0 || (actor->state_10 & 0x00100000U) != 0) {
#endif
        return;
    }
    if (g_runtime_state_61 == 5 &&
        (event->event_id != 0x26 || event->flags == 0xFFFE) &&
        event->event_id != 0x25) {
        return;
    }
    if (g_heading_lock_6479 != 0 || g_heading_context_gate_6454 != 0 ||
        (actor->flags_04 & 0x100U) != 0) {
        return;
    }
    if (g_runtime_mode == 0) {
        func_8004B35C(event, actor, locals.transform, context, &shared->shared);
    } else {
        func_8004BFF8(event, actor, locals.transform, context, &shared->shared);
    }
}
