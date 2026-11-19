#ifndef BUBSY3D_SECONDARY_CONTROLLER_HELPERS_H
#define BUBSY3D_SECONDARY_CONTROLLER_HELPERS_H

#include "grounded_event_records.h"
#include <stdint.h>

extern GroundedEventRecord *volatile g_secondary_controller_event_8e4;
extern volatile uint8_t g_secondary_controller_flag_505;
extern volatile uint8_t g_secondary_controller_flag_507;
extern uint8_t g_secondary_controller_display[];

void func_800493CC(void);
void func_800493F8(void *argument0, void *argument1);

#endif
