#include "controller_cycle_warp.h"
#include "game_transition.h"
#include "heading_common_gates.h"
#include "sequence_queries.h"

extern void *func_80028764(int32_t index);
extern void func_80028E80(void);
extern int32_t func_8002269C(void *actor, uint8_t selector);
extern void func_8003539C(void *actor, int32_t mode);
extern uint8_t g_cycle_unknown_6460;

void func_80038368(void) {
    ControllerCycleActor *actor = func_80028764(0);

    func_80028E80();
    if (g_cycle_mode_flag == 0) func_8001C700(4, 15);
    else func_8001C700(4, 14);
    g_heading_lock_6479 = 2;
    func_8001B558(0x2e, 4, 0, 0, 0, 0, 0, 0);
    func_8002269C(actor, 0x10);
    actor->flags_10 = 0;
    actor->unknown_01[0] = 2;
    g_deceleration_stop_649C = 0;
    g_cycle_unknown_6460 = 0;
    g_deceleration_state_6464 = 0;
    g_deceleration_active_64B8 = 0;
    func_8003539C(actor, 1);
}
