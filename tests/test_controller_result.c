#include "actor_context.h"
#include "controller_callback_helpers.h"
#include "controller_result.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

_Static_assert(offsetof(ControllerResultContext, mode_0C) == 0x0C, "mode offset");
_Static_assert(offsetof(ControllerResultContext, selection_14) == 0x14, "selection offset");

int16_t g_result_previous_selection;
const char g_result_player_one_text[] = "one";
const char g_result_player_two_text[] = "two";
uint32_t g_unknown_8009F58C;

static ControllerResultContext context;
static uint8_t runtime_mode;
static int change_selection;
static int16_t replacement_selection;
static uint32_t global_after_cleanup;
static const char *selected_text;
static unsigned calls[16], call_count;
static const int32_t duration = (int32_t)0x89ABCDEFU;

static void record(unsigned call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

void *func_80026648(void *argument0, void *argument1) {
    record(1);
    assert(argument0 == &context);
    assert(argument1 == &runtime_mode);
    return &context;
}

void func_8002DF24(const char *text) {
    record(2);
    selected_text = text;
    if (change_selection) {
        context.selection_14 = replacement_selection;
    }
}

void func_8002C604(int32_t value) {
    assert(value == 6 || value == 7);
    record((unsigned)value);
}

int32_t func_8002BD0C(int32_t value) {
    record(8);
    assert(value == 17);
    return duration;
}

void func_8002CA88(int32_t value, int32_t argument) {
    if (value == 17) {
        record(9);
        assert(argument == duration);
    } else {
        assert((value == 5 || value == 15) && argument == -1);
        record((unsigned)value + 16U);
    }
}

void func_8002E1E0(int32_t value) {
    record(10);
    assert(value == 0);
}

void func_8002E348(int32_t value) {
    record(11);
    assert(value == 0);
}

void func_8002BC7C(void) {
    record(12);
}

void func_8002C010(void) {
    record(13);
}

void func_8002C1F0(void) {
    record(14);
    g_unknown_8009F58C = global_after_cleanup;
}

uint8_t func_800283E0(void) {
    record(15);
    return runtime_mode;
}

void func_8002BC9C(void) {
    record(16);
}

static void reset(void) {
    memset(&context, 0, sizeof(context));
    g_result_previous_selection = 0;
    g_unknown_8009F58C = 0;
    runtime_mode = 0;
    change_selection = 0;
    selected_text = NULL;
    call_count = 0;
    global_after_cleanup = 0;
}

static void test_result_mode(void) {
    static const int16_t selections[] = {INT16_MIN, -1, 0, 1, INT16_MAX};
    static const unsigned same_calls[] = {1, 6, 7, 8, 9, 10, 11};
    static const unsigned changed_calls[] = {1, 2, 6, 7, 8, 9, 10, 11};
    unsigned i, changed;

    for (i = 0; i < sizeof(selections) / sizeof(selections[0]); i++) {
        for (changed = 0; changed < 2; changed++) {
            const unsigned *expected = changed ? changed_calls : same_calls;
            size_t expected_size = changed ? sizeof(changed_calls) : sizeof(same_calls);
            reset();
            context.mode_0C = 1;
            context.selection_14 = selections[i];
            g_result_previous_selection = changed ? (int16_t)(selections[i] ^ 1) : selections[i];
            change_selection = 1;
            replacement_selection = (int16_t)(selections[i] ^ 2);
            func_8002C370(&context, &runtime_mode);
            assert(call_count == expected_size / sizeof(expected[0]));
            assert(memcmp(calls, expected, expected_size) == 0);
            assert(g_result_previous_selection == (changed ? replacement_selection : selections[i]));
            assert(selected_text == (changed
                ? (selections[i] == 0 ? g_result_player_one_text : g_result_player_two_text)
                : NULL));
        }
    }
}

static void test_other_modes(void) {
    static const uint8_t modes[] = {0, 2, 255};
    static const uint8_t runtimes[] = {0, 1, 2, 255};
    unsigned i, j, enabled;

    for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
        for (j = 0; j < sizeof(runtimes) / sizeof(runtimes[0]); j++) {
            for (enabled = 0; enabled < 2; enabled++) {
                unsigned expected[] = {1, 12, 13, 14, 0, 0, 0, 0};
                unsigned length = 4;
                reset();
                context.mode_0C = modes[i];
                runtime_mode = runtimes[j];
                g_unknown_8009F58C = enabled ? 0 : 1;
                global_after_cleanup = enabled ? 0x80000000U : 0;
                if (enabled) {
                    expected[length++] = 21;
                }
                expected[length++] = 15;
                if (runtime_mode == 1) {
                    expected[length++] = 16;
                    expected[length++] = 31;
                }
                func_8002C370(&context, &runtime_mode);
                assert(call_count == length);
                assert(memcmp(calls, expected, length * sizeof(expected[0])) == 0);
            }
        }
    }
}

int main(void) {
    test_result_mode();
    test_other_modes();
    puts("controller result tests passed");
    return 0;
}
