#include <stdint.h>

extern int32_t g_unknown_800BDE74;
extern int32_t g_unknown_800BE3EC;

void func_8001AE28(void) {
    if (g_unknown_800BDE74 != 0x7fff) {
        g_unknown_800BDE74 = 0x7fff;
    }
}

int32_t func_80061A0C(uint8_t value) {
    g_unknown_800BE3EC = value == 1 ? 2 : 1;
    return 1;
}
