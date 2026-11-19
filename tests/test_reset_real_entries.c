#include "reset_entry_helpers.h"
#include "reset_resources.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

uint8_t g_reset_active;
uint8_t g_reset_secondary_flag;
uint32_t g_reset_entry_count;
RuntimeResetEntry g_reset_entries[2];
int32_t g_reset_clear_count;
uint32_t g_reset_clear_words[2];
ResetTrackedNode *g_reset_tracked_head;
void **g_reset_entry_table;
ProximityObject *g_reference_object;
ProximityNode *g_object_list_head;
uint16_t g_proximity_count;
ProximityObject *g_proximity_objects[40];
const char g_proximity_limit_condition[] = "proximity condition";
const char g_proximity_source_path[] = "proximity source";

enum CallKind { SETUP, PREPARE_0, SELECT_0, REMOVE_0, PREPARE_1, SELECT_1,
                REMOVE_2, TICK, FINALIZE };
static enum CallKind calls[16];
static size_t call_count;
static uint8_t assets[2];
static ResetTrackedNode tracked_nodes[3];
static int mutate_entry;

static void record(enum CallKind kind) {
    assert(g_reset_active == 1);
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = kind;
}

void func_8004380C(void) {
    record(SETUP);
}

int32_t func_80052BDC(void *object, int32_t mode) {
    assert(mode == 1);
    if (object == &assets[0]) {
        record(PREPARE_0);
        if (mutate_entry) {
            g_reset_entries[0].word = 1;
        }
    } else {
        assert(object == &assets[1]);
        record(PREPARE_1);
    }
    return 0;
}

int32_t func_80044B24(uint32_t index, uint32_t mode) {
    assert(mode == 0 && index < 2);
    record(index == 0 ? SELECT_0 : SELECT_1);
    return -1;
}

void func_80042170(ResetTrackedNode *node) {
    if (node == &tracked_nodes[0]) {
        record(REMOVE_0);
    } else {
        assert(node == &tracked_nodes[2]);
        record(REMOVE_2);
    }
}

uint32_t func_800439EC(void) {
    assert(g_reset_secondary_flag == 0);
    assert(g_reset_clear_words[0] == 0 && g_reset_clear_words[1] == 0);
    record(TICK);
    return 0;
}

void func_80044268(void) {
    record(FINALIZE);
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(0 && "unexpected assertion callback");
}

int main(void) {
    static const enum CallKind expected_prefix[] = {
        SETUP, PREPARE_0, SELECT_0, REMOVE_0, PREPARE_1, SELECT_1,
        REMOVE_2, TICK, TICK, FINALIZE
    };
    ProximityPosition reference_position = {0}, nearby_position = {0};
    ProximityObject reference = {0}, other_reference = {0}, nearby_object = {0};
    ProximityNode nearby_node = {0};
    ResetTrackedObject tracked_objects[3] = {{0}};
    void *table[2] = {&assets[0], &assets[1]};
    size_t index;
    int mutation;

    reference.position = &reference_position;
    nearby_position.x = 1;
    nearby_object.position = &nearby_position;
    nearby_object.radius = 2;
    nearby_node.object = &nearby_object;
    tracked_objects[0].reference = &other_reference;
    tracked_objects[1].reference = &reference;
    tracked_objects[2].reference = &other_reference;
    for (index = 0; index < 3; ++index) {
        tracked_nodes[index].object = &tracked_objects[index];
        tracked_nodes[index].tag = index == 0 ? 0 : 1;
        tracked_nodes[index].next = index < 2 ? &tracked_nodes[index + 1] : 0;
    }

    for (mutation = 0; mutation <= 1; ++mutation) {
        mutate_entry = mutation;
        call_count = 0;
        g_reset_active = 0;
        g_reset_secondary_flag = 1;
        g_reset_entry_count = 2;
        g_reset_entries[0].word = 0;
        g_reset_entries[1].word = 1;
        g_reset_clear_count = 2;
        g_reset_clear_words[0] = 123;
        g_reset_clear_words[1] = 456;
        g_reset_tracked_head = &tracked_nodes[0];
        g_reset_entry_table = table;
        g_reference_object = &reference;
        g_object_list_head = &nearby_node;
        g_proximity_count = 0xFFFF;
        g_proximity_objects[0] = 0;

        func_80045094();
        assert(call_count == sizeof(expected_prefix) / sizeof(expected_prefix[0]));
        for (index = 0; index < call_count; ++index) {
            enum CallKind expected = expected_prefix[index];
            if (mutation && index == 3) {
                expected = REMOVE_2;
            }
            assert(calls[index] == expected);
        }
        assert(g_reset_active == 0 && g_reset_secondary_flag == 0);
        assert(g_reset_entries[0].word == (uint32_t)mutation);
        assert(g_proximity_count == 1 && g_proximity_objects[0] == &nearby_object);
    }
    puts("reset integration with real entry and proximity helpers passed");
    return 0;
}
