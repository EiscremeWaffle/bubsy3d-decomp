#ifndef BUBSY3D_SECONDARY_POSITION_H
#define BUBSY3D_SECONDARY_POSITION_H

#include "component_bounds.h"
#include "component_rotation.h"
#include "player_model.h"

typedef union SecondaryPositionOwner {
    ComponentRecordOwner records;
    PlayerActorSequenceView actor;
    struct {
#if UINTPTR_MAX == 0xFFFFFFFFu
        uint8_t unknown_00[0x14];
#else
        /* Keep the native sequence pointer layout intact on 64-bit hosts. */
        PlayerActorSequenceView actor;
#endif
        ComponentRotationMatrix *component_14;
    } position;
} SecondaryPositionOwner;

#if UINTPTR_MAX == 0xFFFFFFFFu && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(SecondaryPositionOwner, position.component_14) == 0x14, "component pointer offset");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(SecondaryPositionOwner, records.table_18) ==
               offsetof(SecondaryPositionOwner, actor.sequence_18), "sequence pointer overlay");
#endif

int32_t func_80058E7C(void *object, void *basis, int32_t output[3]);

#endif
