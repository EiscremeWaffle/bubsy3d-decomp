#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "move_preparation.h"

_Static_assert(sizeof(MovePreparationVector) == 16, "vector size");
_Static_assert(sizeof(MovePreparationBasis) == 32, "basis size");
_Static_assert(sizeof(MovePreparationInfo) == 0xA4, "move-info size");
_Static_assert(offsetof(MovePreparationInfo, separation_18) == 0x18, "separation offset");
_Static_assert(offsetof(MovePreparationInfo, mode_1C) == 0x1C, "mode offset");
_Static_assert(offsetof(MovePreparationInfo, stride_30) == 0x30, "stride offset");
_Static_assert(offsetof(MovePreparationInfo, pending_38) == 0x38, "pending offset");
_Static_assert(offsetof(MovePreparationInfo, origin_58) == 0x58, "origin offset");
_Static_assert(offsetof(MovePreparationInfo, transform_88) == 0x88, "transform offset");
_Static_assert(offsetof(MovePreparationActor, context_0C) == 0x0C, "context offset");
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(offsetof(MovePreparationActor, component_1C) == 0x1C, "component offset");
_Static_assert(offsetof(MovePreparationActor, move_info_28) == 0x28, "move-info offset");
_Static_assert(offsetof(MovePreparationComponent, transform_14) == 0x14, "transform offset");
_Static_assert(sizeof(MovePreparationEntry) == 12, "table stride");
#endif

void *g_move_preparation_transform;
const char g_move_context_condition[] = "context";
const char g_move_info_condition[] = "move-info";
const char g_move_table_condition[] = "table";
const char g_move_error_condition[] = "error";
const char g_move_tag_condition[] = "tag";
const char g_game_source_path[] = "../f/game.c";

static MovePreparationActor actor;
static MovePreparationInfo info;
static MovePreparationComponent component;
static MovePreparationEntry table[4];
static MovePreparationVector payload[4];
static MovePreparationVector values;
static MovePreparationVector targets[8];
static MovePreparationVector global_target;
static MovePreparationBasis saved_basis;
static uint32_t calls[64];
static unsigned int call_count, separation_count, pending_axis;
static int32_t table_count, separations[8];
static uint32_t failure_address, assertion_line;
static uint8_t table_result, selector;
static int transform_marker, global_marker, basis_marker;
static int mutate_on_notify, mutate_tag, repair_context, mutate_after_selection;
static void *expected_transform;

static int32_t record(uint32_t address) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = address;
    return failure_address == address ? -7 : 0;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    record(0x800556D0);
    assert(source == g_game_source_path);
    assertion_line = line;
    if (line == 0xD27) {
        assert(failed == 0 && condition == g_move_context_condition);
        if (repair_context) {
            actor.context_0C = 5;
        }
    } else if (line == 0xD2A) {
        assert(failed == 1 && condition == g_move_info_condition);
    } else if (line == 0xD3A) {
        assert(failed == 1 && condition == g_move_table_condition);
    } else if (line == 0xDA7) {
        assert(failed == 0 && condition == g_move_error_condition);
    } else {
        assert(line == 0xDAD && failed == 0 && condition == g_move_tag_condition);
        if (mutate_tag) {
            table[info.index].tag = 10;
        }
    }
}

uint8_t func_80028C18(int32_t context, int32_t *count, const MovePreparationEntry **output) {
    record(0x80028C18);
    assert(context == actor.context_0C);
    *count = table_count;
    *output = table;
    return table_result;
}

void func_800286F8(int32_t context, uint8_t *output) {
    record(0x800286F8);
    assert(context == actor.context_0C);
    *output = selector;
    if (mutate_after_selection) {
        info.index = 1;
    }
}

int32_t func_80054B20(void *transform, int32_t mode, MovePreparationVector *output) {
    assert(transform == expected_transform && mode == 0);
    *output = (MovePreparationVector){11, 12, 13, 14};
    return record(0x80054B20);
}

int32_t func_80062D4C(const MovePreparationVector *left, const MovePreparationVector *right, int32_t *output) {
    assert(left->x == 11 && left->y == 12 && left->z == 13 && left->padding == 14);
    assert(right == &info.target && output == &info.separation_18);
    assert(separation_count < sizeof(separations) / sizeof(separations[0]));
    targets[separation_count] = *right;
    *output = separations[separation_count++];
    return record(0x80062D4C);
}

int32_t func_80054C7C(void *transform, int32_t mode, const MovePreparationVector *input) {
    assert(transform == g_move_preparation_transform && mode == 0 && input == &info.target);
    global_target = *input;
    return record(0x80054C7C);
}

int32_t func_800546F4(void *transform, MovePreparationBasis *output) {
    assert(transform == expected_transform);
    memset(output, 0x5A, sizeof(*output));
    saved_basis = *output;
    return record(0x800546F4);
}

int32_t func_800550A4(void *transform, void *global_transform) {
    assert(transform == expected_transform && global_transform == g_move_preparation_transform);
    return record(0x800550A4);
}

int32_t func_8005541C(void *transform, const void *input) {
    assert(transform == expected_transform && input == &basis_marker);
    return record(0x8005541C);
}

int32_t func_80054AF0(void *transform, MovePreparationVector *output) {
    assert(transform == expected_transform);
    *output = values;
    return record(0x80054AF0);
}

int32_t func_800549E8(void *transform, const MovePreparationBasis *input) {
    assert(transform == expected_transform && memcmp(input, &saved_basis, sizeof(*input)) == 0);
    return record(0x800549E8);
}

void func_80022944(MovePreparationActor *input) {
    assert(input == &actor);
    record(0x80022944);
    if (mutate_on_notify) {
        info.pending_38 = (MovePreparationVector){0, 0, 0, 99};
        if (pending_axis == 0) {
            info.pending_38.x = -1;
        } else if (pending_axis == 1) {
            info.pending_38.y = -1;
        } else if (pending_axis == 2) {
            info.pending_38.z = -1;
        }
    }
}

void func_80024EBC(MovePreparationActor *input) {
    assert(input == &actor);
    assert(info.pending_38.x || info.pending_38.y || info.pending_38.z);
    record(0x80024EBC);
}

static void reset(void) {
    unsigned int index;
    memset(&actor, 0, sizeof(actor));
    memset(&info, 0, sizeof(info));
    memset(&component, 0, sizeof(component));
    memset(table, 0, sizeof(table));
    memset(targets, 0, sizeof(targets));
    memset(&global_target, 0, sizeof(global_target));
    memset(separations, 0, sizeof(separations));
    actor.context_0C = 5;
    actor.component_1C = &component;
    actor.move_info_28 = &info;
    component.transform_14 = &transform_marker;
    expected_transform = &transform_marker;
    g_move_preparation_transform = &global_marker;
    table_count = 4;
    table_result = 1;
    selector = 1;
    for (index = 0; index < 4; index++) {
        payload[index] = (MovePreparationVector){(int32_t)index + 2, -(int32_t)index - 3, 4, 99};
        table[index] = (MovePreparationEntry){10, (uint8_t)(index + 1), 0, 2, payload};
    }
    info.origin_58 = (MovePreparationVector){100, 200, 300, 400};
    info.target.padding = 123;
    info.transform_88 = (MovePreparationVector){21, 22, 23, 24};
    values = (MovePreparationVector){31, 32, 33, 34};
    separations[0] = 1;
    call_count = separation_count = pending_axis = 0;
    assertion_line = failure_address = 0;
    mutate_on_notify = mutate_tag = repair_context = mutate_after_selection = 0;
}

static int32_t run(uint32_t flags, uint8_t *completion) {
    return func_80025A98(&actor, &basis_marker, flags, completion);
}

static void expect_calls(const uint32_t *expected, unsigned int count) {
    assert(call_count == count);
    assert(memcmp(expected, calls, count * sizeof(*calls)) == 0);
}

#define EXPECT(...) do { \
    const uint32_t expected[] = {__VA_ARGS__}; \
    expect_calls(expected, sizeof(expected) / sizeof(expected[0])); \
} while (0)

static void test_early_paths(void) {
    int32_t index;
    unsigned int axis;
    uint8_t complete;
    for (index = -2; index <= -1; index++) {
        reset();
        info.index = index;
        complete = 99;
        assert(run(0, &complete) == 1 && complete == 0 && call_count == 0);
    }
    for (axis = 0; axis <= 3; axis++) {
        reset();
        info.separation_18 = 1;
        mutate_on_notify = 1;
        pending_axis = axis;
        assert(run(0, &complete) == 1 && complete == 0);
        if (axis < 3) {
            EXPECT(0x80022944, 0x80024EBC);
        } else {
            EXPECT(0x80022944);
        }
        assert(info.transform_88.x == 21 && info.transform_88.padding == 24);
    }
    reset();
    actor.context_0C = 0;
    info.index = -1;
    assert(run(0, &complete) == 1);
    EXPECT(0x800556D0);
    assert(assertion_line == 0xD27);
    reset();
    info.index = 3;
    info.mode_1C = 1;
    assert(run(0, &complete) == 1 && complete == 1);
    EXPECT(0x80028C18);
    assert(info.counter == 2 && info.index == 3 && info.mode_1C == 0);
    reset();
    actor.component_1C = NULL;
    assert(run(0, &complete) == 0 && complete == 0);
    EXPECT(0x80028C18);
}

static void test_vectors_and_tables(void) {
    uint8_t complete;
    reset();
    assert(run(0, &complete) == 1 && complete == 0);
    EXPECT(0x80028C18, 0x800286F8, 0x80054B20, 0x80062D4C,
        0x80054C7C, 0x800546F4, 0x800550A4, 0x8005541C, 0x80022944);
    assert(info.index == 0 && info.counter == 0 && info.mode_1C == 1);
    assert(info.target.x == 102 && info.target.y == 197 && info.target.z == 304);
    assert(info.target.padding == 123 && info.origin_58.x == 100);
    assert(info.stride_30 == 2 && global_target.x == 102);
    assert(info.transform_88.x == 0 && info.transform_88.y == 0 &&
        info.transform_88.z == 0 && info.transform_88.padding == 24);

    reset();
    separations[0] = 0;
    separations[1] = 1;
    assert(run(0, &complete) == 1);
    assert(separation_count == 2 && info.counter == 1);
    assert(targets[0].x == 102 && targets[1].x == 103);
    assert(targets[1].y == 196 && targets[1].z == 304);
    assert(targets[1].padding == 123);

    reset();
    info.index = 2;
    info.mode_1C = 2;
    table[1].tag = 3;
    assert(run(0, &complete) == 1);
    assert(info.index == 0 && info.counter == 1 && info.mode_1C == 2);
    assert(info.target.x == 103);

    reset();
    info.mode_1C = 1;
    assert(run(0, &complete) == 1);
    assert(info.index == 1 && info.counter == 0);
    assert(info.stride_30 == 4);

    reset();
    table[0].tag = 3;
    table[0].limit = INT32_MAX;
    info.counter = INT32_MAX;
    actor.component_1C = NULL;
    assert(run(0, &complete) == 0);
    assert(info.counter == INT32_MIN);

    reset();
    info.origin_58 = (MovePreparationVector){INT32_MAX, INT32_MIN, -1, 0};
    payload[0] = (MovePreparationVector){1, -1, INT32_MIN, 0};
    selector = 0;
    info.stride_30 = 99;
    assert(run(0, &complete) == 1);
    assert(info.target.x == INT32_MIN && info.target.y == INT32_MAX && info.target.z == INT32_MAX);
    assert(info.stride_30 == 99);

    reset();
    mutate_after_selection = 1;
    assert(run(0, &complete) == 1);
    assert(info.stride_30 == 4 && info.target.x == 102);
}

static void test_transform_gates_and_failures(void) {
    static const int32_t y_values[] = {-3276, -3275, 0, 3275, 3276, INT32_MIN, INT32_MAX};
    static const uint32_t failures[] = {
        0x80054B20, 0x80054C7C, 0x800546F4, 0x800550A4, 0x8005541C, 0x80054AF0, 0x800549E8
    };
    unsigned int index, flags, call;
    uint8_t complete;
    for (flags = 0; flags <= 7; flags++) {
        for (index = 0; index < sizeof(y_values) / sizeof(y_values[0]); index++) {
            int enabled = (flags & 5) != 0;
            int stored = enabled && ((flags & 1) != 0 ||
                (uint32_t)y_values[index] + 0xCCBu >= 0x1997u);
            reset();
            values.y = y_values[index];
            assert(run(flags, &complete) == 1);
            assert(call_count == (unsigned int)(9 + enabled + stored));
            if (enabled) {
                assert(calls[8] == 0x80054AF0);
            }
            if (stored) {
                assert(calls[9] == 0x800549E8);
                assert(memcmp(&info.transform_88, &values, sizeof(values)) == 0);
            } else {
                assert(info.transform_88.x == 0 && info.transform_88.y == 0 && info.transform_88.z == 0);
                assert(info.transform_88.padding == 24);
            }
            assert(calls[call_count - 1] == 0x80022944);
        }
    }
    for (index = 0; index < sizeof(failures) / sizeof(failures[0]); index++) {
        reset();
        failure_address = failures[index];
        assert(run(1, &complete) == 0 && complete == 0);
        assert(calls[call_count - 1] == failures[index]);
        for (call = 0; call < call_count; call++) {
            assert(calls[call] != 0x80022944 && calls[call] != 0x80024EBC);
        }
        if (failures[index] == 0x800549E8) {
            assert(memcmp(&info.transform_88, &values, sizeof(values)) == 0);
        }
    }
    reset();
    failure_address = 0x80062D4C;
    assert(run(0, &complete) == 1 && assertion_line == 0xDA7);
    assert(calls[4] == 0x800556D0 && calls[call_count - 1] == 0x80022944);
    reset();
    info.separation_18 = -1;
    expected_transform = NULL;
    assert(run(0, &complete) == 1);
    EXPECT(0x80028C18, 0x80054C7C, 0x800546F4, 0x800550A4, 0x8005541C, 0x80022944);
}

static void test_assertion_continuation(void) {
    uint8_t complete;
    reset();
    table[0].tag = 3;
    table[0].limit = 10;
    separations[0] = 0;
    assert(run(0, &complete) == 1);
    assert(assertion_line == 0xDAD && separation_count == 1 && info.counter == 1);
    assert(calls[4] == 0x800556D0);

    reset();
    table[0].tag = 3;
    table[0].limit = 10;
    separations[0] = 0;
    separations[1] = 1;
    mutate_tag = 1;
    assert(run(0, &complete) == 1);
    assert(assertion_line == 0xDAD && separation_count == 2 && info.counter == 0);
    assert(targets[0].x == 103 && targets[1].x == 102);

    reset();
    table_result = 0;
    assert(run(0, &complete) == 1);
    assert(assertion_line == 0xD3A && calls[1] == 0x800556D0);

    reset();
    repair_context = 1;
    actor.context_0C = 0;
    assert(run(0, &complete) == 1 && actor.context_0C == 5);
    assert(assertion_line == 0xD27 && calls[0] == 0x800556D0);
}

int main(void) {
    test_early_paths();
    test_vectors_and_tables();
    test_transform_gates_and_failures();
    test_assertion_continuation();
    puts("Movement preparation execution tests passed");
    return 0;
}
