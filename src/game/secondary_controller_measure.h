#ifndef BUBSY3D_SECONDARY_CONTROLLER_MEASURE_H
#define BUBSY3D_SECONDARY_CONTROLLER_MEASURE_H

#include "controller_bitmap_gate.h"
#include <stdint.h>

typedef struct SecondaryControllerMeasure {
    int32_t target[3];
    int32_t position[3];
    int32_t direction[3];
    int32_t scaled[3];
    int32_t distance;
    int32_t adjusted_distance;
    int32_t eligible;
} SecondaryControllerMeasure;

/*
 * Bounded model of 0x80049B08..0x80049CB8, not a byte match.
 * The output storage must not overlap the component or global basis.
 */
int32_t secondary_controller_measure(const void *component, SecondaryControllerMeasure *output);

#endif
