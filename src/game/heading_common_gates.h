#ifndef BUBSY3D_HEADING_COMMON_GATES_H
#define BUBSY3D_HEADING_COMMON_GATES_H

#include "bubsy_grounded_state.h"
#include "game_transition.h"
#include <stddef.h>
#include <stdint.h>

typedef struct HeadingComponentView {
    uint8_t unknown_00[0x14];
    void *transform_14;
} HeadingComponentView;

typedef struct HeadingSharedView {
    BubsyGroundedShared shared;
    uint8_t unknown_44[0x38];
    int32_t state_7C;
} HeadingSharedView;

typedef struct HeadingActorView {
    BubsyGroundedActor actor;
    uint8_t state_2C;
} HeadingActorView;

typedef struct HeadingCommonDecision {
    BubsyGroundedActor *actor;
    void *transform;
    BubsyGroundedShared *shared;
    void *context;
    uint8_t proceed;
} HeadingCommonDecision;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(HeadingSharedView, state_7C) == 0x7C, "shared state offset");
#if UINTPTR_MAX == 0xFFFFFFFFu
_Static_assert(offsetof(HeadingComponentView, transform_14) == 0x14, "component transform offset");
_Static_assert(offsetof(HeadingActorView, state_2C) == 0x2C, "actor state offset");
#endif
#endif

extern uint8_t g_runtime_mode;
extern uint8_t g_runtime_state_61;
extern uint8_t g_deceleration_active_64B8;
extern uint8_t g_deceleration_state_6464;
extern uint8_t g_deceleration_stop_649C;
extern uint8_t g_stride_override;
extern uint8_t g_heading_status_6478;

HeadingCommonDecision heading_common_gates(GameTransitionPayload *event, uint8_t counter_changed);

#endif
