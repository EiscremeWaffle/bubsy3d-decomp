#ifndef BUBSY3D_CONTROLLER_CYCLE_WARP_H
#define BUBSY3D_CONTROLLER_CYCLE_WARP_H

#include "cyclic_entries.h"
#include <stddef.h>
#include <stdint.h>

typedef struct ControllerCycleComponent {
    uint8_t unknown_00[0x14];
    void *transform_14;
} ControllerCycleComponent;

typedef struct ControllerCycleShared {
    uint8_t unknown_00[0x38];
    GameCyclicEntry vector_38;
    uint8_t unknown_48[0x20];
    uint32_t position_68[3];
    uint8_t unknown_74[4];
    GameCyclicEntry vector_78;
    uint8_t unknown_88[0x10];
    GameCyclicEntry vector_98;
} ControllerCycleShared;

typedef struct ControllerCycleActor {
    uint8_t state_00;
    uint8_t unknown_01[3];
    int32_t state_04;
    uint8_t unknown_08[4];
    int32_t heading_0c;
    uint32_t flags_10;
    uint8_t unknown_14[8];
    ControllerCycleComponent *component_1c;
    uint8_t unknown_20[8];
    ControllerCycleShared *shared_28;
} ControllerCycleActor;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(ControllerCycleShared, position_68) == 0x68, "cycle position offset");
_Static_assert(offsetof(ControllerCycleShared, vector_98) == 0x98, "cycle vector offset");
#if UINTPTR_MAX == 0xFFFFFFFFu
_Static_assert(offsetof(ControllerCycleComponent, transform_14) == 0x14, "cycle transform offset");
_Static_assert(offsetof(ControllerCycleActor, component_1c) == 0x1c, "cycle component offset");
_Static_assert(offsetof(ControllerCycleActor, shared_28) == 0x28, "cycle shared offset");
#endif
#endif

extern GameCyclicEntry g_cycle_initial_vector;
extern uint32_t g_cycle_saved_position[3];
extern uint8_t g_heading_lock_6479;
extern uint8_t g_cycle_mode_flag;
extern uint16_t g_cycle_saved_effect;
extern int16_t g_cycle_level_id;
extern uint32_t g_cycle_height_offset, g_cycle_motion_scale;
extern int32_t g_cycle_motion_base;

void func_80038430(GameCyclicEntry *position);

#endif
