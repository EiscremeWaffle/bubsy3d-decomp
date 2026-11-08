#include "timeout_resources.h"

extern void func_80062750(void);
extern void func_8008714C(void);
extern void func_800968BC(int32_t value);
extern void func_80096AEC(int32_t value);
extern void func_80082EFC(void *allocation);
extern void func_800601D0(void *state);

int32_t func_80062890(void) {
    if (g_timeout_resource_active == 0) {
        return 0;
    }

    func_80062750();
    func_8008714C();
    func_800968BC(0);
    func_80096AEC(0);

    if (g_timeout_resource_protected == 0) {
        if (g_timeout_resource_a != 0) {
            func_80082EFC(g_timeout_resource_a);
            g_timeout_resource_a = 0;
        }
        if (g_timeout_resource_b != 0) {
            func_80082EFC(g_timeout_resource_b);
            g_timeout_resource_b = 0;
        }
        if (g_timeout_resource_c != 0) {
            func_80082EFC(g_timeout_resource_c);
            g_timeout_resource_c = 0;
        }
        if (g_timeout_resource_d != 0) {
            func_80082EFC(g_timeout_resource_d);
            g_timeout_resource_d = 0;
        }
    }

    func_800601D0(g_timeout_resource_state);
    g_timeout_resource_active = 0;
    return 0;
}
