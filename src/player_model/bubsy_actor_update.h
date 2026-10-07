#ifndef BUBSY3D_BUBSY_ACTOR_UPDATE_H
#define BUBSY3D_BUBSY_ACTOR_UPDATE_H

#include <stddef.h>
#include <stdint.h>

typedef struct BubsyActorUpdateActorView {
    uint8_t unknown_00_to_0F[0x10];
    uint32_t update_state_10;
} BubsyActorUpdateActorView;

_Static_assert(offsetof(BubsyActorUpdateActorView, update_state_10) == 0x10, "actor update state offset must match the executable");

typedef struct BubsyActorUpdateEnvironment {
    int16_t update_gate_6454;
    uint8_t update_mode_647A;
    uint8_t actor_event_guard_6460;
    int32_t sequence_counter_38C;
} BubsyActorUpdateEnvironment;

typedef enum BubsyActorUpdateEntryResult {
    BUBSY_ACTOR_UPDATE_SKIP = 0,
    BUBSY_ACTOR_UPDATE_CONTINUE = 1,
} BubsyActorUpdateEntryResult;

BubsyActorUpdateEntryResult bubsy_actor_update_entry_gate(
    BubsyActorUpdateActorView *actor,
    uint8_t update_mode,
    BubsyActorUpdateEnvironment *environment,
    uint8_t *local_state_24
);

#endif