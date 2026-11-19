#ifndef BUBSY3D_RESULT_DISPLAY_H
#define BUBSY3D_RESULT_DISPLAY_H

#include "grounded_event_records.h"
#include <stdint.h>

extern uint8_t g_result_display_phase;
extern int32_t g_result_display_scale;
extern int8_t g_result_display_tag;
extern GroundedEventRecord *g_result_display_event;

void func_8002DE34(void);
void func_8002DF24(const char *text);

#endif
