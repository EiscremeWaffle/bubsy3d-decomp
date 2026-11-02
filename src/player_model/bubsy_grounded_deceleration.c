#include "bubsy_grounded_state.h"

typedef struct BubsyDecelerationComponent {
    uint8_t unknown_00[0x14];
    void *transform_14;
} BubsyDecelerationComponent;

extern uint8_t g_stride_mode;
extern uint8_t g_deceleration_state_6464;
extern uint8_t g_deceleration_stop_649C;
extern uint8_t g_deceleration_active_64B8;
extern int32_t g_deceleration_state_64C0;
extern const char g_stride_error_condition[];
extern const char g_stride_source_path[];

extern void func_8002262C(BubsyGroundedActor *actor, int32_t selector);
extern void func_80022650(BubsyGroundedActor *actor, int32_t selector);
extern void func_8003539C(BubsyGroundedActor *actor, int32_t mode);
extern void func_80054B20(void *transform, int32_t mode, int32_t *output);
extern void func_8001B558(int32_t state, int32_t mode, int32_t argument_2, int32_t argument_3,
    int32_t *argument_4, int32_t *argument_5, int32_t *argument_6, int32_t *argument_7);
extern int32_t func_80052AEC(void *component, int32_t sequence);
extern void func_80052E18(void *component);
extern void func_800556D0(int32_t level, const char *condition, const char *source, uint32_t line);
extern void func_80022944(BubsyGroundedActor *actor);

void func_80038844(BubsyGroundedActor *actor) {
    BubsyGroundedShared *shared = actor->shared_28;
    uint8_t was_state_8 = 0;
    int32_t translation[4];
    int32_t event_value;
    int32_t delta;
    int32_t stride;
    int32_t prospective;

    if (shared->stride_30 <= 0) {
        shared->stride_30 = 0;
        shared->stride_delta_34 = 0;
        func_8002262C(actor, 11);
        if (*(volatile uint8_t *)&g_stride_mode == 0 && (actor->state_10 & 0x44) == 0) {
            goto reset_actor;
        }
        if (g_stride_mode == 1 && g_deceleration_state_6464 == 0) {
            func_8002262C(actor, 6);
reset_actor:
            func_8003539C(actor, 1);
        }
        goto deactivate;
    }

    if (shared->stride_delta_34 >= 0) {
        if ((actor->state_10 & 8) != 0) {
            was_state_8 = 1;
        }
        if (g_stride_mode != 0 || (actor->state_10 & 0x200) != 0 ||
            (actor->state_10 & 0x18) == 0) {
            goto reset_delta;
        }
        if ((actor->state_10 & 0x10) != 0) {
            func_80054B20(((BubsyDecelerationComponent *)actor->component_1C)->transform_14,
                0, translation);
            event_value = 14;
            func_8001B558(26, 4, 0xFFFE, 0xFFFE, &event_value,
                &translation[0], &translation[1], &translation[2]);
        }
        if (func_80052AEC(actor->component_1C, 0x318) != 0) {
            func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0xBC5);
        }
        func_80052E18(actor->component_1C);
reset_delta:
        func_8002262C(actor, 3);
        func_8002262C(actor, 4);
        func_80022650(actor, 11);
        shared->stride_delta_34 = 0;
    }

    if ((uint8_t)was_state_8 != 0) {
        shared->stride_delta_34 = -0xFF78;
    } else if (g_stride_mode == 0 &&
        ((actor->state_10 & 4) != 0 || g_deceleration_state_6464 != 0)) {
        if (shared->stride_delta_34 >= 0) {
            shared->stride_delta_34 = -0x199;
        }
        if (g_deceleration_state_64C0 != 0) {
            shared->stride_delta_34 = (int32_t)((uint32_t)shared->stride_delta_34 - 0x332u);
        } else {
            shared->stride_delta_34 = (int32_t)((uint32_t)shared->stride_delta_34 - 0x199u);
        }
    } else if ((actor->state_10 & 0x200) != 0) {
        shared->stride_delta_34 = (int32_t)((uint32_t)shared->stride_delta_34 - 0x51u);
    } else {
        shared->stride_delta_34 = (int32_t)((uint32_t)shared->stride_delta_34 - 0x51Cu);
    }

    if (shared->stride_30 > 35) {
        shared->stride_30 = 35;
    }
    if (g_deceleration_stop_649C != 0) {
deactivate:
        g_deceleration_active_64B8 = 0;
        return;
    }
    if (shared->pending_38[0] != 0 || shared->pending_38[2] != 0) {
        return;
    }

    delta = shared->stride_delta_34;
    stride = shared->stride_30;
    if (delta >= 0) {
        prospective = (int32_t)((uint32_t)stride + (uint32_t)(delta >> 12));
        if (prospective > 35) {
            goto cap_stride;
        }
    } else {
        delta = (int32_t)(0u - (uint32_t)delta);
        prospective = (int32_t)((uint32_t)stride - (uint32_t)(delta >> 12));
        if (prospective > 35) {
            goto cap_stride;
        }
    }
    goto check_lower_bound;
cap_stride:
    {
        delta = shared->stride_delta_34;
        if (delta >= 0) {
            shared->stride_30 = (int32_t)(35u - (uint32_t)(delta >> 12));
        } else {
            delta = (int32_t)(0u - (uint32_t)delta);
            shared->stride_30 = (int32_t)((uint32_t)(delta >> 12) + 35u);
        }
    }
    goto produce;
check_lower_bound:
    {
        delta = shared->stride_delta_34;
        stride = shared->stride_30;
        if (delta >= 0) {
            prospective = (int32_t)((uint32_t)stride + (uint32_t)(delta >> 12));
            if (prospective < 0) {
                goto adjust_lower_bound;
            }
            goto produce;
        } else {
            delta = (int32_t)(0u - (uint32_t)delta);
            prospective = (int32_t)((uint32_t)stride - (uint32_t)(delta >> 12));
            if (prospective >= 0) {
                goto produce;
            }
        }
adjust_lower_bound:
        if (shared->stride_30 >= 0) {
            delta = shared->stride_delta_34;
            if (delta >= 0) {
                shared->stride_30 = (int32_t)(0u - (uint32_t)(delta >> 12));
            } else {
                delta = (int32_t)(0u - (uint32_t)delta);
                shared->stride_30 = delta >> 12;
            }
        }
    }
produce:
    func_80022944(actor);
}
