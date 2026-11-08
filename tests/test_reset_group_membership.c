#include "reset_entry_helpers.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

ResetEntryGroup *g_reset_entry_groups;
void **g_reset_selected_objects;
void **g_reset_entry_table;
uint32_t g_reset_active_group_count;
uint32_t g_reset_active_group_indices[4];

static ResetEntryGroup groups[4];
static uint32_t entries[4][3];
static ResetSelectionActor actor;
static uint8_t assets[3], entry_asset;
static void *objects[3], *entry_table[1];
static void *prepared[4];
static size_t prepared_count;
static int mutate_on_first_object;
static unsigned int actor_calls;
static uint32_t random_state = 0x44A88u;

void *func_80028764(int32_t index) {
    assert(index == 0);
    ++actor_calls;
    return &actor;
}

void func_80052BDC(void *object, int32_t mode) {
    assert(mode == 1 && prepared_count < sizeof(prepared) / sizeof(prepared[0]));
    prepared[prepared_count++] = object;
    if (mutate_on_first_object && object == &assets[0]) {
        groups[1].count = 0;
    }
}

static uint32_t next_random(void) {
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}

static int expected_membership(uint32_t object_index, const ResetEntryGroup *excluded) {
    uint32_t active;
    for (active = 0; active < g_reset_active_group_count; ++active) {
        const ResetEntryGroup *group = &groups[g_reset_active_group_indices[active]];
        int32_t entry;
        if (group == excluded) {
            continue;
        }
        for (entry = 0; entry < group->count; ++entry) {
            if (group->indices[entry] == object_index) {
                return 1;
            }
        }
    }
    return 0;
}

static void test_membership(void) {
    static const int32_t counts[] = {INT32_MIN, -1, 0, 1, 2, 3};
    size_t sample;

    g_reset_active_group_count = 0;
    g_reset_entry_groups = NULL;
    assert(func_80044A88(0, NULL) == 0);

    g_reset_entry_groups = groups;
    for (sample = 0; sample < 2048; ++sample) {
        size_t group, entry, excluded, value;
        for (group = 0; group < 4; ++group) {
            groups[group].count = counts[next_random() % (sizeof(counts) / sizeof(counts[0]))];
            groups[group].indices = groups[group].count > 0 ? entries[group] : NULL;
            for (entry = 0; entry < 3; ++entry) {
                uint32_t chosen = next_random() % 9;
                entries[group][entry] = chosen == 8 ? UINT32_MAX : chosen;
            }
        }
        g_reset_active_group_count = next_random() % 5;
        for (group = 0; group < g_reset_active_group_count; ++group) {
            g_reset_active_group_indices[group] = next_random() % 4;
        }
        for (excluded = 0; excluded <= 4; ++excluded) {
            for (value = 0; value < 10; ++value) {
                uint32_t object_index = value == 9 ? UINT32_MAX : (uint32_t)value;
                int expected = expected_membership(object_index, &groups[excluded]);
                assert(func_80044A88(object_index, &groups[excluded]) == expected);
            }
        }
    }

    groups[0].indices = entries[0];
    groups[0].count = 1;
    entries[0][0] = 7;
    groups[1].indices = entries[1];
    groups[1].count = 1;
    entries[1][0] = 7;
    g_reset_active_group_count = 3;
    g_reset_active_group_indices[0] = 0;
    g_reset_active_group_indices[1] = 0;
    g_reset_active_group_indices[2] = 1;
    assert(func_80044A88(7, &groups[0]) == 1);
    groups[1].count = -1;
    assert(func_80044A88(7, &groups[0]) == 0);

    g_reset_active_group_count = 1;
    g_reset_active_group_indices[0] = 0;
    groups[0].count = 1;
    groups[0].indices = NULL;
    assert(func_80044A88(1, &groups[0]) == 0);
}

static void test_dispatch_with_real_membership(uint32_t mode, int mutate) {
    size_t index;

    g_reset_entry_groups = groups;
    g_reset_active_group_count = 2;
    g_reset_active_group_indices[0] = 0;
    g_reset_active_group_indices[1] = 1;
    groups[0].indices = entries[0];
    groups[0].count = 3;
    groups[1].indices = entries[1];
    groups[1].count = 1;
    entries[1][0] = 2;
    for (index = 0; index < 3; ++index) {
        entries[0][index] = (uint32_t)index;
        objects[index] = &assets[index];
    }
    entry_table[0] = &entry_asset;
    g_reset_selected_objects = objects;
    g_reset_entry_table = entry_table;
    actor.object = &assets[1];
    actor_calls = 0;
    prepared_count = 0;
    mutate_on_first_object = mutate;

    assert(func_80044458(0, mode) == 1);
    assert(actor_calls == 1);
    assert(prepared_count == (size_t)((mutate || mode == 0) ? 3 : 2));
    assert(prepared[0] == &entry_asset && prepared[1] == &assets[0]);
    if (mutate || mode == 0) {
        assert(prepared[2] == &assets[2]);
    }
}

int main(void) {
    test_membership();
    test_dispatch_with_real_membership(1, 0);
    test_dispatch_with_real_membership(1, 1);
    test_dispatch_with_real_membership(0, 0);
    puts("reset group membership and real-dispatch integration tests passed");
    return 0;
}
