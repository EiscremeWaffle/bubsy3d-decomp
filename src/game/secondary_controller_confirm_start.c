#include "secondary_controller_confirm.h"
#include "heading_timed_records.h"
#include "secondary_controller_helpers.h"
#include "sequence_queries.h"

extern void func_8002E348(int32_t delta);
extern int32_t func_800516F8(void **output);

SecondaryControllerConfirmResult secondary_controller_confirm_start(ResultCounterContext *context) {
    void *reference;

    func_8002E348(-1);
    g_secondary_controller_flag_505 = 1;
    func_800516F8(&reference);
    g_secondary_controller_basis = *(const ComponentRotationMatrix *)reference;
    func_80055D94(&g_secondary_controller_confirm_position, 256, 240);
    func_8004FF50((void *)func_800493CC, (void *)(uintptr_t)60, &g_secondary_controller_confirm_event);
    context->unknown[0x0D] = 1;
    func_8002C708(6);
    func_8002C708(7);
    if (context->selection != 0) {
        func_8002BAD4(9);
        func_8002BAD4(10);
    } else {
        func_8002BAD4(8);
        func_8002BAD4(11);
    }
    func_8002CA88(5, 10);
    func_8001C6BC(6, 2, 1);
    return g_secondary_controller_flag_505 != 0 ? SECONDARY_CONFIRM_ACTIVE : SECONDARY_CONFIRM_EXIT;
}
