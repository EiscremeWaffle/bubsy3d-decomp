#include "reset_entry_helpers.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(sizeof(ResetEntryGroup) == 0x10, "reset entry group size");
_Static_assert(offsetof(ResetEntryGroup, count) == 0x08, "reset entry group count");
_Static_assert(offsetof(ResetSelectionActor, object) == 0x08, "selection actor object");
#endif

ResetEntryGroup *g_reset_entry_groups;
void **g_reset_selected_objects;
void **g_reset_entry_table;

enum CallKind { GET_ACTOR, CHECK_ENTRY, PREPARE_OBJECT };
typedef struct Call {
    enum CallKind kind;
    uint32_t index;
    void *pointer;
} Call;

static Call calls[16];
static size_t call_count;
static ResetSelectionActor actor;
static ResetEntryGroup groups[2], other_groups[2];
static uint32_t indices[3], other_indices[3];
static uint8_t assets[3], replacements[3];
static void *objects[3], *other_objects[3];
static int32_t check_results[3];
static int mutate_getter, mutate_check;

static void record(enum CallKind kind, uint32_t index, void *pointer) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count].kind = kind;
    calls[call_count].index = index;
    calls[call_count].pointer = pointer;
    ++call_count;
}

void *func_80028764(int32_t index) {
    assert(index == 0);
    record(GET_ACTOR, 0, 0);
    if (mutate_getter) {
        g_reset_entry_groups = other_groups;
        groups[0].count = 2;
    }
    return &actor;
}

int32_t func_80044A88(uint32_t index, ResetEntryGroup *group) {
    assert(index < 3);
    record(CHECK_ENTRY, index, group);
    if (mutate_check) {
        assert(index == 0 && group == &groups[0]);
        group->indices = other_indices;
        group->count = 3;
        g_reset_selected_objects = other_objects;
        actor.object = &assets[2];
        mutate_check = 0;
    }
    return check_results[index];
}

int32_t func_80052BDC(void *object, int32_t mode) {
    assert(mode == 1);
    record(PREPARE_OBJECT, 0, object);
    return 0;
}

static void initialize(int32_t count) {
    size_t index;
    call_count = 0;
    mutate_getter = 0;
    mutate_check = 0;
    for (index = 0; index < 3; ++index) {
        indices[index] = (uint32_t)index;
        objects[index] = &assets[index];
        other_objects[index] = &replacements[index];
        check_results[index] = 0;
    }
    groups[0].indices = indices;
    groups[0].count = count;
    other_groups[0].indices = indices;
    other_groups[0].count = 0;
    g_reset_entry_groups = groups;
    g_reset_selected_objects = objects;
    actor.object = &assets[1];
}

static void expect(size_t *next, enum CallKind kind, uint32_t index, void *pointer) {
    assert(*next < call_count);
    assert(calls[*next].kind == kind && calls[*next].index == index);
    assert(calls[*next].pointer == pointer);
    ++*next;
}

static void test_modes_and_counts(void) {
    static const uint32_t modes[] = {0, 1, 255, 256, 257, UINT32_MAX};
    static const int32_t counts[] = {INT32_MIN, -1, 0, 1, 3};
    size_t mode, count, entry;

    for (mode = 0; mode < sizeof(modes) / sizeof(modes[0]); ++mode) {
        for (count = 0; count < sizeof(counts) / sizeof(counts[0]); ++count) {
            size_t next = 0;
            initialize(counts[count]);
            check_results[0] = 0x100;
            check_results[2] = 0x101;
            assert(func_80044B24(0, modes[mode]) == 1);
            expect(&next, GET_ACTOR, 0, 0);
            for (entry = 0; entry < (size_t)(counts[count] > 0 ? counts[count] : 0); ++entry) {
                if (entry == 1) {
                    continue;
                }
                if ((modes[mode] & 0xFFu) != 0) {
                    expect(&next, CHECK_ENTRY, (uint32_t)entry, &groups[0]);
                }
                if ((modes[mode] & 0xFFu) == 0 || (check_results[entry] & 0xFF) == 0) {
                    expect(&next, PREPARE_OBJECT, 0, &assets[entry]);
                }
            }
            assert(next == call_count);
        }
    }
}

static void test_live_mutations(void) {
    size_t next = 0;
    initialize(1);
    mutate_getter = 1;
    mutate_check = 1;
    other_indices[0] = 0;
    other_indices[1] = 2;
    other_indices[2] = 1;
    other_objects[1] = &assets[1];
    other_objects[2] = &assets[2];
    check_results[0] = 0x100;
    check_results[1] = 0x101;
    assert(func_80044B24(0, 1) == 1);
    expect(&next, GET_ACTOR, 0, 0);
    expect(&next, CHECK_ENTRY, 0, &groups[0]);
    expect(&next, PREPARE_OBJECT, 0, &assets[0]);
    expect(&next, CHECK_ENTRY, 1, &groups[0]);
    assert(next == call_count);
    assert(g_reset_entry_groups == other_groups);
    assert(g_reset_selected_objects == other_objects && groups[0].count == 3);
}

static void test_real_entry_dispatch(void) {
    void *table[2] = {&replacements[0], &replacements[1]};
    size_t next = 0;
    initialize(3);
    g_reset_entry_table = table;
    assert(func_80044458(0, 256) == 1);
    expect(&next, PREPARE_OBJECT, 0, &replacements[0]);
    expect(&next, GET_ACTOR, 0, 0);
    expect(&next, PREPARE_OBJECT, 0, &assets[0]);
    expect(&next, PREPARE_OBJECT, 0, &assets[2]);
    assert(next == call_count);
}

int main(void) {
    test_modes_and_counts();
    test_live_mutations();
    test_real_entry_dispatch();
    puts("reset-entry group dispatcher tests passed");
    return 0;
}
