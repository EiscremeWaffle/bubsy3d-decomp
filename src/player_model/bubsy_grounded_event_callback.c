#include "bubsy_grounded_state.h"
#include "bubsy_actor_state_update.h"
#include "grounded_event_records.h"
#include "level_records.h"

extern int16_t g_grounded_counter;
extern void *g_grounded_event;
extern void *func_80040FC0(int32_t index);

void func_8004B2A4(
    BubsyGroundedActor *unused_actor,
    BubsyGroundedState *unused_state,
    void *unused_context,
    int32_t unused_counter
) {
    BubsyGroundedActor *actor;

    (void)unused_actor;
    (void)unused_state;
    (void)unused_context;
    (void)unused_counter;

    g_grounded_counter = (int16_t)(g_grounded_counter - 2);
    if (g_grounded_counter < 0) {
        g_grounded_counter = 0;
    }
    if (g_grounded_counter == 0) {
        if (g_level_record_count <= 0) {
            actor = func_80040FC0(0);
        } else {
            actor = g_level_records->object_1C;
        }
        func_8004FFEC((GroundedEventRecord *)g_grounded_event);
        g_grounded_event = 0;
        actor->state_14 = 0;
        actor->state_10 &= ~0x200u;
        actor->state_10 &= ~0x20u;
        func_8003539C(actor, 1);
    }
}
