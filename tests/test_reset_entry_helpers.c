#include "reset_entry_helpers.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(offsetof(ResetTrackedObject, flags) == 0x10, "tracked flags offset");
_Static_assert(offsetof(ResetTrackedObject, reference) == 0x1C, "tracked reference offset");
_Static_assert(offsetof(ResetTrackedNode, tag) == 0x04, "tracked tag offset");
_Static_assert(offsetof(ResetTrackedNode, next) == 0x0C, "tracked next offset");
#endif

ResetTrackedNode *g_reset_tracked_head;
void **g_reset_entry_table;
ProximityObject *g_reference_object;

static ProximityObject reference_a, reference_b;
static ResetTrackedNode *reference_getter_replacement;
static ResetTrackedNode *captured_calls[8];
static ResetTrackedNode *first_node;
static ResetTrackedNode *alternate_node;
static ResetTrackedObject *third_object;
static unsigned int getter_calls, removal_calls;
static int mutate_reference_getter, mutate_first_removal;
static void *selected_object;
static uint32_t selected_index, selected_mode;
static int32_t backend_result;
static void **replacement_table;
static int mutate_entry_table;
static unsigned int prepare_calls, select_calls;

ProximityObject *func_8005327C(void) {
    ++getter_calls;
    if (mutate_reference_getter) {
        g_reset_tracked_head = reference_getter_replacement;
    }
    return g_reference_object;
}

void func_80042170(ResetTrackedNode *node) {
    assert(removal_calls < sizeof(captured_calls) / sizeof(captured_calls[0]));
    captured_calls[removal_calls++] = node;
    if (mutate_first_removal && node == first_node) {
        node->next = alternate_node;
        g_reference_object = &reference_b;
        third_object->flags = 0;
        mutate_first_removal = 0;
    }
}

int32_t func_80052BDC(void *object, int32_t mode) {
    ++prepare_calls;
    assert(mode == 1);
    selected_object = object;
    if (mutate_entry_table) {
        g_reset_entry_table = replacement_table;
    }
    return 0;
}

int32_t func_80044B24(uint32_t index, uint32_t mode) {
    ++select_calls;
    assert(prepare_calls == select_calls);
    selected_index = index;
    selected_mode = mode;
    return backend_result;
}

static void test_tracked_tag_and_gates(void) {
    ResetTrackedObject object = {0};
    ResetTrackedNode node = {0};
    uint32_t bits;

    getter_calls = 0;
    g_reset_tracked_head = 0;
    g_reference_object = &reference_a;
    func_8004232C(0);
    assert(getter_calls == 1 && removal_calls == 0);

    object.reference = &reference_b;
    node.object = &object;
    for (bits = 0; bits <= 0xFFFF; ++bits) {
        int32_t signed_tag = bits < 0x8000 ? (int32_t)bits : (int32_t)bits - 0x10000;
        node.tag = (int16_t)signed_tag;
        g_reset_tracked_head = &node;
        getter_calls = removal_calls = 0;
        func_8004232C((int32_t)bits + 0x10000);
        assert(getter_calls == 1 && removal_calls == 1 && captured_calls[0] == &node);
    }

    node.tag = -1;
    object.reference = &reference_a;
    g_reset_tracked_head = &node;
    getter_calls = removal_calls = 0;
    func_8004232C(-1);
    assert(getter_calls == 1 && removal_calls == 0);
    object.reference = &reference_b;
    object.flags = 0x200000;
    func_8004232C(-1);
    assert(getter_calls == 2 && removal_calls == 0);
    object.flags = 0;
    func_8004232C(0);
    assert(getter_calls == 3 && removal_calls == 0);
    node.object = 0;
    func_8004232C(0);
    assert(getter_calls == 4 && removal_calls == 0);
}

static void test_tracked_callback_mutations(void) {
    ResetTrackedObject objects[4] = {{0}};
    ResetTrackedNode nodes[4] = {{0}};
    size_t index;

    for (index = 0; index < 4; ++index) {
        objects[index].reference = &reference_b;
        nodes[index].object = &objects[index];
        nodes[index].tag = 7;
    }
    nodes[0].next = &nodes[1];
    nodes[1].next = &nodes[2];
    objects[1].reference = &reference_a;
    objects[2].flags = 0x200000;
    first_node = &nodes[0];
    alternate_node = &nodes[3];
    third_object = &objects[2];
    reference_getter_replacement = alternate_node;
    g_reset_tracked_head = first_node;
    g_reference_object = &reference_a;
    getter_calls = removal_calls = 0;
    mutate_reference_getter = 1;
    mutate_first_removal = 1;
    func_8004232C(7);
    assert(getter_calls == 1 && removal_calls == 2);
    assert(captured_calls[0] == &nodes[0] && captured_calls[1] == &nodes[2]);
    assert(g_reset_tracked_head == &nodes[3] && nodes[0].next == &nodes[3]);
    assert(g_reference_object == &reference_b && objects[2].flags == 0);
    mutate_reference_getter = 0;
}

static void test_entry_dispatch(void) {
    uint8_t assets[4], alternatives[4];
    void *table[4], *other_table[4];
    uint32_t mode, index;

    for (index = 0; index < 4; ++index) {
        table[index] = &assets[index];
        other_table[index] = &alternatives[index];
    }
    replacement_table = other_table;
    prepare_calls = select_calls = 0;
    for (mode = 0; mode < 512; ++mode) {
        index = mode % 4;
        backend_result = (int32_t)((mode << 16) | (255u - (mode & 0xFFu)));
        g_reset_entry_table = table;
        mutate_entry_table = 1;
        assert(func_80044458(index, mode) == (uint8_t)backend_result);
        assert(selected_object == &assets[index]);
        assert(selected_index == index && selected_mode == (mode & 0xFFu));
        assert(g_reset_entry_table == other_table);
    }
    g_reset_entry_table = table;
    mutate_entry_table = 0;
    backend_result = -1;
    assert(func_80044458(2, UINT32_MAX) == 0xFF);
    assert(selected_object == &assets[2] && selected_index == 2 && selected_mode == 0xFF);
    assert(prepare_calls == select_calls && prepare_calls == 513);
}

int main(void) {
    test_tracked_tag_and_gates();
    test_tracked_callback_mutations();
    test_entry_dispatch();
    puts("reset-entry helper tests passed");
    return 0;
}
