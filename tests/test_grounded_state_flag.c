#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bubsy_grounded_state.h"

static BubsyGroundedActor actor;
static uint8_t state_table[0x40 + 256];
static int context;
static uint8_t selected_state;
static unsigned int calls;

void *func_80026648(void *object, void *object_context) {
    assert(object == &actor);
    assert(object_context == &context);
    ++calls;
    actor.state_01 = selected_state;
    return state_table;
}

int main(void) {
    unsigned int state;
    unsigned int flag;

    for (state = 0; state < 256; ++state) {
        for (flag = 0; flag < 256; ++flag) {
            uint8_t previous_state = (uint8_t)(state + 1);

            memset(&actor, 0, sizeof(actor));
            memset(state_table, 0xA5, sizeof(state_table));
            actor.state_01 = previous_state;
            selected_state = (uint8_t)state;
            state_table[0x40 + previous_state] = flag == 0 ? 1 : 0;
            state_table[0x40 + state] = (uint8_t)flag;
            calls = 0;

            assert(func_8004B264(&actor, &context) == (flag == 0));
            assert(calls == 1);
            assert(actor.state_01 == state);
        }
    }

    puts("grounded state flag: 65536 cases passed");
    return 0;
}
