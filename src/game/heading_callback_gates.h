#ifndef BUBSY3D_HEADING_CALLBACK_GATES_H
#define BUBSY3D_HEADING_CALLBACK_GATES_H

#include <stdint.h>

extern volatile uint8_t g_grounded_case_event;
extern int16_t g_actor_environment;
extern uint8_t g_heading_context_gate;
extern const char g_heading_gate_condition[];
extern const char g_heading_gate_source_path[];

void func_8002E7D8(void);
void func_8002E7FC(void);
uint32_t func_8002E824(void);
uint8_t func_8002B620(void);

#endif
