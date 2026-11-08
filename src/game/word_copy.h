#ifndef BUBSY3D_WORD_COPY_H
#define BUBSY3D_WORD_COPY_H

#include <stdint.h>

/* Valid aligned arrays and a nonzero count are required; overlaps copy forwards. */
int32_t func_80010220(uint32_t *output, const uint32_t *input, uint32_t count);

#endif
