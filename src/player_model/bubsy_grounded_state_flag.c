#include "actor_context.h"
#include "bubsy_grounded_state.h"

int32_t func_8004B264(BubsyGroundedActor *actor, void *context) {
    uint8_t *state_table = func_80026648(actor, context);

    state_table += actor->state_01;
    return state_table[0x40] == 0;
}
