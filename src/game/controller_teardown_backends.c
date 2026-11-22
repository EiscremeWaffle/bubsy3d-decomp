#include "camera_slots.h"

extern uint8_t g_unknown_800BE59C[3];
extern uint8_t g_unknown_800BE5A0[3];
extern uint8_t g_unknown_800BE5E0[3];
extern uint8_t g_unknown_801D8A24;
extern void func_80067DE0(int32_t value);
extern void func_800678F8(int32_t value);
extern void func_80044C64(void);
extern void func_8005080C(void);

void func_80045398(void) {
    func_80067DE0(-1);
    func_800678F8(0);
    func_80044C64();
}

void func_80028E50(void) {
    if (g_unknown_801D8A24 == 1) {
        func_8005080C();
    }
}

int32_t func_8004EFD4(void) {
    func_80072D14(0);
    g_camera_mode = 0;
    return 0;
}

int32_t func_8004F428(const uint8_t *input) {
    g_unknown_800BE59C[0] = input[0];
    g_unknown_800BE59C[1] = input[1];
    g_unknown_800BE59C[2] = input[2];
    return 1;
}

int32_t func_8004F454(int32_t x, int32_t y, int32_t z) {
    g_unknown_800BE5A0[0] = (uint8_t)(x >> 4);
    g_unknown_800BE5A0[1] = (uint8_t)(y >> 4);
    g_unknown_800BE5A0[2] = (uint8_t)(z >> 4);
    return 1;
}

int32_t func_80050824(const uint8_t *input) {
    g_unknown_800BE5E0[0] = input[0];
    g_unknown_800BE5E0[1] = input[1];
    g_unknown_800BE5E0[2] = input[2];
    return 0;
}
