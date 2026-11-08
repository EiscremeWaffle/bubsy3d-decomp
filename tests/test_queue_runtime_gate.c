#include "queue_update.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

uint8_t g_unknown_800BE023;
uint8_t g_unknown_800BE024;
uint8_t g_unknown_80186174;
ComponentRotationMatrix g_unknown_80186154;

static int calls[8];
static unsigned int call_count;
static void *resolved_component;

enum {
    CALL_SETUP,
    CALL_GET_COMPONENT,
    CALL_ENABLE,
    CALL_CLEAR,
    CALL_APPLY,
    CALL_FINALIZE,
    CALL_FINISH
};

static void record_call(int call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

void func_8003AAF0(void) {
    record_call(CALL_SETUP);
}

void func_800516F8(void **output) {
    record_call(CALL_GET_COMPONENT);
    *output = resolved_component;
}

void func_800334C4(void) {
    record_call(CALL_ENABLE);
}

void func_80033CF4(void) {
    record_call(CALL_CLEAR);
}

void func_8005476C(void *source, void *destination) {
    assert(source == resolved_component);
    assert(destination == &g_unknown_80186154);
    record_call(CALL_APPLY);
}

void func_80032EFC(void) {
    record_call(CALL_FINALIZE);
}

void func_80032DF4(void) {
    record_call(CALL_FINISH);
}

static void reset_calls(void) {
    unsigned int i;
    call_count = 0;
    for (i = 0; i < sizeof(calls) / sizeof(calls[0]); ++i) {
        calls[i] = -1;
    }
}

static void expect_calls(const int *expected, unsigned int count) {
    unsigned int i;
    assert(call_count == count);
    for (i = 0; i < count; ++i) {
        assert(calls[i] == expected[i]);
    }
}

int main(void) {
    static const int mode_zero_enabled[] = {
        CALL_SETUP, CALL_GET_COMPONENT, CALL_ENABLE, CALL_CLEAR,
        CALL_APPLY, CALL_FINALIZE, CALL_FINISH
    };
    static const int mode_zero_disabled[] = {
        CALL_SETUP, CALL_GET_COMPONENT, CALL_CLEAR, CALL_APPLY,
        CALL_FINISH
    };
    static const int nonzero_mode[] = {
        CALL_SETUP, CALL_GET_COMPONENT, CALL_CLEAR, CALL_APPLY, CALL_FINISH
    };
    unsigned int flag;
    int data;

    resolved_component = &data;
    for (flag = 0; flag < 256; ++flag) {
        g_unknown_800BE024 = (uint8_t)flag;
        g_unknown_80186174 = 1;
        g_unknown_800BE023 = 1;
        reset_calls();
        assert(func_80032C8C(0) == 1);
        if (flag == 0) {
            expect_calls(NULL, 0);
        } else {
            expect_calls(mode_zero_enabled,
                         sizeof(mode_zero_enabled) / sizeof(mode_zero_enabled[0]));
        }
    }

    g_unknown_800BE024 = 1;
    g_unknown_80186174 = 0;
    g_unknown_800BE023 = 0;
    reset_calls();
    assert(func_80032C8C(0) == 1);
    expect_calls(mode_zero_disabled,
                 sizeof(mode_zero_disabled) / sizeof(mode_zero_disabled[0]));

    g_unknown_80186174 = 1;
    g_unknown_800BE023 = 1;
    reset_calls();
    assert(func_80032C8C(1) == 1);
    expect_calls(nonzero_mode, sizeof(nonzero_mode) / sizeof(nonzero_mode[0]));
    reset_calls();
    assert(func_80032C8C(256) == 1);
    expect_calls(mode_zero_enabled,
                 sizeof(mode_zero_enabled) / sizeof(mode_zero_enabled[0]));

    puts("Queue runtime gate: all global flag values, mode narrowing and callback order passed.");
    return 0;
}
