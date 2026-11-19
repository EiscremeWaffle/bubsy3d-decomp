#ifndef BUBSY3D_TICK_HISTORY_H
#define BUBSY3D_TICK_HISTORY_H

#include <stdint.h>

extern volatile uint32_t g_tick_window_reference;
extern volatile uint32_t g_tick_window_snapshot;
extern volatile uint32_t g_tick_window_count;
extern volatile uint32_t g_tick_total_count;
extern volatile uint32_t g_tick_history_index;
extern uint32_t g_tick_history[];

uint8_t func_8004F688(void);

#endif
