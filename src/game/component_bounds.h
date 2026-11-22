#ifndef BUBSY3D_COMPONENT_BOUNDS_H
#define BUBSY3D_COMPONENT_BOUNDS_H

#include "component_records.h"

typedef struct ComponentBoundsVector {
    uint32_t x, y, z, pad;
} ComponentBoundsVector;

typedef union ComponentBounds {
    ComponentRecord record;
    uint32_t words[8];
    int32_t signed_words[8];
    struct {
        ComponentBoundsVector lower, upper;
    } vectors;
} ComponentBounds;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(ComponentBounds) == 0x20, "component bounds size");
_Static_assert(offsetof(ComponentBounds, vectors.lower.y) == 0x04, "lower Y offset");
_Static_assert(offsetof(ComponentBounds, vectors.upper.y) == 0x14, "upper Y offset");
#endif

int32_t func_80058C74(ComponentRecordOwner *object, ComponentBounds *bounds);

#endif
