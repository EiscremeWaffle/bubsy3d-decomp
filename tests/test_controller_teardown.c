#include "game_transition.h"
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

typedef struct ControllerTeardownInitial {
    uint8_t bytes[4];
} ControllerTeardownInitial;

ControllerTeardownInitial g_unknown_800BE210 = {{1, 2, 3, 4}};
uint8_t g_unknown_800BE214;
uint32_t g_unknown_800BE218;
int16_t g_unknown_800BE21C;
int8_t g_unknown_800BE21E;
uint8_t g_unknown_800BE680;
extern void func_80047050(int32_t argument);

static unsigned calls[32];
static unsigned call_count;
static unsigned poll_count;
static unsigned polls_until_complete;
static int complete_in_update;
static uint8_t *pending_complete;

static void record(unsigned call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

void func_80045398(void) { assert(g_unknown_800BE214 == 0); record(1); }
void func_80027F40(void) { record(2); }
void func_80040634(void) { record(3); }
void func_8002C488(void) { record(4); }
void func_8005070C(void) { record(5); }
int32_t func_80050824(const uint8_t *initial) {
    assert(memcmp(initial, g_unknown_800BE210.bytes, 4) == 0);
    record(6);
    return 0;
}
int32_t func_8004F428(const uint8_t *initial) {
    assert(memcmp(initial, g_unknown_800BE210.bytes, 4) == 0);
    record(7);
    return 1;
}
int32_t func_8004F454(int32_t a, int32_t b, int32_t c) {
    assert(a == 0 && b == 0 && c == 0);
    record(8);
    return 1;
}
void func_80028E50(void) { record(9); }
int32_t func_8004EFD4(void) { record(10); return 0; }
void func_8004F474(int32_t value, void *initial, uint8_t *complete) {
    assert(value == -10 && memcmp(initial, g_unknown_800BE210.bytes, 4) == 0);
    ++poll_count;
    *complete = !complete_in_update && poll_count == polls_until_complete;
    pending_complete = complete;
    record(11);
}
int32_t func_800509A4(int32_t mode, void *argument, int32_t flag) {
    assert(mode == 1 && argument == 0 && flag == 1);
    if (complete_in_update && poll_count == polls_until_complete) {
        *pending_complete = 1;
    }
    record(12);
    return 0;
}
void func_8001AE28(void) { record(13); }
void func_8004FA18(int32_t width, int32_t height, void *initial) {
    assert(width == 0x200 && height == 0x1e0);
    assert(memcmp(initial, g_unknown_800BE210.bytes, 4) == 0);
    record(14);
}
int32_t func_80061A0C(uint8_t value) { assert(value == 0); record(15); return 1; }
int32_t func_80061A64(int32_t value) { assert(value == 30); record(16); return 0; }

static uint32_t word_at(const uint8_t *data, size_t offset) {
    uint32_t value;
    memcpy(&value, data + offset, sizeof(value));
    return value;
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    const uint8_t *event = argument_4;
    uint16_t second;
    memcpy(&second, event + 4, sizeof(second));
    assert(event_id == 4 && mode == 5 && flags == 0xffff && heading == 0xffff);
    assert(argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    assert(g_unknown_800BE680 == 0);
    assert(word_at(event, 0) == g_unknown_800BE218);
    assert(second == (uint16_t)g_unknown_800BE21C);
    assert(event[6] == (uint8_t)g_unknown_800BE21E);
    assert(word_at(event, 0x80) == 0x200 && word_at(event, 0x84) == 0xf0);
    assert(word_at(event, 0x88) == 0 && word_at(event, 0x8c) == 0);
    assert(word_at(event, 0x90) == UINT32_MAX);
    assert(event[0x94] == 0 && event[0x95] == 0 && event[0x96] == 0);
    assert(event[0x98] == 0xfe && event[0xa5] == 1 && event[0xa6] == 0);
    record(17);
}
uint32_t func_8006BB84(int32_t count) { assert(count == 0); record(18); return 0; }
void func_8004F688(void) { record(19); }

static void run(int32_t argument, unsigned expected_polls, int update_completes) {
    static const unsigned prefix[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    static const unsigned suffix[] = {13, 14, 15, 16, 17, 18, 19, 18, 19};
    unsigned index;
    call_count = 0;
    poll_count = 0;
    polls_until_complete = expected_polls;
    complete_in_update = update_completes;
    g_unknown_800BE214 = 1;
    g_unknown_800BE218 = 0x12345678;
    g_unknown_800BE21C = -2;
    g_unknown_800BE21E = -3;
    g_unknown_800BE680 = 9;
    func_80047050(argument);
    assert(g_unknown_800BE214 == 0 && poll_count == expected_polls);
    assert(call_count == 19 + 2 * expected_polls);
    assert(memcmp(calls, prefix, sizeof(prefix)) == 0);
    for (index = 0; index < expected_polls; ++index) {
        assert(calls[10 + 2 * index] == 11 && calls[11 + 2 * index] == 12);
    }
    assert(memcmp(calls + 10 + 2 * expected_polls, suffix, sizeof(suffix)) == 0);
}

int main(void) {
    g_unknown_800BE214 = 0;
    g_unknown_800BE680 = 9;
    func_80047050(1);
    assert(call_count == 0 && g_unknown_800BE680 == 9);
    run(0, 0, 0);
    run(0x100, 0, 0);
    run(-1, 2, 0);
    run(1, 2, 1);
    puts("controller teardown tests passed");
    return 0;
}
