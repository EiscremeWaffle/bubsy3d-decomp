#include "move_preparation.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

void *g_move_preparation_transform;
const char g_move_table_condition[] = "table";
const char g_move_error_condition[] = "error";
const char g_game_source_path[] = "../f/game.c";

static MovePreparationActor actor;
static MovePreparationInfo info, replacement;
static MovePreparationComponent component, replacement_component;
static MovePreparationEntry table[2];
static MovePreparationVector payload[2], direction;
static MovePreparationBasis snapshot;
static uint32_t calls[16], failure;
static unsigned count, assertions;
static uint8_t query_status, selector;
static int mutation;
static int transform_marker, changed_marker, global_marker, changed_global, scale_marker;
static void *expected_transform;

static int32_t record(uint32_t address) {
    assert(count < sizeof(calls) / sizeof(calls[0]));
    calls[count++] = address;
    return failure == address ? -7 : 0;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    record(0x800556D0);
    assert(source == g_game_source_path);
    if (line == 0xE41) {
        assert(failed == 1 && condition == g_move_table_condition);
    } else {
        assert(line == 0xE55 && failed == 0 && condition == g_move_error_condition);
    }
    ++assertions;
}

uint8_t func_80028C18(int32_t context, int32_t *table_count, const MovePreparationEntry **output) {
    record(0x80028C18);
    assert(context == 5);
    *table_count = 2;
    *output = table;
    if (mutation) {
        actor.move_info_28 = &replacement;
        info.origin_58.x = INT32_MAX;
    }
    return query_status;
}

void func_800286F8(int32_t context, uint8_t *output) {
    record(0x800286F8);
    assert(context == 5);
    assert(info.counter == 0 && info.index == 0 && info.mode_1C == 1);
    assert(info.target.x == INT32_MIN && info.target.y == INT32_MAX && info.target.z == -3);
    *output = selector;
    if (mutation) {
        info.index = 1;
        info.origin_58.y = 1234;
    }
}

int32_t func_80062D4C(const MovePreparationVector *left,
    const MovePreparationVector *right, int32_t *output) {
    assert(left == &info.origin_58 && right == &info.target && output == &info.separation_18);
    assert(left->y == (mutation ? 1234 : INT32_MIN));
    *output = 17;
    if (mutation) {
        actor.component_1C = &replacement_component;
        expected_transform = &changed_marker;
    }
    return record(0x80062D4C);
}

int32_t func_80054C7C(void *transform, int32_t mode, const MovePreparationVector *input) {
    assert(transform == g_move_preparation_transform && mode == 0 && input == &info.target);
    if (mutation) {
        actor.component_1C = &component;
        g_move_preparation_transform = &changed_global;
    }
    return record(0x80054C7C);
}

int32_t func_800546F4(void *transform, MovePreparationBasis *output) {
    assert(transform == expected_transform);
    memset(output, 0x5A, sizeof(*output));
    snapshot = *output;
    return record(0x800546F4);
}

int32_t func_800550A4(void *transform, void *target) {
    assert(transform == expected_transform && target == g_move_preparation_transform);
    return record(0x800550A4);
}

int32_t func_8005541C(void *transform, const void *input) {
    assert(transform == expected_transform && input == &scale_marker);
    return record(0x8005541C);
}

int32_t func_80054AF0(void *transform, MovePreparationVector *output) {
    assert(transform == expected_transform);
    *output = direction;
    return record(0x80054AF0);
}

int32_t func_800549E8(void *transform, const MovePreparationBasis *input) {
    assert(transform == expected_transform);
    assert(memcmp(input, &snapshot, sizeof(*input)) == 0);
    assert(memcmp(&info.transform_88, &direction, sizeof(direction)) == 0);
    return record(0x800549E8);
}

static void reset(void) {
    memset(&actor, 0, sizeof(actor));
    memset(&info, 0, sizeof(info));
    memset(&replacement, 0, sizeof(replacement));
    memset(table, 0, sizeof(table));
    actor.context_0C = 5;
    actor.move_info_28 = &info;
    actor.component_1C = &component;
    component.transform_14 = &transform_marker;
    replacement_component.transform_14 = &changed_marker;
    expected_transform = &transform_marker;
    g_move_preparation_transform = &global_marker;
    info.counter = 99;
    info.index = 1;
    info.mode_1C = 2;
    info.stride_30 = -123;
    info.unknown_34 = 987;
    info.origin_58 = (MovePreparationVector){INT32_MAX, INT32_MIN, -1, 44};
    info.target.padding = 55;
    info.transform_88 = (MovePreparationVector){1, 2, 3, 66};
    info.pending_38 = (MovePreparationVector){7, 8, 9, 10};
    payload[0] = (MovePreparationVector){1, -1, -2, 77};
    payload[1] = (MovePreparationVector){999, 999, 999, 88};
    table[0].selector = 7;
    table[1].selector = 255;
    table[0].payload = payload;
    table[1].payload = payload + 1;
    direction = (MovePreparationVector){-111, 0, 222, 0x12345678};
    query_status = 1;
    selector = 1;
    failure = 0;
    mutation = 0;
    count = 0;
    assertions = 0;
}

static void check_preserved(void) {
    assert(info.target.padding == 55 && info.unknown_34 == 987);
    assert(info.pending_38.x == 7 && info.pending_38.y == 8 &&
        info.pending_38.z == 9 && info.pending_38.padding == 10);
    assert(memcmp(&replacement, &(MovePreparationInfo){0}, sizeof(replacement)) == 0);
}

static void test_boundaries(void) {
    static const uint32_t expected_calls[] = {
        0x80028C18, 0x800286F8, 0x80062D4C, 0x80054C7C,
        0x800546F4, 0x800550A4, 0x8005541C, 0x80054AF0, 0x800549E8
    };
    static const int32_t ys[] = {
        INT32_MIN, -3277, -3276, -3275, 0, 3275, 3276, 3277, INT32_MAX
    };
    static const uint8_t selectors[] = {0, 1, 255};
    unsigned flags, y, s;
    for (flags = 0; flags < 8; ++flags) {
        for (y = 0; y < sizeof(ys) / sizeof(ys[0]); ++y) {
            for (s = 0; s < sizeof(selectors) / sizeof(selectors[0]); ++s) {
                int extract = (flags & 5u) != 0;
                int keep = extract && ((flags & 1u) || ys[y] < -3275 || ys[y] > 3275);
                reset();
                direction.y = ys[y];
                selector = selectors[s];
                assert(func_8002611C(&actor, &scale_marker, flags) == 1);
                assert(count == (unsigned)(7 + extract + keep));
                assert(memcmp(calls, expected_calls, count * sizeof(calls[0])) == 0);
                assert(info.counter == 0 && info.index == 0 && info.mode_1C == 1);
                assert(info.stride_30 == (selector == 1 ? 14 : -123));
                assert(info.separation_18 == 17);
                if (keep) {
                    assert(memcmp(&info.transform_88, &direction, sizeof(direction)) == 0);
                } else {
                    assert(info.transform_88.x == 0 && info.transform_88.y == 0 &&
                        info.transform_88.z == 0 && info.transform_88.padding == 66);
                }
                check_preserved();
            }
        }
    }
}

static void test_failures_and_mutations(void) {
    static const uint32_t failures[] = {
        0x80054C7C, 0x800546F4, 0x800550A4, 0x8005541C, 0x80054AF0, 0x800549E8
    };
    unsigned i;
    for (i = 0; i < sizeof(failures) / sizeof(failures[0]); ++i) {
        reset();
        failure = failures[i];
        assert(func_8002611C(&actor, &scale_marker, 1) == 0);
        assert(calls[count - 1] == failure);
        if (failure != 0x800549E8) {
            assert(info.transform_88.x == 1 && info.transform_88.padding == 66);
        }
        check_preserved();
    }
    reset();
    failure = 0x80062D4C;
    query_status = 255;
    assert(func_8002611C(&actor, &scale_marker, 0) == 1);
    assert(assertions == 2);
    check_preserved();
    reset();
    mutation = 1;
    assert(func_8002611C(&actor, &scale_marker, 1) == 1);
    assert(actor.move_info_28 == &replacement && info.index == 1 && info.stride_30 == 510);
    assert(g_move_preparation_transform == &changed_global);
    check_preserved();
}

int main(void) {
    test_boundaries();
    test_failures_and_mutations();
    puts("Movement initializer: 216 flag/direction/selector boundaries, six failure exits and helper mutations passed.");
    return 0;
}
