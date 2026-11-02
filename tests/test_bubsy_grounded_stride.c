#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "bubsy_grounded_state.h"

_Static_assert(offsetof(BubsyGroundedShared, stride_30) == 0x30, "stride offset");
_Static_assert(offsetof(BubsyGroundedShared, stride_delta_34) == 0x34, "stride delta offset");
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(offsetof(BubsyGroundedActor, component_1C) == 0x1C, "component offset");
_Static_assert(offsetof(BubsyGroundedActor, shared_28) == 0x28, "move-info offset");
#endif

uint8_t g_stride_mode;
uint8_t g_stride_override;
uint8_t g_stride_marker;
const char g_stride_error_condition[] = "errorFlag == EGSErrorNone";
const char g_stride_source_path[] = "../f/bubsy.c";

enum Action { GET_SEQUENCE, SELECT_SEQUENCE, ASSERTION, RESTART, READ_TRANSLATION,
    TRANSITION, CLEAR_BIT, SET_BIT, PRODUCE };

typedef struct Call {
    enum Action action;
    int32_t argument;
    int32_t stride;
    int32_t delta;
} Call;

typedef struct Component {
    uint32_t reserved;
    uint8_t flags;
} Component;

static BubsyGroundedActor actor;
static BubsyGroundedShared shared;
static BubsyGroundedShared other_shared;
static Component component;
static Component replacement;
static void *expected_component;
static int transform;
static int32_t sequence;
static int32_t selection_result;
static Call actual[16];
static unsigned int actual_count;
static int mutate_selection_mode;
static int mutate_selection_component;
static int mutate_property_mode;
static int mutate_property_stride;
static int mutate_shared_pointer;
static int mutate_producer;
static int32_t observed_stride;
static int32_t observed_delta;

static void record(enum Action action, int32_t argument) {
    Call *call;
    assert(actual_count < sizeof(actual) / sizeof(actual[0]));
    call = &actual[actual_count++];
    call->action = action;
    call->argument = argument;
    call->stride = shared.stride_30;
    call->delta = shared.stride_delta_34;
}

void func_80052ABC(void *value, int32_t *output) {
    assert(value == expected_component);
    assert(g_stride_marker == 0);
    record(GET_SEQUENCE, sequence);
    *output = sequence;
    if (mutate_shared_pointer) {
        actor.shared_28 = &other_shared;
    }
}

int32_t func_80052AEC(void *value, int32_t key) {
    assert(value == expected_component);
    record(SELECT_SEQUENCE, key);
    if (mutate_selection_mode) {
        g_stride_mode = 1;
    }
    if (mutate_selection_component) {
        actor.component_1C = &replacement;
        expected_component = &replacement;
    }
    return selection_result;
}

void func_800556D0(int32_t level, const char *condition, const char *source, uint32_t line) {
    assert(level == 1);
    assert(condition == g_stride_error_condition && source == g_stride_source_path);
    record(ASSERTION, (int32_t)line);
}

void func_80052E18(void *value) {
    assert(value == expected_component);
    record(RESTART, 0);
}

void func_80054B20(void *value, int32_t mode, int32_t *output) {
    assert(value == &transform && mode == 0);
    record(READ_TRANSLATION, 0);
    output[0] = 123;
    output[1] = -456;
    output[2] = INT_MAX;
}

void func_8001B558(int32_t state, int32_t mode, int32_t a2, int32_t a3,
    int32_t *a4, int32_t *a5, int32_t *a6, int32_t *a7) {
    assert(state == 26 && mode == 4 && a2 == 0xFFFE && a3 == 0xFFFE);
    assert(*a4 == 14 && *a5 == 123 && *a6 == -456 && *a7 == INT_MAX);
    assert(a6 == a5 + 1 && a7 == a5 + 2);
    record(TRANSITION, 14);
}

void func_8002262C(BubsyGroundedActor *value, int32_t bit) {
    assert(value == &actor);
    record(CLEAR_BIT, bit);
    if (mutate_property_mode) {
        g_stride_mode = 2;
    }
    if (mutate_property_stride) {
        shared.stride_30 = INT_MAX;
    }
}

void func_80022650(BubsyGroundedActor *value, int32_t bit) {
    assert(value == &actor);
    record(SET_BIT, bit);
}

void func_80022944(BubsyGroundedActor *value) {
    assert(value == &actor);
    observed_stride = shared.stride_30;
    observed_delta = shared.stride_delta_34;
    record(PRODUCE, 0);
    if (mutate_producer) {
        shared.stride_30 = -12345;
        shared.stride_delta_34 = INT_MIN;
    }
}

static void reset(uint8_t mode, int32_t stride, int32_t delta, int32_t key,
    uint8_t flags, uint8_t override, int32_t error) {
    memset(&actor, 0, sizeof(actor));
    memset(&shared, 0, sizeof(shared));
    memset(&other_shared, 0, sizeof(other_shared));
    memset(&component, 0, sizeof(component));
    memset(&replacement, 0, sizeof(replacement));
    actor.shared_28 = &shared;
    actor.component_1C = &component;
    shared.stride_30 = stride;
    shared.stride_delta_34 = delta;
    component.flags = flags;
    g_stride_mode = mode;
    g_stride_override = override;
    g_stride_marker = 77;
    sequence = key;
    selection_result = error;
    expected_component = &component;
    actual_count = 0;
    mutate_selection_mode = 0;
    mutate_selection_component = 0;
    mutate_property_mode = 0;
    mutate_property_stride = 0;
    mutate_shared_pointer = 0;
    mutate_producer = 0;
}

static Call expected[16];
static unsigned int expected_count;
static int32_t expected_stride;
static int32_t expected_delta;

static void expect(enum Action action, int32_t argument) {
    Call *call;
    assert(expected_count < sizeof(expected) / sizeof(expected[0]));
    call = &expected[expected_count++];
    call->action = action;
    call->argument = argument;
    call->stride = expected_stride;
    call->delta = expected_delta;
}

static void expect_sequence(int32_t key, uint32_t line, int emit_transition) {
    if (sequence == key) {
        return;
    }
    expect(SELECT_SEQUENCE, key);
    if (selection_result != 0) {
        expect(ASSERTION, (int32_t)line);
    }
    expect(RESTART, 0);
    if (emit_transition) {
        expect(READ_TRANSLATION, 0);
        expect(TRANSITION, 14);
    }
}

static void check_stable_case(void) {
    unsigned int i;
    int32_t floor = g_stride_mode == 0 ? 17 : 12;
    int32_t cap = g_stride_mode == 0 ? 35 : 12;
    expected_count = 0;
    expected_stride = shared.stride_30;
    expected_delta = shared.stride_delta_34;
    expect(GET_SEQUENCE, sequence);
    if (g_stride_mode < 2) {
        if (expected_stride < floor) {
            expect_sequence(g_stride_mode == 0 ? 19 : 0,
                g_stride_mode == 0 ? 0x536 : 0x542, g_stride_mode == 0);
            expected_stride = floor;
            expected_delta = 0x147;
            if (g_stride_mode == 0) {
                expect(SET_BIT, 3);
            }
            expect(CLEAR_BIT, 4);
        } else if (expected_stride < cap) {
            expect_sequence(19, 0x55E, 0);
            if ((component.flags & 0x20) != 0) {
                expected_delta = (int32_t)((uint32_t)expected_delta + 0x147u);
            }
        } else {
            expect(CLEAR_BIT, 3);
            expect(SET_BIT, 4);
            expect_sequence(g_stride_mode == 0 ? 36 : 0,
                g_stride_mode == 0 ? 0x578 : 0x584, g_stride_mode == 0);
            expected_stride = cap;
            expected_delta = 0;
        }
    }
    if (g_stride_override == 1) {
        expected_stride = g_stride_mode == 0 ? 35 : 26;
    } else if (g_stride_override == 0 && g_stride_mode < 2 && expected_stride > cap) {
        expected_stride = cap;
    }
    expect(PRODUCE, 0);
    func_80035ACC(&actor, &transform);
    assert(actual_count == expected_count);
    for (i = 0; i < expected_count; ++i) {
        assert(actual[i].action == expected[i].action);
        assert(actual[i].argument == expected[i].argument);
        assert(actual[i].stride == expected[i].stride);
        assert(actual[i].delta == expected[i].delta);
    }
    assert(shared.stride_30 == expected_stride && shared.stride_delta_34 == expected_delta);
    assert(observed_stride == expected_stride && observed_delta == expected_delta);
}

static void test_helper_mutations(void) {
    uint8_t mode;
    reset(0, 0, -1, 7, 0, 1, 0);
    mutate_selection_mode = 1;
    mutate_selection_component = 1;
    func_80035ACC(&actor, &transform);
    assert(shared.stride_30 == 26 && shared.stride_delta_34 == 0x147);
    assert(actual_count == 7 && actual[5].action == CLEAR_BIT && actual[5].argument == 4);

    reset(0, 35, -1, 7, 0, 0, 0);
    mutate_property_mode = 1;
    func_80035ACC(&actor, &transform);
    assert(actual_count == 4);
    assert(actual[1].action == CLEAR_BIT && actual[2].action == SET_BIT);
    assert(shared.stride_30 == 12 && shared.stride_delta_34 == 0);

    reset(0, 0, -1, 19, 0, 0, 0);
    mutate_shared_pointer = 1;
    func_80035ACC(&actor, &transform);
    assert(actor.shared_28 == &other_shared);
    assert(shared.stride_30 == 17 && shared.stride_delta_34 == 0x147);
    assert(other_shared.stride_30 == 0 && other_shared.stride_delta_34 == 0);

    reset(2, 77, 91, 0, 0, 1, 0);
    mutate_producer = 1;
    func_80035ACC(&actor, &transform);
    assert(observed_stride == 26 && observed_delta == 91);
    assert(shared.stride_30 == -12345 && shared.stride_delta_34 == INT_MIN);

    for (mode = 0; mode < 2; ++mode) {
        reset(mode, 0, -1, mode == 0 ? 19 : 0, 0, 0, 0);
        mutate_property_stride = 1;
        func_80035ACC(&actor, &transform);
        assert(observed_stride == (mode == 0 ? 35 : 12));
        assert(observed_delta == 0x147);
    }

    reset(0, 0, -1, 19, 0, 2, 0);
    mutate_property_stride = 1;
    func_80035ACC(&actor, &transform);
    assert(observed_stride == INT_MAX && observed_delta == 0x147);

    reset(0, 20, INT_MAX, 77, 0, 0, 0);
    mutate_selection_mode = 1;
    mutate_selection_component = 1;
    replacement.flags = 0x20;
    func_80035ACC(&actor, &transform);
    assert(observed_stride == 12);
    assert(observed_delta == (int32_t)((uint32_t)INT_MAX + 0x147u));
    assert(actual_count == 4 && actual[2].action == RESTART && actual[3].action == PRODUCE);
}

int main(void) {
    static const uint8_t modes[] = {0, 1, 2, 255};
    static const int32_t strides[] = {INT_MIN, -1, 0, 11, 12, 16, 17, 18, 34, 35, 36, INT_MAX};
    static const int32_t deltas[] = {0, -0x147, INT_MAX - 100, INT_MIN};
    static const int32_t keys[] = {0, 19, 36, 77};
    unsigned int m, s, d, k, f, o, e, cases = 0;
    for (m = 0; m < sizeof(modes) / sizeof(modes[0]); ++m)
    for (s = 0; s < sizeof(strides) / sizeof(strides[0]); ++s)
    for (d = 0; d < sizeof(deltas) / sizeof(deltas[0]); ++d)
    for (k = 0; k < sizeof(keys) / sizeof(keys[0]); ++k)
    for (f = 0; f < 2; ++f)
    for (o = 0; o < 3; ++o)
    for (e = 0; e < 2; ++e) {
        reset(modes[m], strides[s], deltas[d], keys[k], f ? 0x20 : 0, (uint8_t)o, e ? -1 : 0);
        check_stable_case();
        ++cases;
    }
    test_helper_mutations();
    printf("Grounded stride producer: %u boundary combinations and helper mutations passed\n", cases);
    return 0;
}
