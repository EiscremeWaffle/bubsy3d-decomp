#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "bubsy_grounded_state.h"
#include "bubsy_actor_update.h"

_Static_assert(offsetof(BubsyGroundedDescriptor, state_11) == 0x11, "descriptor layout");
_Static_assert(offsetof(BubsyGroundedDescriptor, state_14) == 0x14, "descriptor layout");
_Static_assert(offsetof(BubsyGroundedDescriptor, state_4C) == 0x4C, "descriptor layout");
_Static_assert(offsetof(BubsyGroundedDescriptor, state_68) == 0x68, "descriptor layout");
_Static_assert(offsetof(BubsyGroundedShared, stride_30) == 0x30, "shared layout");
_Static_assert(offsetof(BubsyGroundedLookup, fields.index_14) == 0x14, "lookup layout");
_Static_assert(offsetof(BubsyGroundedLookup, fields.gate_0D) == 0x0D, "lookup layout");
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(offsetof(BubsyGroundedActor, component_1C) == 0x1C, "actor target layout");
_Static_assert(offsetof(BubsyGroundedActor, shared_28) == 0x28, "actor target layout");
#endif

int16_t g_grounded_counter;
void *g_grounded_event;
int32_t g_grounded_counter_state;
int32_t g_grounded_reference_vector[4];
int32_t g_grounded_current_vector[4];
const char g_grounded_error_none[] = "errorFlag == EGSErrorNone";
const char g_grounded_source_path[] = "../f/bubsy.c";

typedef struct Call {
    uint32_t address;
    intptr_t argument;
} Call;

static Call calls[32];
static unsigned int call_count;
static BubsyGroundedDescriptor descriptor;
static BubsyGroundedActor actor;
static BubsyGroundedActor companion;
static BubsyGroundedShared shared;
static BubsyGroundedState state;
static BubsyGroundedLookup lookup;
static int32_t sequence;
static int32_t normalized_dot;
static int32_t selection_result;
static uint8_t boundary;
static void *context;
static int component;
static int transform;
static int event;
static int companion_present;
static int mutate_on_set_bit;
static int mutate_on_allocate;

static void record(uint32_t address, intptr_t argument) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count].address = address;
    calls[call_count++].argument = argument;
}

void func_8003AA1C(BubsyGroundedDescriptor **output) {
    record(0x8003AA1C, 0);
    *output = &descriptor;
}

void *func_8001DCE0(void) {
    record(0x8001DCE0, 0);
    return companion_present ? &companion : NULL;
}

void func_800516F8(void **output) {
    record(0x800516F8, 0);
    *output = &transform;
}

void func_80054AF0(void *value, int32_t *output) {
    assert(value == &transform);
    assert(output == g_grounded_reference_vector || output == g_grounded_current_vector);
    record(0x80054AF0, output == g_grounded_reference_vector ? 0 : 1);
}

void func_8005395C(const int32_t *input, int32_t *output) {
    assert(input == output);
    assert(output == g_grounded_reference_vector || output == g_grounded_current_vector);
    record(0x8005395C, output == g_grounded_reference_vector ? 0 : 1);
}

void func_800100A0(const int32_t *left, const int32_t *right, int32_t *output) {
    assert(left == g_grounded_reference_vector && right == g_grounded_current_vector);
    record(0x800100A0, 0);
    *output = normalized_dot;
}

void func_80052ABC(void *value, int32_t *output) {
    assert(value == &component);
    record(0x80052ABC, 0);
    *output = sequence;
}

int32_t func_80052AEC(void *value, int32_t property) {
    assert(value == &component);
    record(0x80052AEC, property);
    return selection_result;
}

void func_80052E18(void *value) {
    assert(value == &component);
    record(0x80052E18, 0);
}

void func_8005290C(void *value, uint8_t *output) {
    assert(value == &component);
    record(0x8005290C, 0);
    *output = boundary;
}

void func_8002262C(BubsyGroundedActor *value, int32_t selector) {
    assert(value == &actor);
    if (state.id == 38 && selector == 17) {
        assert(descriptor.state_11 == 0);
    }
    record(0x8002262C, selector);
}

void func_80022650(BubsyGroundedActor *value, int32_t selector) {
    assert(value == &actor);
    record(0x80022650, selector);
    if (mutate_on_set_bit) {
        actor.flags_04 |= 0x80;
        actor.state_10 = 0;
    }
}

void func_8002269C(BubsyGroundedActor *value, int32_t selector) {
    assert(value == &actor);
    record(0x8002269C, selector);
}

void func_8002E7D8(void) { record(0x8002E7D8, 0); }
void func_80037B30(int32_t mode) { record(0x80037B30, mode); }

void func_80039158(BubsyGroundedActor *value, void *argument) {
    assert(value == &actor && argument == context);
    record(0x80039158, 0);
}

void func_800347EC(BubsyGroundedActor *value, void *argument, int32_t mode) {
    assert(value == &actor && argument == context);
    record(0x800347EC, mode);
}

void func_800358C8(BubsyGroundedActor *value, void *argument, int32_t mode) {
    assert(value == &actor && argument == context);
    record(0x800358C8, mode);
}

void func_80035ACC(BubsyGroundedActor *value, void *argument) {
    assert(value == &actor && argument == context);
    record(0x80035ACC, 0);
}

void func_8001C700(int32_t selection, int32_t value) {
    assert(selection == 4 && value == 3);
    record(0x8001C700, selection);
}

void func_8001B558(int32_t value, int32_t mode, uint32_t flags, uint32_t heading,
    int32_t a4, int32_t a5, int32_t a6, int32_t a7) {
    assert(value == 15 && mode == 4 && flags == 0xFFFE && heading == actor.heading_0C);
    assert(a4 == 0 && a5 == 0 && a6 == 0 && a7 == 0);
    record(0x8001B558, value);
}

void func_800556D0(int32_t level, const char *condition, const char *source, uint32_t line) {
    assert(level == 1 && condition == g_grounded_error_none && source == g_grounded_source_path);
    record(0x800556D0, line);
}

void func_80022944(BubsyGroundedActor *value) {
    assert(value == &actor && shared.stride_30 == -17);
    record(0x80022944, 0);
}

void func_8003539C(BubsyGroundedActor *value, int32_t mode) {
    assert(value == &actor && mode == 1);
    record(0x8003539C, mode);
}

void func_8004FFEC(void *value) {
    assert(value == &event);
    record(0x8004FFEC, 0);
}

void func_8004B2A4(BubsyGroundedActor *value, BubsyGroundedState *argument, void *ctx, int32_t count) {
    (void)value;
    (void)argument;
    (void)ctx;
    (void)count;
    assert(0);
}

void func_8004FF50(void (*callback)(BubsyGroundedActor *, BubsyGroundedState *, void *, int32_t),
    int32_t mode, void **output) {
    assert(callback == func_8004B2A4 && mode == 1 && output == &g_grounded_event);
    record(0x8004FF50, mode);
    *output = &event;
    if (mutate_on_allocate) {
        g_grounded_counter = 0;
    }
}

void func_80034EF8(BubsyGroundedActor *value, BubsyGroundedState *argument, void *ctx, int32_t count) {
    assert(value == &actor && argument == &state && ctx == context);
    record(0x80034EF8, count);
}

static void reset(uint16_t id) {
    memset(&descriptor, 0, sizeof(descriptor));
    memset(&actor, 0, sizeof(actor));
    memset(&companion, 0, sizeof(companion));
    memset(&shared, 0, sizeof(shared));
    memset(&lookup, 0, sizeof(lookup));
    state.id = id;
    lookup.words[1] = 3;
    actor.component_1C = &component;
    actor.shared_28 = &shared;
    actor.heading_0C = 0x12345678;
    context = &lookup;
    sequence = normalized_dot = selection_result = 0;
    boundary = 0;
    g_grounded_counter = 0;
    g_grounded_event = NULL;
    g_grounded_counter_state = 0;
    companion_present = mutate_on_set_bit = mutate_on_allocate = 0;
    call_count = 0;
}

static void run(void) {
    bubsy_grounded_state_handler(&state, &actor, context, &lookup, &shared);
}

static void expect_calls(const Call *expected, unsigned int count) {
    unsigned int index;
    assert(call_count == count + 1);
    assert(calls[0].address == 0x8003AA1C);
    for (index = 0; index < count; index++) {
        assert(calls[index + 1].address == expected[index].address);
        assert(calls[index + 1].argument == expected[index].argument);
    }
}

#define EXPECT(...) do { \
    const Call expected[] = {__VA_ARGS__}; \
    expect_calls(expected, sizeof(expected) / sizeof(expected[0])); \
} while (0)

static void test_paths(void) {
    unsigned int id;
    for (id = 0; id <= 40; id++) {
        reset((uint16_t)id);
        run();
        assert(call_count > 0);
        if (id < 6 || (id >= 18 && id <= 19) || (id >= 22 && id <= 28) || id > 38) {
            expect_calls(NULL, 0);
        }
    }

    reset(20);
    run();
    EXPECT({0x800516F8, 0}, {0x80054AF0, 0}, {0x80052ABC, 0},
        {0x80022650, 12}, {0x80052AEC, 0x349});
    reset(20);
    mutate_on_set_bit = 1;
    run();
    EXPECT({0x800516F8, 0}, {0x80054AF0, 0}, {0x80052ABC, 0}, {0x80022650, 12});

    reset(21);
    actor.state_10 = 0x1000;
    run();
    EXPECT({0x8001DCE0, 0}, {0x800516F8, 0}, {0x80054AF0, 1},
        {0x8005395C, 0}, {0x8005395C, 1}, {0x800100A0, 0},
        {0x8002E7D8, 0}, {0x80022650, 8}, {0x800347EC, 1});
    reset(21);
    actor.state_10 = 0x1000;
    normalized_dot = 0xCCC;
    descriptor.state_02 = 7;
    run();
    EXPECT({0x8001DCE0, 0}, {0x800516F8, 0}, {0x80054AF0, 1},
        {0x8005395C, 0}, {0x8005395C, 1}, {0x800100A0, 0}, {0x8002262C, 12});
    assert(descriptor.state_02 == 0);

    reset(21);
    actor.state_10 = 0x1000;
    companion_present = 1;
    companion.shared_28 = &shared;
    descriptor.state_02 = 7;
    run();
    EXPECT({0x8001DCE0, 0}, {0x800516F8, 0}, {0x80054AF0, 1},
        {0x8005395C, 0}, {0x8005395C, 1}, {0x800100A0, 0}, {0x8002262C, 12});
    assert(descriptor.state_02 == 0);

    reset(12);
    actor.flags_04 = 8;
    descriptor.state_02 = 7;
    run();
    EXPECT({0x800347EC, 1});
    assert(descriptor.state_02 == 7 && actor.state_01 == 1);

    reset(14);
    actor.flags_04 = 8;
    selection_result = 1;
    run();
    EXPECT({0x80022650, 7}, {0x80052AEC, 0x2BE}, {0x800556D0, 0x1B8}, {0x80052E18, 0});

    reset(15);
    sequence = 0x2BE;
    selection_result = 1;
    run();
    EXPECT({0x80052ABC, 0}, {0x80052AEC, 0x2C3}, {0x800556D0, 0x1C7},
        {0x80052E18, 0}, {0x8001B558, 15});
    reset(15);
    boundary = 1;
    run();
    EXPECT({0x80052ABC, 0}, {0x8005290C, 0}, {0x8002262C, 7}, {0x8003539C, 1});

    reset(17);
    actor.state_10 = 4;
    run();
    EXPECT({0x8001C700, 4}, {0x80022650, 2}, {0x800347EC, 1});

    reset(31);
    descriptor.counter_4D = 11;
    sequence = 5;
    run();
    EXPECT({0x80052ABC, 0}, {0x80052AEC, 0}, {0x80052E18, 0},
        {0x80022650, 23}, {0x80022944, 0});
    assert(descriptor.counter_4D == 12 && shared.stride_30 == -17);
    reset(31);
    descriptor.counter_4D = UINT8_MAX;
    run();
    EXPECT({0x80052ABC, 0}, {0x80022650, 23}, {0x80022944, 0});
    assert(descriptor.counter_4D == 0 && shared.stride_30 == -17);

    reset(32);
    run();
    EXPECT({0x80022650, 16}, {0x800347EC, 1});
    assert(actor.state_01 == 2 && descriptor.counter_4D == 0);
    reset(33);
    run();
    EXPECT({0x80022650, 15}, {0x80022650, 8}, {0x800347EC, 1});
    assert(descriptor.state_00 == 3);

    reset(8);
    g_grounded_event = &event;
    g_grounded_counter = 40;
    g_grounded_counter_state = 8;
    run();
    EXPECT({0x8004FFEC, 0}, {0x80034EF8, 40});
    assert(g_grounded_event == NULL && g_grounded_counter == 40);
    reset(10);
    g_grounded_counter = INT16_MAX;
    g_grounded_counter_state = 10;
    run();
    EXPECT({0x80034EF8, INT16_MIN});
    reset(9);
    sequence = 0x2BB;
    actor.state_10 = 0x720;
    g_grounded_counter = 7;
    run();
    EXPECT({0x80052ABC, 0}, {0x80052AEC, 0x2B5}, {0x80052E18, 0},
        {0x8004FF50, 1}, {0x80034EF8, 7});
    assert(actor.state_10 == 0x220 && g_grounded_event == &event);
    reset(11);
    sequence = 0x2B8;
    actor.state_10 = 0xFFFFEFFFu;
    actor.state_14 = 0x12345678;
    run();
    EXPECT({0x80052ABC, 0}, {0x80052AEC, 0x2B5}, {0x80052E18, 0}, {0x8003539C, 1});
    assert(actor.state_10 == (0xFFFFEFFFu & ~0x720u) && actor.state_14 == 0);
    reset(9);
    actor.state_10 = 0x220;
    g_grounded_counter = 7;
    mutate_on_allocate = 1;
    run();
    EXPECT({0x80052ABC, 0}, {0x8004FF50, 1}, {0x8003539C, 1});
    assert(actor.state_10 == 0 && g_grounded_event == &event && g_grounded_counter == 0);
    reset(9);
    actor.state_10 = 0x220;
    g_grounded_counter = 7;
    g_grounded_event = &event;
    run();
    EXPECT({0x80052ABC, 0}, {0x80034EF8, 7});
    assert(g_grounded_event == &event);

    reset(37);
    run();
    EXPECT({0x80037B30, 0});
    assert(descriptor.state_11 == 1);
    reset(38);
    descriptor.state_11 = 1;
    run();
    EXPECT({0x8002262C, 17}, {0x80039158, 0});
    assert(descriptor.state_11 == 0);
    reset(38);
    descriptor.state_11 = 2;
    run();
    EXPECT({0x80039158, 0});
    assert(descriptor.state_11 == 4);
    reset(37);
    descriptor.state_11 = 5;
    run();
    expect_calls(NULL, 0);
    reset(37);
    descriptor.state_11 = 3;
    run();
    EXPECT({0x80039158, 0});
}

static void test_prelude(void) {
    static const int32_t values[] = {-1, 0, 1, 2, 3, 4, INT32_MAX, INT32_MIN};
    static const uint16_t ids[] = {6, 14, 15, 29};
    unsigned int value_index, id_index, flags, active, prior;
    for (value_index = 0; value_index < sizeof(values) / sizeof(values[0]); value_index++) {
        for (id_index = 0; id_index < sizeof(ids) / sizeof(ids[0]); id_index++) {
            for (flags = 0; flags <= 1; flags++) {
                for (active = 0; active <= 1; active++) {
                    for (prior = 0; prior <= 5; prior++) {
                        int16_t expected = (int16_t)prior;
                        int32_t dispatch;
                        reset(ids[id_index]);
                        lookup.words[1] = values[value_index];
                        actor.flags_04 = flags ? 0x1100 : 0;
                        actor.state_10 = active ? 0x1000 : 0;
                        descriptor.state_02 = expected;
                        dispatch = bubsy_grounded_handler_should_dispatch_switch(state.id,
                            values[value_index], actor.flags_04, actor.state_10, &expected);
                        run();
                        assert(descriptor.state_02 == expected);
                        if (!dispatch) {
                            expect_calls(NULL, 0);
                            assert(descriptor.state_10 == 0);
                        } else if (state.id == 29) {
                            assert(descriptor.state_10 == 1);
                        }
                    }
                }
            }
        }
    }

    reset(29);
    lookup.fields.index_14 = 1;
    lookup.words[1] = -1;
    lookup.words[2] = 3;
    run();
    assert(descriptor.state_10 == 1);
    reset(20);
    lookup.fields.gate_0D = 1;
    run();
    expect_calls(NULL, 0);
}

static void test_model_regressions(void) {
    static const uint32_t flags[] = {0, 2, 4, 8, 0x40, 0x80, 0x100, 0x800, 0x1000, 0x1104};
    static const uint32_t states[] = {
        0, 4, 0x40, 0x44, 0x1000, 0x2000, 0x10000, 0x20000, 0x21000, 0x80000,
        0x82000, 0x1000000, 0x1002000, 0x1004, 0x100040
    };
    static const uint8_t counters[] = {0, 10, 11, 255};
    unsigned int f, s, prior, c;
    for (f = 0; f < sizeof(flags) / sizeof(flags[0]); f++) {
        for (s = 0; s < sizeof(states) / sizeof(states[0]); s++) {
            for (prior = 0; prior <= 2; prior++) {
                uint8_t expected_actor = (uint8_t)prior;
                int16_t expected_descriptor = 7;
                int32_t dispatch;
                uint8_t expected_state_14 = (uint8_t)prior;
                reset(12);
                actor.flags_04 = flags[f];
                actor.state_10 = states[s];
                actor.state_01 = expected_actor;
                descriptor.state_02 = expected_descriptor;
                bubsy_grounded_case12_update_state(flags[f], states[s],
                    &expected_actor, &expected_descriptor);
                run();
                assert(actor.state_01 == expected_actor && descriptor.state_02 == expected_descriptor);

                reset(16);
                actor.flags_04 = flags[f];
                actor.state_10 = states[s];
                actor.state_01 = (uint8_t)prior;
                descriptor.state_14 = expected_state_14;
                dispatch = bubsy_grounded_case16_should_dispatch(flags[f], states[s],
                    (uint8_t)prior, &expected_state_14);
                run();
                assert(descriptor.state_14 == expected_state_14);
                if (dispatch) {
                    EXPECT({0x800347EC, 0});
                } else {
                    expect_calls(NULL, 0);
                }

                for (c = 0; c < sizeof(counters) / sizeof(counters[0]); c++) {
                    uint8_t expected_counter = counters[c];
                    uint8_t expected_4C = 4;
                    uint8_t expected_68 = 5;
                    int32_t expected_stride = 7;
                    uint8_t actions;
                    expected_actor = (uint8_t)prior;
                    expected_descriptor = 6;
                    reset(32);
                    actor.flags_04 = flags[f];
                    actor.state_10 = states[s];
                    actor.state_01 = expected_actor;
                    descriptor.state_02 = expected_descriptor;
                    descriptor.state_4C = expected_4C;
                    descriptor.state_68 = expected_68;
                    descriptor.counter_4D = expected_counter;
                    shared.stride_30 = expected_stride;
                    actions = bubsy_grounded_case32_update_state(flags[f], states[s],
                        &expected_actor, &expected_counter, &expected_4C, &expected_68,
                        &expected_descriptor, &expected_stride);
                    run();
                    assert(actor.state_01 == expected_actor && descriptor.state_02 == expected_descriptor);
                    assert(descriptor.state_4C == expected_4C && descriptor.state_68 == expected_68);
                    assert(descriptor.counter_4D == expected_counter && shared.stride_30 == expected_stride);
                    if ((actions & BUBSY_GROUNDED_CASE32_CALL_22650_10) != 0) {
                        EXPECT({0x80022650, 16}, {0x800347EC, 1});
                    } else if ((actions & BUBSY_GROUNDED_CASE32_CALL_3539C_1) != 0) {
                        EXPECT({0x8002262C, 23}, {0x8003539C, 1});
                    } else if ((actions & BUBSY_GROUNDED_CASE32_CALL_2262C_17) != 0) {
                        EXPECT({0x8002262C, 23});
                    } else {
                        expect_calls(NULL, 0);
                    }
                }
            }
        }
    }
}

int main(void) {
    test_paths();
    test_prelude();
    test_model_regressions();
    puts("Grounded handler execution tests passed");
    return 0;
}
