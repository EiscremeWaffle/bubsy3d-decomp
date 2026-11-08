#include "bubsy_actor_state_update.h"
#include "bubsy_grounded_interaction.h"
#include "game_transition.h"
#include "sequence_queries.h"

extern BubsyInteractionState g_interaction_state;
extern int32_t g_interaction_previous_phase;
extern uint8_t g_interaction_active;
extern uint8_t g_interaction_marker;
extern uint8_t g_stride_mode;

extern void func_80037214(int32_t mode, BubsyGroundedActor *actor);
extern void func_8002262C(BubsyGroundedActor *actor, int32_t selector);
extern void func_80022650(BubsyGroundedActor *actor, int32_t selector);
extern int32_t func_80054B20(void *transform, int32_t mode, int32_t *position);
extern int32_t func_80054C7C(void *transform, int32_t mode, const int32_t *position);
extern int32_t func_800516F8(void **output);
extern int32_t func_800550A4(void *transform, void *basis);
extern uint8_t func_80038C0C(
    BubsyGroundedActor *actor,
    void *context,
    BubsyInteractionTarget *target,
    int32_t mode
);
extern void func_8002DBD4(int32_t mode, void *actor);

typedef struct BubsyInteractionVector {
    int32_t values[3];
    int32_t padding;
} BubsyInteractionVector;

void func_80039158(BubsyGroundedActor *actor_input, void *context_input) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register BubsyGroundedActor *actor __asm__("$16") = actor_input;
    register void *context __asm__("$20") = context_input;
    register BubsyInteractionTarget *target __asm__("$18");
    register BubsyInteractionState *interaction __asm__("$19");
    register void *target_transform __asm__("$17");
    register uint8_t phase __asm__("$17");
    register BubsyGroundedActor *update_actor_arg __asm__("$4");
    register int32_t update_mode_arg __asm__("$5");
#else
    BubsyGroundedActor *actor = actor_input;
    void *context = context_input;
    BubsyInteractionTarget *target;
    BubsyInteractionState *interaction;
    void *target_transform;
    uint8_t phase;
    BubsyGroundedActor *update_actor_arg;
    int32_t update_mode_arg;
#endif
    void *reference_transform;
    BubsyInteractionVector target_position;
    BubsyInteractionVector context_position;
    int32_t event_id;

    if (actor == 0) {
        g_interaction_active = 0;
        return;
    }

    interaction = &g_interaction_state;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(interaction) : "0"(interaction));
#endif
    target = func_80028764(interaction->target_index);
    phase = interaction->phase_05;
    if (phase == 2) {
        if (target->used_2C != 0) {
            g_interaction_active = 0;
            interaction->phase_05 = 0;
            func_8001C700(5, 1);
            func_80037214(1, actor);
            func_80037214(0, actor);
            func_8002262C(actor, 5);
            func_8002262C(actor, 17);
            update_actor_arg = actor;
            update_mode_arg = 1;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            __asm__("" : "=r"(update_actor_arg), "=r"(update_mode_arg) : "0"(update_actor_arg), "1"(update_mode_arg));
#endif
            update_actor_arg->state_10 = 0;
            goto update_actor_ready;
        }

        if (g_interaction_active == 0) {
            actor->state_10 = 0;
            actor->shared_28->stride_30 = 0;
            actor->shared_28->stride_delta_34 = 0;
            func_80022650(actor, 17);
            func_80037214(1, actor);
            target_transform = target->component_1C->transform_14;
            g_interaction_active = 1;
            func_80054B20(target_transform, 0, target_position.values);
            func_80054B20(context, 0, context_position.values);
            target_position.values[1] = context_position.values[1];
            func_80054C7C(context, 0, target_position.values);
            if (g_stride_mode == 0) {
                target_position.values[1] = (int32_t)((uint32_t)target_position.values[1] + 0x40u);
            } else {
                target_position.values[1] = (int32_t)((uint32_t)target_position.values[1] + 0x60u);
            }
            func_80054C7C(target_transform, 0, target_position.values);
            func_800516F8(&reference_transform);
            func_800550A4(reference_transform, context);
            func_8001C6BC(5, 1, 1);
            event_id = 0x25;
            goto event_common;
        }

        if (func_8001C7C4(5, 1) == 0) {
            target_transform = target->component_1C->transform_14;
            func_80054B20(context, 0, context_position.values);
            target_position.values[0] = context_position.values[0];
            if (g_stride_mode == 0) {
                target_position.values[1] = (int32_t)((uint32_t)context_position.values[1] + 0x40u);
            } else {
                target_position.values[1] = (int32_t)((uint32_t)context_position.values[1] + 0x60u);
            }
            target_position.values[2] = context_position.values[2];
            func_80054C7C(target_transform, 0, target_position.values);
            func_800516F8(&reference_transform);
            func_800550A4(target_transform, reference_transform);
            return;
        }

        {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            register uint32_t confirmed_phase __asm__("$21") = interaction->phase_05;
#else
            uint32_t confirmed_phase = interaction->phase_05;
#endif
            if (confirmed_phase == phase) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                register BubsyInteractionTarget *confirmed_target __asm__("$17");
#else
                BubsyInteractionTarget *confirmed_target;
#endif
                if (func_8001C7C4(5, 1) == 0) {
                    goto process_phase;
                }
                confirmed_target = func_80028764(interaction->target_index);
                if (confirmed_target->used_2C != 0) {
                    goto event_25;
                }
                interaction->phase_05 = 3;
                g_interaction_previous_phase = confirmed_phase;
                func_80038C0C(actor, context, target, 1);
                func_8001B558(0x1B, 4, actor->heading_0C, confirmed_target->heading_0C, 0, 0, 0, 0);
                confirmed_target->used_2C = 1;
event_25:
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                __asm__ volatile("" : : : "memory");
#endif
                event_id = 0x25;
                goto event_common;
            }
        }
    }

process_phase:
    {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register uint8_t *phase_address __asm__("$17");
#else
        uint8_t *phase_address;
#endif
        uint32_t tail_phase;
        phase_address = &g_interaction_state.phase_05;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(phase_address) : "0"(phase_address));
#endif
        tail_phase = *phase_address;
        if (tail_phase == 3) {
            if (func_80038C0C(actor, context, target, 0) == 0) {
                event_id = 0x25;
                goto event_common;
            }
            g_interaction_active = 0;
            *phase_address = 0;
            func_8002262C(actor, 13);
            func_8002262C(actor, 17);
            goto update_actor;
        }
        if (tail_phase == 4) {
            int32_t target_index = g_interaction_state.target_index;
            g_interaction_active = 0;
            g_interaction_marker = 0;
            if (g_level_records[target_index].flag_20 == 0) {
                *phase_address = 5;
                func_8001C700(5, 1);
                func_8001B558(0x2F, 4, actor->heading_0C, g_interaction_state.target_index, 0, 0, 0, 0);
            } else {
                func_8001C700(5, 1);
                func_80037214(0, actor);
                func_8002262C(actor, 17);
                func_8002262C(actor, 5);
                *phase_address = 0;
            }
            event_id = 0x16;
event_common:
            func_8001B558(event_id, 4, actor->heading_0C, actor->heading_0C, 0, 0, 0, 0);
            return;
        }
        if (tail_phase == 5) {
            func_80037214(0, actor);
            g_interaction_marker = 0;
            *phase_address = 0;
            func_8002262C(actor, 17);
            func_8002262C(actor, 5);
            func_8002DBD4(1, 0);
            goto update_actor;
        }
        return;
    }

update_actor:
    update_actor_arg = actor;
    update_mode_arg = 1;
update_actor_ready:
    func_8003539C(update_actor_arg, update_mode_arg);
    return;
}
