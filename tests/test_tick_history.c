#include "controller_input.h"
#include "tick_history.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int32_t g_input_stream_sample;
volatile uint32_t g_tick_window_reference;
volatile uint32_t g_tick_window_snapshot;
volatile uint32_t g_tick_window_count;
volatile uint32_t g_tick_total_count;
volatile uint32_t g_tick_history_index;
uint32_t g_tick_history[128];

static unsigned calls;
static uint32_t backend_result;
static uint32_t expected_reference;
static uint32_t expected_snapshot;
static uint32_t expected_count;
static uint32_t expected_total;
static uint32_t expected_index;
static uint32_t expected_sample;

void func_8006C8C0(int32_t mode) {
    assert(calls++ == 0);
    assert(mode == 1);
    assert(g_tick_window_reference == expected_reference);
    assert(g_tick_window_snapshot == expected_snapshot);
    assert(g_tick_window_count == expected_count);
    assert(g_tick_total_count == expected_total);
    assert(g_tick_history_index == expected_index);
    assert(g_tick_history[expected_index] == expected_sample);
    g_tick_window_count = 0xABCDEF01U;
}

void func_800720FC(void) {
    assert(calls++ == 1);
    assert(g_tick_window_count == 0xABCDEF01U);
    g_tick_window_snapshot = 0xF1234567U;
}

uint32_t func_80051360(void) {
    assert(calls++ == 2);
    assert(g_tick_window_snapshot == 0xF1234567U);
    return backend_result;
}

static void check_case(
    uint32_t index, uint32_t count, uint32_t total,
    uint32_t reference, uint32_t sample, uint32_t result
) {
    unsigned i;
    uint32_t incremented = count + 1U;
    uint32_t elapsed = sample - reference;

    memset(g_tick_history, 0xA5, sizeof(g_tick_history));
    g_tick_window_reference = reference;
    g_tick_window_snapshot = 0x98765432U;
    g_tick_window_count = count;
    g_tick_total_count = total;
    g_tick_history_index = index;
    g_input_stream_sample = (int32_t)sample;
    backend_result = result;
    calls = 0;

    expected_index = index + 1U;
    if (expected_index == 100U) {
        expected_index = 0;
    }
    assert(expected_index < sizeof(g_tick_history) / sizeof(g_tick_history[0]));
    expected_sample = sample;
    expected_total = total + 1U;
    expected_reference = elapsed >= 60U ? sample : reference;
    expected_snapshot = elapsed >= 60U ? incremented : 0x98765432U;
    expected_count = elapsed >= 60U ? 0 : incremented;

    assert(func_8004F688() == (uint8_t)result);
    assert(calls == 3);
    assert(g_tick_window_count == 0xABCDEF01U);
    assert(g_tick_window_snapshot == 0xF1234567U);
    for (i = 0; i < sizeof(g_tick_history) / sizeof(g_tick_history[0]); i++) {
        assert(g_tick_history[i] == (i == expected_index ? sample : 0xA5A5A5A5U));
    }
}

int main(void) {
    check_case(0, 0, 0, 100, 159, 0);
    check_case(98, 3, 5, 100, 160, 1);
    check_case(99, 7, 9, 100, 161, 0x1234ABCDU);
    check_case(100, UINT32_MAX, UINT32_MAX, UINT32_MAX - 30U, 29, 0x100);
    check_case(UINT32_MAX, 5, 12, 0, UINT32_MAX, 0xFF);
    check_case(12, 0, UINT32_MAX, 0x80000000U, 0x7FFFFFFFU, 0x10001U);
    puts("tick history tests passed");
    return 0;
}
