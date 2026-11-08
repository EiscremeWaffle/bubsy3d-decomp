#include "timeout_resources.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

uint8_t g_timeout_resource_active;
int32_t g_timeout_resource_protected;
void *g_timeout_resource_a;
void *g_timeout_resource_b;
void *g_timeout_resource_c;
void *g_timeout_resource_d;
uint8_t g_timeout_resource_state[16];

enum CallKind { SHUTDOWN_62750, SHUTDOWN_8714C, SHUTDOWN_968BC, SHUTDOWN_96AEC, FREE, STATE };
typedef struct Call {
    enum CallKind kind;
    void *argument;
} Call;

static Call calls[9];
static size_t call_count;
static uint8_t allocations[6];
static int mutate_on_first_free;
static int unprotect_on_shutdown;

static void record(enum CallKind kind, void *argument) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count].kind = kind;
    calls[call_count].argument = argument;
    ++call_count;
}

void func_80062750(void) {
    record(SHUTDOWN_62750, 0);
    if (unprotect_on_shutdown) {
        g_timeout_resource_protected = 0;
    }
}

void func_8008714C(void) {
    record(SHUTDOWN_8714C, 0);
}

void func_800968BC(int32_t value) {
    record(SHUTDOWN_968BC, 0);
    assert(value == 0);
}

void func_80096AEC(int32_t value) {
    record(SHUTDOWN_96AEC, 0);
    assert(value == 0);
}

void func_80082EFC(void *allocation) {
    record(FREE, allocation);
    if (mutate_on_first_free) {
        mutate_on_first_free = 0;
        g_timeout_resource_protected = 1;
        g_timeout_resource_a = &allocations[4];
        g_timeout_resource_b = &allocations[5];
        g_timeout_resource_c = 0;
    }
}

void func_800601D0(void *state) {
    record(STATE, state);
    assert(state == g_timeout_resource_state);
    assert(g_timeout_resource_active != 0);
    g_timeout_resource_active = 0x77;
}

static void initialize(uint8_t active, int32_t protected_value, unsigned int mask) {
    g_timeout_resource_active = active;
    g_timeout_resource_protected = protected_value;
    g_timeout_resource_a = (mask & 1) ? &allocations[0] : 0;
    g_timeout_resource_b = (mask & 2) ? &allocations[1] : 0;
    g_timeout_resource_c = (mask & 4) ? &allocations[2] : 0;
    g_timeout_resource_d = (mask & 8) ? &allocations[3] : 0;
    call_count = 0;
    mutate_on_first_free = 0;
    unprotect_on_shutdown = 0;
}

static void expect_call(size_t *next, enum CallKind kind, void *argument) {
    assert(*next < call_count);
    assert(calls[*next].kind == kind && calls[*next].argument == argument);
    ++*next;
}

static void test_resource_combinations(void) {
    static const uint8_t active_values[] = {0, 1, 2, 255};
    static const int32_t protected_values[] = {0, 1, -1};
    unsigned int mask;
    size_t active, protected_index;
    for (active = 0; active < sizeof(active_values) / sizeof(active_values[0]); ++active) {
        for (protected_index = 0;
             protected_index < sizeof(protected_values) / sizeof(protected_values[0]);
             ++protected_index) {
            for (mask = 0; mask < 16; ++mask) {
                void *pointers[4];
                size_t next = 0;
                unsigned int index;
                initialize(active_values[active], protected_values[protected_index], mask);
                assert(func_80062890() == 0);
                pointers[0] = g_timeout_resource_a;
                pointers[1] = g_timeout_resource_b;
                pointers[2] = g_timeout_resource_c;
                pointers[3] = g_timeout_resource_d;
                if (active_values[active] != 0) {
                    expect_call(&next, SHUTDOWN_62750, 0);
                    expect_call(&next, SHUTDOWN_8714C, 0);
                    expect_call(&next, SHUTDOWN_968BC, 0);
                    expect_call(&next, SHUTDOWN_96AEC, 0);
                    if (protected_values[protected_index] == 0) {
                        for (index = 0; index < 4; ++index) {
                            if (mask & (1u << index)) {
                                expect_call(&next, FREE, &allocations[index]);
                            }
                        }
                    }
                    expect_call(&next, STATE, g_timeout_resource_state);
                    assert(g_timeout_resource_active == 0);
                } else {
                    assert(g_timeout_resource_active == 0);
                }
                for (index = 0; index < 4; ++index) {
                    void *expected = (mask & (1u << index)) ? &allocations[index] : 0;
                    if (active_values[active] != 0 && protected_values[protected_index] == 0) {
                        expected = 0;
                    }
                    assert(pointers[index] == expected);
                }
                assert(call_count == next);
            }
        }
    }
}

static void test_callback_mutations(void) {
    size_t next = 0;
    initialize(1, 1, 15);
    unprotect_on_shutdown = 1;
    mutate_on_first_free = 1;
    assert(func_80062890() == 0);
    expect_call(&next, SHUTDOWN_62750, 0);
    expect_call(&next, SHUTDOWN_8714C, 0);
    expect_call(&next, SHUTDOWN_968BC, 0);
    expect_call(&next, SHUTDOWN_96AEC, 0);
    expect_call(&next, FREE, &allocations[0]);
    expect_call(&next, FREE, &allocations[5]);
    expect_call(&next, FREE, &allocations[3]);
    expect_call(&next, STATE, g_timeout_resource_state);
    assert(call_count == next);
    assert(g_timeout_resource_protected == 1);
    assert(g_timeout_resource_a == 0 && g_timeout_resource_b == 0);
    assert(g_timeout_resource_c == 0 && g_timeout_resource_d == 0);
    assert(g_timeout_resource_active == 0);
}

int main(void) {
    test_resource_combinations();
    test_callback_mutations();
    puts("timeout resource cleanup tests passed");
    return 0;
}
