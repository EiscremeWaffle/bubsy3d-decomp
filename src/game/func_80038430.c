#include "controller_cycle_warp.h"
#include "game_transition.h"
#include "sequence_queries.h"

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

extern void *func_80028764(int32_t heading);
extern int32_t func_800226C0(void *actor, uint8_t flags);
extern int32_t func_80054AF0(const int16_t *component, int32_t *direction);
extern int32_t func_80054C7C(
    ComponentTransformView *component, const ComponentBasisView *basis, const int32_t *position
);
extern void func_8002E910(const int32_t position[3]);
extern uint32_t func_80044C1C(int32_t index);
extern void func_80044C04(int32_t index, uint32_t mode);
extern int32_t func_800516F8(void **component);
extern int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t *position
);
extern int32_t func_80022B5C(const int32_t position[3], const int32_t other[3]);
extern void func_80038368(void);
extern void func_8002EB70(
    GameCyclicEntry *direction, int32_t distance, uint32_t height, int32_t duration,
    int32_t a4, void (*callback)(void), int32_t a6, int32_t a7
);
extern void func_80028FC8(void);
extern void func_8001C8A0(int32_t a, int32_t b);

void func_80038430(GameCyclicEntry *position) {
    GameCyclicEntry initial = g_cycle_initial_vector;
    GameCyclicEntry direction;
    GameCyclicEntry other;
    void *reference;
    ControllerCycleActor *actor = func_80028764(0);

    if (((actor->flags_10 & 0x2000) != 0 && g_heading_lock_6479 != 3)
        || (actor->flags_10 & 0x100000) != 0) {
        return;
    }
    if (g_heading_lock_6479 == 0) {
        actor->state_04 = 0;
        func_800226C0(actor, 0x10);
        func_8001C700(4, 3);
        if (g_cycle_mode_flag == 0) {
            func_8001C700(5, 8);
        } else {
            actor->state_00 = 0;
            func_8001C700(5, 5);
        }
        g_cycle_saved_position[0] = position->words[0];
        g_cycle_saved_position[1] = position->words[1];
        g_cycle_saved_position[2] = position->words[2];
        g_heading_lock_6479 = 1;
        func_8001B558(0x2e, 4, 0, 0, 0, 0, 0, 0);
    } else if (g_heading_lock_6479 == 3) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register GameCyclicEntry *direction_pointer __asm__("$20") = &direction;
#else
        GameCyclicEntry *direction_pointer = &direction;
#endif
        void *transform = actor->component_1c->transform_14;
        ControllerCycleShared *shared = actor->shared_28;
        int32_t duration;

        func_80054AF0(transform, (int32_t *)direction_pointer->words);
        direction.words[0] = -direction.words[0];
        direction.words[1] = -direction.words[1];
        direction.words[2] = -direction.words[2];
        shared->vector_38 = initial;
        shared->vector_78 = initial;
        shared->vector_98 = initial;
        shared->position_68[0] = position->words[0];
        shared->position_68[1] = position->words[1];
        shared->position_68[2] = position->words[2];
        func_80054C7C(transform, 0, (const int32_t *)position->words);
        position->words[1] += g_cycle_height_offset;
        func_8002E910((const int32_t *)position->words);
        position->words[1] -= g_cycle_height_offset;
        g_cycle_saved_effect = (uint16_t)func_80044C1C(1);
        func_80044C04(1, 12);
        func_800516F8(&reference);
        func_80054B20(reference, 0, (int32_t *)other.words);
        duration = func_80022B5C((const int32_t *)position->words, (const int32_t *)other.words) / 160;
        if (duration <= 0) duration = 1;
        if (g_cycle_level_id == 11 || g_cycle_level_id == 13 || g_cycle_level_id == 15) {
            func_8002EB70(&direction, g_cycle_motion_base,
                          position->words[1] + g_cycle_motion_scale * 2,
                          duration, 0, func_80038368, 0, 0);
        } else if (g_cycle_level_id == 10 || g_cycle_level_id == 12 || g_cycle_level_id == 14) {
            func_8002EB70(&direction, g_cycle_motion_base,
                          position->words[1] + g_cycle_motion_scale * 4,
                          duration, 0, func_80038368, 0, 0);
        } else {
            func_8002EB70(direction_pointer, g_cycle_motion_base,
                          position->words[1] + g_cycle_motion_scale * 10,
                          duration, 0, func_80038368, 0, 0);
        }
        func_8001B558(0x13, 4, (uintptr_t)(uint32_t)actor->heading_0c, actor->heading_0c, 0, 0, 0, 0);
        func_80028FC8();
        func_8001C8A0(0, 0);
        if (g_cycle_mode_flag == 0) func_8001C6BC(4, 15, -1);
        else func_8001C6BC(4, 14, -1);
    }
}
