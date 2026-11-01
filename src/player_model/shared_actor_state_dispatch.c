#include "player_model.h"

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
typedef struct SharedActorDispatchTarget {
    uint8_t unknown_00_to_03[4];
    uint32_t flags_04;
    uint8_t unknown_08_to_0F[8];
    uint32_t state_10;
    uint8_t unknown_14_to_1B[8];
    void *component_1C;
    uint8_t unknown_20_to_27[8];
    PlayerActorSharedStateView *shared_28;
} SharedActorDispatchTarget;

_Static_assert(offsetof(SharedActorDispatchTarget, component_1C) == 0x1C, "actor component offset must match executable");
_Static_assert(offsetof(SharedActorDispatchTarget, shared_28) == 0x28, "shared state pointer offset must match executable");

extern int32_t func_80052ABC(void *component, int32_t *sequence_key_out);
extern void func_800546F4(void *context, void *plan);
extern void func_800547C8(void *context, void *plan);
extern void func_80022944(void *actor);
extern void func_800549E8(void *context, void *plan);

void shared_actor_state_dispatch(
    SharedActorDispatchTarget *actor,
    void *context,
    int32_t mode
) {
    register SharedActorDispatchTarget *actor_value __asm__("$18") = actor;
    register void *context_value __asm__("$19") = context;
    register int32_t mode_value __asm__("$17") = mode;
    register void *component_value __asm__("$4") = actor_value->component_1C;
    register PlayerActorSharedStateView *shared __asm__("$16") = actor_value->shared_28;
    register volatile uint8_t *runtime_globals __asm__("$28");
    register int32_t saved_progress_30 __asm__("$20");
    register int32_t saved_progress_34 __asm__("$21");
    int32_t sequence_key;
    uint32_t plan_storage[12];

    __asm__ volatile ("" : : "r"(actor_value));
    __asm__ volatile ("" : : "r"(context_value));
    __asm__ volatile ("" : : "r"(mode_value));
    func_80052ABC(component_value, &sequence_key);
    if (sequence_key == 0x2A7 || (actor_value->flags_04 & 0x1100u) != 0) {
        shared->progress_30 = 0;
        shared->progress_34 = 0;
        return;
    }

    saved_progress_34 = shared->progress_34;
    switch (mode_value) {
    case 0:
        goto mode_zero;
    case 1:
        goto mode_one;
    case 2:
    case 3:
        goto mode_two_or_three;
    default:
        goto epilogue;
    }

mode_zero:
    if ((actor_value->state_10 & 0x18000u) != 0) {
        goto epilogue;
    }
    if (shared->progress_30 < 5) {
        shared->progress_30 = 5;
    }
    if (shared->progress_30 >= 0x11) {
        shared->progress_30 = 0x11;
        shared->progress_34 = 0;
    } else {
        shared->progress_34 = 0x198C;
    }
    goto mode_callback;

mode_one:
    if ((actor_value->state_10 & 0x8000u) != 0) {
        goto epilogue;
    }
    if ((actor_value->state_10 & 0x10000u) != 0) {
        shared->progress_30 = -0x34;
    } else if (shared->progress_30 < -0x11) {
        shared->progress_34 = 0;
    } else {
        shared->progress_34 = -0x198C;
    }

mode_callback:
    func_80022944(actor_value);
    goto epilogue;

mode_two_or_three:
    if ((actor_value->state_10 & 0x10000u) == 0) {
        saved_progress_30 = shared->progress_30;
        plan_storage[10] = 0;
        plan_storage[8] = 0;
        func_800546F4(context_value, plan_storage);
        plan_storage[9] = mode_value == 3 ? 0x400 : -0x400;
        func_800547C8(context_value, plan_storage + 8);
        if ((actor_value->state_10 & 0x8000u) == 0) {
            shared->progress_30 = 0xE;
        } else if (runtime_globals[0x7EC] != 0) {
            shared->progress_30 = 6;
        } else {
            shared->progress_30 = 8;
        }
        func_80022944(actor_value);
        func_800549E8(context_value, plan_storage);
        shared->progress_30 = saved_progress_30;
    }

epilogue:
    shared->progress_34 = saved_progress_34;
    return;
}
#endif