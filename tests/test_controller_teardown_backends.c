#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

uint8_t g_unknown_800BE59C[3];
uint8_t g_unknown_800BE5A0[3];
uint8_t g_unknown_800BE5E0[3];
uint8_t g_unknown_801D8A24;
uint8_t g_camera_mode;
int32_t g_unknown_800BDE74;
int32_t g_unknown_800BE3EC;

extern void func_8001AE28(void);
extern void func_80045398(void);
extern void func_80028E50(void);
extern int32_t func_8004EFD4(void);
extern int32_t func_8004F428(const uint8_t *input);
extern int32_t func_8004F454(int32_t x, int32_t y, int32_t z);
extern int32_t func_80050824(const uint8_t *input);
extern int32_t func_80061A0C(uint8_t value);

static unsigned calls[8];
static unsigned call_count;

static void record(unsigned call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

void func_80067DE0(int32_t value) { assert(value == -1); record(1); }
void func_800678F8(int32_t value) { assert(value == 0); record(2); }
void func_80044C64(void) { record(3); }
void func_8005080C(void) { record(4); }
int32_t func_80072D14(int32_t mode) {
    assert(mode == 0 && g_camera_mode == 9);
    g_camera_mode = 255;
    record(5);
    return -1;
}

int main(void) {
    static const uint8_t input[] = {0, 0x80, 0xff};
    static const uint8_t shifted[] = {0xfe, 0xff, 0};
    static const uint8_t shifted_again[] = {0xff, 0xff, 1};
    static const unsigned expected[] = {1, 2, 3};

    g_unknown_800BDE74 = -1;
    func_8001AE28();
    assert(g_unknown_800BDE74 == 0x7fff);
    func_8001AE28();
    assert(g_unknown_800BDE74 == 0x7fff);
    assert(func_80061A0C(0) == 1 && g_unknown_800BE3EC == 1);
    assert(func_80061A0C(1) == 1 && g_unknown_800BE3EC == 2);
    assert(func_80061A0C(255) == 1 && g_unknown_800BE3EC == 1);

    func_80045398();
    assert(call_count == 3 && memcmp(calls, expected, sizeof(expected)) == 0);

    call_count = 0;
    g_unknown_801D8A24 = 0;
    func_80028E50();
    g_unknown_801D8A24 = 2;
    func_80028E50();
    g_unknown_801D8A24 = 255;
    func_80028E50();
    assert(call_count == 0);
    g_unknown_801D8A24 = 1;
    func_80028E50();
    assert(call_count == 1 && calls[0] == 4);

    call_count = 0;
    g_camera_mode = 9;
    assert(func_8004EFD4() == 0);
    assert(call_count == 1 && calls[0] == 5 && g_camera_mode == 0);

    assert(func_8004F428(input) == 1);
    assert(memcmp(g_unknown_800BE59C, input, 3) == 0);
    assert(func_8004F428(g_unknown_800BE59C) == 1);
    assert(memcmp(g_unknown_800BE59C, input, 3) == 0);
    assert(func_80050824(input) == 0);
    assert(memcmp(g_unknown_800BE5E0, input, 3) == 0);
    assert(func_80050824(g_unknown_800BE5E0) == 0);
    assert(memcmp(g_unknown_800BE5E0, input, 3) == 0);
    assert(func_8004F454(-17, INT32_MAX, INT32_MIN) == 1);
    assert(memcmp(g_unknown_800BE5A0, shifted, 3) == 0);
    assert(func_8004F454(-1, -16, 16) == 1);
    assert(memcmp(g_unknown_800BE5A0, shifted_again, 3) == 0);

    puts("controller teardown backend tests passed");
    return 0;
}
