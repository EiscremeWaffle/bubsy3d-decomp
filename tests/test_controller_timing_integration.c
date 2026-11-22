#include <stdint.h>
#include <stdio.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

extern uint32_t func_8006BB84(int32_t count);
volatile uint32_t *volatile g_unknown_800A436C;
volatile uint32_t *volatile g_unknown_800A4370;
volatile uint32_t g_unknown_800A4374;
volatile uint32_t g_unknown_800A4378;
volatile uint32_t g_unknown_800A5438;
const char g_unknown_80013EA8[] = "timing failure";
static volatile uint32_t status_word;
static volatile uint32_t tick_word;
static unsigned calls;

void func_80082E4C(const char *message) {
    assert(message == g_unknown_80013EA8);
    assert(calls++ == 0);
    g_unknown_800A5438 = 2;
    tick_word = 100;
}

void func_8006BB74(int32_t value) {
    assert(value == 0 && calls++ == 1);
}

void func_8006BD68(int32_t value, int32_t argument) {
    assert(value == 3 && argument == 0 && calls++ == 2);
}

int main(void) {
    g_unknown_800A436C = &status_word;
    g_unknown_800A4370 = &tick_word;
    status_word = 0;
    tick_word = 50;
    g_unknown_800A4374 = 46;
    g_unknown_800A4378 = 0;
    g_unknown_800A5438 = 0;

    assert(func_8006BB84(0) == 4);
    assert(calls == 3);
    assert(g_unknown_800A4378 == 2);
    assert(g_unknown_800A4374 == 100);
    puts("controller timing integration tests passed");
    return 0;
}
