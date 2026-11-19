#ifndef BUBSY3D_SQRT_BACKEND_H
#define BUBSY3D_SQRT_BACKEND_H

#include <stdint.h>

/* The preceding 320 halfwords are also addressable by negative inputs. */
extern int16_t g_sqrt_lookup_domain[512];

int32_t func_800764D0(int32_t value);
int32_t func_80076554(int32_t value);

#endif
