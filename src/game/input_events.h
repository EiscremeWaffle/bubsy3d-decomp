#ifndef BUBSY3D_INPUT_EVENTS_H
#define BUBSY3D_INPUT_EVENTS_H

#include <stdint.h>

extern uint8_t g_event_runtime_byte;
extern uint8_t g_event_input_gate;
extern uint32_t g_event_input_gate_reference;
extern uint32_t g_input_sample_counter;
extern uint32_t g_input_gate_last_sample;
extern uint32_t global_0x800BDE98;
extern uint8_t g_timeout_event_id;
extern volatile uint32_t g_timeout_event_flags;
extern volatile int32_t g_timeout_event_heading;
extern uint8_t g_unknown_800BE1C2;

void func_8001BC08(void);
void func_8001BC28(void);
int32_t func_8001C04C(void);
int32_t func_80045278(void);
uint8_t func_8001BCE4(void);
uint8_t func_8001BCF0(void);
uint32_t func_8001BD00(void);
uint8_t func_80023A30(int32_t interval);
int32_t func_80050128(uint32_t *output);

#endif
