#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "bubsy_grounded_state.h"

typedef struct Component {
    uint8_t reserved[0x14];
    void *transform;
} Component;

_Static_assert(offsetof(BubsyGroundedShared, stride_delta_34) == 0x34, "increment layout");
_Static_assert(offsetof(BubsyGroundedShared, pending_38) == 0x38, "pending vector layout");
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(offsetof(Component, transform) == 0x14, "component target layout");
_Static_assert(offsetof(BubsyGroundedActor, component_1C) == 0x1C, "actor target layout");
_Static_assert(offsetof(BubsyGroundedActor, shared_28) == 0x28, "move-info target layout");
#endif

uint8_t g_stride_mode;
uint8_t g_deceleration_state_6464;
uint8_t g_deceleration_stop_649C;
uint8_t g_deceleration_active_64B8;
int32_t g_deceleration_state_64C0;
const char g_stride_error_condition[] = "errorFlag == EGSErrorNone";
const char g_stride_source_path[] = "../f/bubsy.c";

enum Action { CLEAR_BIT, SET_BIT, RESET_ACTOR, READ_TRANSLATION, TRANSITION,
    SELECT_SEQUENCE, ASSERTION, RESTART, PRODUCE };
typedef struct Call {
    enum Action action;
    int32_t argument;
    int32_t stride;
    int32_t delta;
    uint32_t state;
    uint8_t active;
} Call;

static BubsyGroundedActor actor;
static BubsyGroundedShared shared;
static BubsyGroundedShared other_shared;
static Component component;
static Component replacement;
static void *expected_component;
static int transform;
static int32_t selection_result;
static Call actual[16];
static unsigned int actual_count;
static Call expected[16];
static unsigned int expected_count;
static int32_t expected_stride;
static int32_t expected_delta;
static uint32_t expected_state;
static uint8_t expected_active;
static int replace_on_transition;
static int replace_on_assertion;
static int replace_shared_on_set;
static int mutate_on_clear;
static int stop_on_set;
static int pending_on_set;
static int mutate_on_produce;
static int mutate_on_reset;
static int negative_stride_on_set;

static int32_t wrap32(int64_t value) {
    uint32_t bits = (uint32_t)(uint64_t)value;
    return bits <= INT_MAX ? (int32_t)bits : (int32_t)((int64_t)bits - 4294967296LL);
}

static int32_t target_step(int32_t delta) {
    /* MIPS negu leaves INT_MIN negative before the arithmetic shift. */
    return delta == INT_MIN ? 524288 : delta / 4096;
}

static void record(enum Action action, int32_t argument) {
    Call *call;
    assert(actual_count < sizeof(actual) / sizeof(actual[0]));
    call = &actual[actual_count++];
    call->action = action;
    call->argument = argument;
    call->stride = shared.stride_30;
    call->delta = shared.stride_delta_34;
    call->state = actor.state_10;
    call->active = g_deceleration_active_64B8;
}

void func_8002262C(BubsyGroundedActor *value, int32_t bit) {
    assert(value == &actor);
    record(CLEAR_BIT, bit);
    actor.state_10 &= ~(1u << bit);
    if (mutate_on_clear) {
        g_stride_mode = 1;
        actor.state_10 = 0x200;
    }
}

void func_80022650(BubsyGroundedActor *value, int32_t bit) {
    assert(value == &actor);
    record(SET_BIT, bit);
    actor.state_10 |= 1u << bit;
    if (replace_shared_on_set) {
        actor.shared_28 = &other_shared;
        shared.stride_delta_34 = INT_MAX;
    }
    if (stop_on_set) {
        g_deceleration_stop_649C = 1;
    }
    if (pending_on_set) {
        shared.pending_38[2] = 123;
    }
    if (negative_stride_on_set) {
        shared.stride_30 = -3;
    }
}

void func_8003539C(BubsyGroundedActor *value, int32_t mode) {
    assert(value == &actor && mode == 1);
    record(RESET_ACTOR, mode);
    if (mutate_on_reset) {
        g_deceleration_active_64B8 = 0xFE;
        shared.stride_30 = 99;
        shared.stride_delta_34 = -77;
    }
}

void func_80054B20(void *value, int32_t mode, int32_t *output) {
    assert(value == &transform && mode == 0);
    record(READ_TRANSLATION, 0);
    output[0] = INT_MIN;
    output[1] = -321;
    output[2] = INT_MAX;
}

void func_8001B558(int32_t state, int32_t mode, int32_t a2, int32_t a3,
    int32_t *a4, int32_t *a5, int32_t *a6, int32_t *a7) {
    assert(state == 26 && mode == 4 && a2 == 0xFFFE && a3 == 0xFFFE);
    assert(*a4 == 14 && *a5 == INT_MIN && *a6 == -321 && *a7 == INT_MAX);
    assert(a6 == a5 + 1 && a7 == a5 + 2);
    record(TRANSITION, 14);
    if (replace_on_transition) {
        actor.component_1C = &replacement;
        expected_component = &replacement;
    }
}

int32_t func_80052AEC(void *value, int32_t sequence) {
    assert(value == expected_component && sequence == 0x318);
    record(SELECT_SEQUENCE, sequence);
    return selection_result;
}

void func_800556D0(int32_t level, const char *condition, const char *source, uint32_t line) {
    assert(level == 1 && line == 0xBC5);
    assert(condition == g_stride_error_condition && source == g_stride_source_path);
    record(ASSERTION, (int32_t)line);
    if (replace_on_assertion) {
        actor.component_1C = &replacement;
        expected_component = &replacement;
    }
}

void func_80052E18(void *value) {
    assert(value == expected_component);
    record(RESTART, 0);
}

void func_80022944(BubsyGroundedActor *value) {
    assert(value == &actor);
    record(PRODUCE, 0);
    if (mutate_on_produce) {
        shared.stride_30 = -333;
        shared.stride_delta_34 = 456;
        shared.pending_38[0] = 789;
        g_deceleration_active_64B8 = 5;
    }
}

static void reset(uint8_t mode, uint32_t state, int32_t stride, int32_t delta,
    uint8_t state_6464, int32_t state_64C0, uint8_t stop, unsigned int pending) {
    memset(&actor, 0, sizeof(actor));
    memset(&shared, 0, sizeof(shared));
    memset(&other_shared, 0, sizeof(other_shared));
    memset(&component, 0, sizeof(component));
    memset(&replacement, 0, sizeof(replacement));
    actor.component_1C = &component;
    actor.shared_28 = &shared;
    actor.flags_04 = 0xA1234567;
    actor.state_10 = state;
    component.transform = &transform;
    replacement.transform = &transform;
    shared.stride_30 = stride;
    shared.stride_delta_34 = delta;
    if (pending != 0) {
        shared.pending_38[pending - 1] = -123;
    }
    g_stride_mode = mode;
    g_deceleration_state_6464 = state_6464;
    g_deceleration_state_64C0 = state_64C0;
    g_deceleration_stop_649C = stop;
    g_deceleration_active_64B8 = 0x99;
    selection_result = 0;
    expected_component = &component;
    actual_count = 0;
    replace_on_transition = 0;
    replace_on_assertion = 0;
    replace_shared_on_set = 0;
    mutate_on_clear = 0;
    stop_on_set = 0;
    pending_on_set = 0;
    mutate_on_produce = 0;
    mutate_on_reset = 0;
    negative_stride_on_set = 0;
}

static void expect(enum Action action, int32_t argument) {
    Call *call;
    assert(expected_count < sizeof(expected) / sizeof(expected[0]));
    call = &expected[expected_count++];
    call->action = action;
    call->argument = argument;
    call->stride = expected_stride;
    call->delta = expected_delta;
    call->state = expected_state;
    call->active = expected_active;
    if (action == CLEAR_BIT) {
        expected_state &= ~(1u << argument);
    } else if (action == SET_BIT) {
        expected_state |= 1u << argument;
    }
}

static void check_stable_case(void) {
    int32_t pending[3];
    int32_t decrement;
    int32_t step;
    int32_t next;
    int captured_state_8;
    unsigned int i;
    expected_count = 0;
    expected_stride = shared.stride_30;
    expected_delta = shared.stride_delta_34;
    expected_state = actor.state_10;
    expected_active = g_deceleration_active_64B8;
    memcpy(pending, shared.pending_38, sizeof(pending));
    if (expected_stride <= 0) {
        expected_stride = 0;
        expected_delta = 0;
        expect(CLEAR_BIT, 11);
        if (g_stride_mode == 1 && g_deceleration_state_6464 == 0) {
            expect(CLEAR_BIT, 6);
            expect(RESET_ACTOR, 1);
        } else if (g_stride_mode == 0 && (expected_state & 0x44) == 0) {
            expect(RESET_ACTOR, 1);
        }
        expected_active = 0;
    } else {
        captured_state_8 = expected_delta >= 0 && (expected_state & 8) != 0;
        if (expected_delta >= 0) {
            if (g_stride_mode == 0 && (expected_state & 0x200) == 0 &&
                (expected_state & 0x18) != 0) {
                if ((expected_state & 0x10) != 0) {
                    expect(READ_TRANSLATION, 0);
                    expect(TRANSITION, 14);
                }
                expect(SELECT_SEQUENCE, 0x318);
                if (selection_result != 0) {
                    expect(ASSERTION, 0xBC5);
                }
                expect(RESTART, 0);
            }
            expect(CLEAR_BIT, 3);
            expect(CLEAR_BIT, 4);
            expect(SET_BIT, 11);
            expected_delta = 0;
        }
        if (captured_state_8) {
            expected_delta = -65400;
        } else {
            if (g_stride_mode == 0 &&
                ((expected_state & 4) != 0 || g_deceleration_state_6464 != 0)) {
                if (expected_delta >= 0) {
                    expected_delta = -409;
                }
                decrement = g_deceleration_state_64C0 != 0 ? 818 : 409;
            } else {
                decrement = (expected_state & 0x200) != 0 ? 81 : 1308;
            }
            expected_delta = wrap32((int64_t)expected_delta - decrement);
        }
        if (expected_stride > 35) {
            expected_stride = 35;
        }
        if (g_deceleration_stop_649C != 0) {
            expected_active = 0;
        } else if (pending[0] == 0 && pending[2] == 0) {
            step = target_step(expected_delta);
            next = wrap32((int64_t)expected_stride + step);
            if (next > 35) {
                expected_stride = wrap32(35LL - step);
            } else if (next < 0 && expected_stride >= 0) {
                expected_stride = wrap32(-(int64_t)step);
            }
            expect(PRODUCE, 0);
        }
    }
    func_80038844(&actor);
    assert(actual_count == expected_count);
    for (i = 0; i < expected_count; ++i) {
        assert(actual[i].action == expected[i].action);
        assert(actual[i].argument == expected[i].argument);
        assert(actual[i].stride == expected[i].stride);
        assert(actual[i].delta == expected[i].delta);
        assert(actual[i].state == expected[i].state);
        assert(actual[i].active == expected[i].active);
    }
    assert(shared.stride_30 == expected_stride && shared.stride_delta_34 == expected_delta);
    assert(actor.state_10 == expected_state && actor.flags_04 == 0xA1234567);
    assert(g_deceleration_active_64B8 == expected_active);
    assert(memcmp(pending, shared.pending_38, sizeof(pending)) == 0);
}

static void test_helper_mutations(void) {
    reset(0, 0x10, 20, 0, 0, 0, 0, 0);
    replace_on_transition = 1;
    replace_shared_on_set = 1;
    mutate_on_produce = 1;
    func_80038844(&actor);
    assert(actual_count == 8);
    assert(actual[7].action == PRODUCE && actual[7].stride == 20 && actual[7].delta == -1308);
    assert(actor.shared_28 == &other_shared);
    assert(other_shared.stride_30 == 0 && other_shared.stride_delta_34 == 0);
    assert(shared.stride_30 == -333 && shared.stride_delta_34 == 456);
    assert(shared.pending_38[0] == 789 && g_deceleration_active_64B8 == 5);

    reset(0, 0x18, 20, 0, 0, 0, 0, 0);
    replace_on_assertion = 1;
    selection_result = 9;
    mutate_on_clear = 1;
    func_80038844(&actor);
    assert(actual_count == 9);
    assert(actual[8].action == PRODUCE && actual[8].stride == 20 && actual[8].delta == -65400);

    reset(0, 0, 20, 0, 0, 0, 0, 0);
    mutate_on_clear = 1;
    func_80038844(&actor);
    assert(actual_count == 4 && actual[3].action == PRODUCE);
    assert(shared.stride_delta_34 == -81);

    reset(0, 0, 0, -555, 0, 0, 0, 0);
    mutate_on_clear = 1;
    mutate_on_reset = 1;
    func_80038844(&actor);
    assert(actual_count == 3 && actual[1].argument == 6 && actual[2].action == RESET_ACTOR);
    assert(shared.stride_30 == 99 && shared.stride_delta_34 == -77);
    assert(g_deceleration_active_64B8 == 0);

    reset(2, 0, 99, 0, 0, 0, 0, 0);
    stop_on_set = 1;
    func_80038844(&actor);
    assert(actual_count == 3 && shared.stride_30 == 35 && shared.stride_delta_34 == -1308);
    assert(g_deceleration_active_64B8 == 0);

    reset(2, 0, 99, 0, 0, 0, 0, 0);
    pending_on_set = 1;
    func_80038844(&actor);
    assert(actual_count == 3 && shared.stride_30 == 35 && shared.pending_38[2] == 123);
    assert(g_deceleration_active_64B8 == 0x99);

    reset(0, 0, 20, 0, 0, 0, 0, 0);
    negative_stride_on_set = 1;
    func_80038844(&actor);
    assert(actual_count == 4 && shared.stride_30 == -3 && shared.stride_delta_34 == -1308);
}

int main(void) {
    static const uint8_t modes[] = {0, 1, 2, 255};
    static const int32_t strides[] = {INT_MIN, -1, 0, 1, 15, 16, 34, 35, 36, INT_MAX};
    static const int32_t deltas[] = {INT_MIN, INT_MIN + 1, INT_MIN + 81, INT_MIN + 409,
        INT_MIN + 818, INT_MIN + 1308, -65536, -4097, -4096, -4095, -1, 0, 1, 4095, 4096, INT_MAX};
    static const uint32_t bits[] = {4, 8, 0x10, 0x40, 0x200};
    unsigned int m, s, d, state_bits, g, c, stop, p, e, b, cases = 0;
    uint32_t state;
    for (m = 0; m < sizeof(modes) / sizeof(modes[0]); ++m)
    for (s = 0; s < sizeof(strides) / sizeof(strides[0]); ++s)
    for (d = 0; d < sizeof(deltas) / sizeof(deltas[0]); ++d)
    for (state_bits = 0; state_bits < 32; ++state_bits)
    for (g = 0; g < 2; ++g)
    for (c = 0; c < 2; ++c)
    for (stop = 0; stop < 2; ++stop)
    for (p = 0; p < 4; ++p)
    for (e = 0; e < 2; ++e) {
        state = 0x40000000 | ((s & 1) != 0 ? 0x800 : 0);
        for (b = 0; b < sizeof(bits) / sizeof(bits[0]); ++b) {
            if ((state_bits & (1u << b)) != 0) {
                state |= bits[b];
            }
        }
        reset(modes[m], state, strides[s], deltas[d], g ? 255 : 0, c ? -1 : 0,
            stop ? 7 : 0, p);
        selection_result = e ? -1 : 0;
        check_stable_case();
        ++cases;
    }
    test_helper_mutations();
    printf("Grounded deceleration: %u boundary combinations and helper mutations passed\n", cases);
    return 0;
}
