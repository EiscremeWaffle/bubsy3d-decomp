#ifndef BUBSY3D_TIMEOUT_RESOURCES_H
#define BUBSY3D_TIMEOUT_RESOURCES_H

#include <stdint.h>

extern uint8_t g_timeout_resource_active;
extern int32_t g_timeout_resource_protected;
extern void *g_timeout_resource_a;
extern void *g_timeout_resource_b;
extern void *g_timeout_resource_c;
extern void *g_timeout_resource_d;
extern uint8_t g_timeout_resource_state[];

int32_t func_80062890(void);

#endif
