#ifndef BUBSY3D_HEADING_STATE_DISPATCH_H
#define BUBSY3D_HEADING_STATE_DISPATCH_H

#include "heading_common_gates.h"
#include <stddef.h>
#include <stdint.h>

typedef struct HeadingDispatchSharedView {
    BubsyGroundedShared shared;
    uint8_t unknown_44[4];
    int32_t vector_48[3];
    uint8_t unknown_54[0x24];
    int32_t vector_78[3];
    uint8_t unknown_84[0x14];
    int32_t vector_98[3];
} HeadingDispatchSharedView;

typedef struct HeadingDispatchLevelView {
    uint8_t unknown_00[0x2A2A];
    uint8_t flags_2A2A;
} HeadingDispatchLevelView;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(HeadingDispatchSharedView, vector_48) == 0x48, "heading vector 48 offset");
_Static_assert(offsetof(HeadingDispatchSharedView, vector_78) == 0x78, "heading vector 78 offset");
_Static_assert(offsetof(HeadingDispatchSharedView, vector_98) == 0x98, "heading vector 98 offset");
_Static_assert(offsetof(HeadingDispatchLevelView, flags_2A2A) == 0x2A2A, "heading level flags offset");
#endif

extern uint32_t g_heading_dispatch_stop_tick;
extern int16_t g_heading_saved_effect5;
extern int16_t g_heading_saved_effect15;
extern uint8_t g_heading_sequence_started;
extern uint16_t g_heading_context_6450;
extern uint16_t g_heading_context_6452;
extern int16_t g_heading_context_gate_6454;
extern int32_t g_heading_context_645C;
extern uint8_t g_heading_state_6462;
extern uint8_t g_heading_flag_647A;
extern uint8_t g_heading_lock_6479;
extern HeadingDispatchLevelView *g_heading_level_config;
extern const char g_heading_diagnostic_48[];
extern const char g_heading_resource_mode_zero[];
extern const char g_heading_resource_mode_other[];
extern const char g_heading_resource_condition[];
extern const char g_heading_resource_source[];

void heading_state_dispatch(GameTransitionPayload *event, const HeadingCommonDecision *decision);
void func_80036018(GameTransitionPayload *event);

#endif
