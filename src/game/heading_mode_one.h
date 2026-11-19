#ifndef BUBSY3D_HEADING_MODE_ONE_H
#define BUBSY3D_HEADING_MODE_ONE_H

#include "bubsy_grounded_state.h"
#include <stdint.h>

#ifdef BUBSY3D_TEST_REAL_TIMED_RECORDS
#include "heading_timed_records.h"
/* The PSX budget aliases record 15's duration; native pointer layout differs. */
#define g_unknown_8009F7BC (*(volatile int32_t *)&g_heading_timed_records[15].duration_30)
#else
extern volatile int32_t g_unknown_8009F7BC;
#endif

typedef struct HeadingModeCameraSeed {
    uint8_t bytes[4];
} HeadingModeCameraSeed;

extern void *g_mode_one_handle;
extern int32_t g_interaction_previous_phase;
extern void *g_heading_mode_camera_component;
extern uint32_t g_heading_mode_interval;
extern uint32_t g_heading_mode_previous_tick;
extern HeadingModeCameraSeed g_heading_mode_camera_seed;
extern uint8_t g_heading_mode_direction;
extern uint32_t g_heading_mode_counter;
extern int32_t g_heading_mode_oscillation;
extern const char g_heading_mode_allocation_condition[];
extern const char g_heading_mode_source_path[];

void func_800395E8(BubsyGroundedActor *actor, void *transform);

#endif
