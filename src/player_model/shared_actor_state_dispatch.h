#ifndef BUBSY3D_SHARED_ACTOR_STATE_DISPATCH_H
#define BUBSY3D_SHARED_ACTOR_STATE_DISPATCH_H

#include "player_model.h"

typedef struct SharedActorDispatchTarget {
    uint8_t unknown_00_to_03[4];
    uint32_t flags_04;
    uint8_t unknown_08_to_0F[8];
    uint32_t state_10;
    uint8_t unknown_14_to_1B[8];
    void *component_1C;
    uint8_t unknown_20_to_27[8];
    PlayerActorSharedStateView *shared_28;
} SharedActorDispatchTarget;

#if UINTPTR_MAX == 0xFFFFFFFFu
_Static_assert(offsetof(SharedActorDispatchTarget, component_1C) == 0x1C, "actor component offset must match executable");
_Static_assert(offsetof(SharedActorDispatchTarget, shared_28) == 0x28, "shared state pointer offset must match executable");
#endif

void shared_actor_state_dispatch(SharedActorDispatchTarget *actor, void *context, int32_t mode);

#endif
