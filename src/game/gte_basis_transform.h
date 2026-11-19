#ifndef BUBSY3D_GTE_BASIS_TRANSFORM_H
#define BUBSY3D_GTE_BASIS_TRANSFORM_H

#include <stdint.h>

/* The basis is a packed PS1 matrix; the output contains three signed words. */
int32_t *func_80077298(const volatile void *basis, const int32_t *input, int32_t *output);

#endif
