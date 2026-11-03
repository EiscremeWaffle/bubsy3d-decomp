#include "move_preparation.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

void *g_move_preparation_transform;
const char g_move_table_condition[] = "table";
const char g_game_source_path[] = "../f/game.c";

static MovePreparationActor actor;
static MovePreparationInfo info;
static MovePreparationVector origin;
static MovePreparationEntry table;
static uint32_t calls[5];
static unsigned count;
static int32_t table_count, initializer_result, transform_status;
static uint8_t table_status, allocation_status;
static int clear_context, repair_table, install_transform;
static int transform_marker, scale_marker;
static uint32_t expected_flags;

static void record(uint32_t address) {
    assert(count < sizeof(calls) / sizeof(calls[0]));
    calls[count++] = address;
}

uint8_t func_80028C18(int32_t context, int32_t *output_count,
    const MovePreparationEntry **output_table) {
    record(0x80028C18);
    assert(context == 5);
    *output_count = table_count;
    *output_table = &table;
    return table_status;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    record(0x800556D0);
    assert(failed == 1 && condition == g_move_table_condition &&
        source == g_game_source_path && line == 0xE08);
    if (repair_table) {
        table.limit = 1;
    }
}

uint8_t func_800222EC(MovePreparationInfo **output, const MovePreparationVector *input) {
    record(0x800222EC);
    assert(output == &actor.move_info_28 && input == &origin);
    if (allocation_status != 0) {
        *output = &info;
    }
    if (clear_context) {
        actor.context_0C = 0;
    }
    if (install_transform) {
        g_move_preparation_transform = &transform_marker;
    }
    return allocation_status;
}

int32_t func_80054624(void **output) {
    record(0x80054624);
    assert(output == &g_move_preparation_transform);
    if (transform_status == 0) {
        *output = &transform_marker;
    }
    return transform_status;
}

int32_t func_8002611C(MovePreparationActor *input, const void *scale, uint32_t flags) {
    record(0x8002611C);
    assert(input == &actor && scale == &scale_marker && flags == expected_flags);
    assert(actor.move_info_28 == &info);
    assert(g_move_preparation_transform == &transform_marker);
    return initializer_result;
}

static void reset(void) {
    memset(&actor, 0, sizeof(actor));
    memset(&info, 0, sizeof(info));
    memset(&table, 0, sizeof(table));
    actor.context_0C = 5;
    table.limit = 1;
    table_count = 1;
    table_status = 1;
    allocation_status = 1;
    initializer_result = 1;
    transform_status = 0;
    g_move_preparation_transform = NULL;
    clear_context = 0;
    repair_table = 0;
    install_transform = 0;
    expected_flags = 0xFEDCBA98;
    count = 0;
}

static int32_t setup(void) {
    return func_80025FFC(&actor, &origin, &scale_marker, expected_flags);
}

static void test_early_exits(void) {
    reset();
    actor.move_info_28 = &info;
    actor.context_0C = 0;
    assert(setup() == 1 && count == 0 && g_move_preparation_transform == NULL);
    reset();
    table_count = 0;
    assert(setup() == 1 && count == 1 && actor.move_info_28 == NULL);
    reset();
    table.limit = 0;
    assert(setup() == 1 && count == 1 && actor.move_info_28 == NULL);
    reset();
    allocation_status = 0;
    assert(setup() == 0 && count == 2 && actor.move_info_28 == NULL);
    reset();
    clear_context = 1;
    assert(setup() == 1 && count == 2 && actor.move_info_28 == &info);
    assert(g_move_preparation_transform == NULL);
    reset();
    transform_status = -7;
    assert(setup() == 0 && count == 3 && actor.move_info_28 == &info);
    assert(calls[2] == 0x80054624);
    reset();
    table_status = 255;
    table_count = 0;
    assert(setup() == 1 && count == 2 && calls[1] == 0x800556D0);
}

static void test_results_and_mutations(void) {
    static const int32_t results[] = {
        INT32_MIN, -257, -256, -255, -1, 0, 1, 255, 256, 257, INT32_MAX
    };
    static const uint8_t allocation[] = {1, 255};
    static const uint32_t flags[] = {0, 1, 4, UINT32_MAX};
    unsigned result, a, f, ready;
    for (result = 0; result < sizeof(results) / sizeof(results[0]); ++result) {
        for (a = 0; a < sizeof(allocation) / sizeof(allocation[0]); ++a) {
            for (f = 0; f < sizeof(flags) / sizeof(flags[0]); ++f) {
                for (ready = 0; ready < 2; ++ready) {
                    reset();
                    initializer_result = results[result];
                    allocation_status = allocation[a];
                    expected_flags = flags[f];
                    if (ready) {
                        g_move_preparation_transform = &transform_marker;
                    }
                    assert(setup() == ((uint8_t)results[result] != 0));
                    assert(count == (ready ? 3u : 4u));
                    assert(calls[0] == 0x80028C18 && calls[1] == 0x800222EC);
                    if (!ready) {
                        assert(calls[2] == 0x80054624);
                    }
                    assert(calls[count - 1] == 0x8002611C);
                    count = 0;
                    assert(setup() == 1 && count == 0);
                }
            }
        }
    }
    reset();
    table_status = 0;
    table.limit = 0;
    repair_table = 1;
    install_transform = 1;
    assert(setup() == 1 && count == 4);
    assert(calls[0] == 0x80028C18 && calls[1] == 0x800556D0 &&
        calls[2] == 0x800222EC && calls[3] == 0x8002611C);
}

int main(void) {
    test_early_exits();
    test_results_and_mutations();
    puts("Movement setup: 176 result/allocation/flag/global combinations, repeat calls, early exits and helper mutations passed.");
    return 0;
}
