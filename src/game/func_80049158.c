#include "component_rotation.h"
#include "secondary_controller_helpers.h"

extern uint32_t g_secondary_controller_tick;
extern uint8_t g_secondary_controller_tick_result;
extern ComponentRotationMatrix g_secondary_controller_basis;

extern int32_t func_800516F8(void **output);
extern int32_t func_80032D7C(void);
extern uint32_t func_800439EC(void);
extern void func_80044268(void);
extern void func_8001B0A0(void);
extern int32_t func_80050534(void *payload);
extern int32_t func_80032C8C(int32_t mode);
extern int32_t func_80051678(void);
extern int32_t func_800509A4(int32_t mode, void *argument, int32_t flag);
extern int32_t func_80050650(void *payload);

void func_80049158(void *argument) {
    void *reference;
    ComponentRotationMatrix saved;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register void *saved_argument __asm__("$17") = argument;
    register int32_t flag __asm__("$16");
#else
    void *saved_argument = argument;
    int32_t flag;
#endif

    ++g_secondary_controller_tick;
    func_800516F8(&reference);
    func_80032D7C();
    saved = *(ComponentRotationMatrix *)reference;
    *(ComponentRotationMatrix *)reference = g_secondary_controller_basis;
    if (g_secondary_controller_tick & 1) {
        g_secondary_controller_tick_result = (uint8_t)func_800439EC();
    } else if (g_secondary_controller_tick_result == 0) {
        func_80044268();
    }
    flag = 1;
    func_8001B0A0();
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__("" : : "r"(flag));
#else
    (void)flag;
#endif
    if (g_secondary_controller_flag_507 == flag) {
        func_80050534(g_secondary_controller_display);
    }
    func_80032C8C(0);
    func_80051678();
    func_800509A4(1, saved_argument, 1);
    if (g_secondary_controller_flag_507 == flag) {
        func_80050650(g_secondary_controller_display);
    }
    func_80032D7C();
    *(ComponentRotationMatrix *)reference = saved;
    func_80032C8C(0);
}
