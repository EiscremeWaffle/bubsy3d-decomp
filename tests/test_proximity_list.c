#include "proximity_list.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(offsetof(ProximityPosition, x) == 0x14, "position X offset");
_Static_assert(offsetof(ProximityPosition, z) == 0x1C, "position Z offset");
_Static_assert(offsetof(ProximityObject, position) == 0x14, "object position offset");
_Static_assert(offsetof(ProximityObject, radius) == 0x1E, "object radius offset");
_Static_assert(offsetof(ProximityNode, next) == 0x08, "node next offset");
#endif

uint16_t g_proximity_count;
ProximityObject *g_proximity_objects[32768];
const char g_proximity_limit_condition[] = "proximity limit";
const char g_proximity_source_path[] = "proximity source";

static ProximityPosition reference_position;
static ProximityObject reference_object;
static ProximityObject sentinel_object;
static ProximityObject *reference;
static ProximityNode *list_head;
static ProximityNode **live_list_slot;
static ProximityNode *replacement_current;
static ProximityNode *replacement_next;
static int initial_count;
static int mutate_reference_at_list;
static int assertion_mutation;
static uint16_t replacement_count;
static unsigned int reference_calls;
static unsigned int list_calls;
static unsigned int assertion_calls;
static uint16_t assertion_counts[4];

ProximityObject *func_8005327C(void) {
    ++reference_calls;
    g_proximity_count = 0xA55A;
    return reference;
}

int32_t func_800529D0(ProximityNode **output) {
    ++list_calls;
    assert(reference_calls == 1 && g_proximity_count == 0);
    live_list_slot = output;
    *output = list_head;
    if (initial_count >= 0) {
        g_proximity_count = (uint16_t)initial_count;
    }
    if (mutate_reference_at_list) {
        reference_position.x = 100000;
        reference_position.z = -100000;
    }
    return 0;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 1 && condition == g_proximity_limit_condition);
    assert(source == g_proximity_source_path && line == 0x342);
    assert(assertion_calls < sizeof(assertion_counts) / sizeof(assertion_counts[0]));
    assertion_counts[assertion_calls++] = g_proximity_count;
    if (assertion_mutation != 0) {
        assert(g_proximity_count == 40);
        assert(g_proximity_objects[39] == list_head->object);
        if (assertion_mutation == 1) {
            *live_list_slot = replacement_current;
        } else {
            (*live_list_slot)->next = replacement_next;
        }
        g_proximity_count = replacement_count;
        reference_position.x = 100000;
        reference_position.z = -100000;
        assertion_mutation = 0;
    }
}

static void initialize(ProximityObject *reference_value, ProximityNode *head) {
    reference = reference_value;
    list_head = head;
    live_list_slot = 0;
    replacement_current = 0;
    replacement_next = 0;
    initial_count = -1;
    mutate_reference_at_list = 0;
    assertion_mutation = 0;
    replacement_count = 0;
    reference_calls = 0;
    list_calls = 0;
    assertion_calls = 0;
    g_proximity_count = 0xFFFF;
    g_proximity_objects[0] = &sentinel_object;
    g_proximity_objects[5] = &sentinel_object;
    g_proximity_objects[38] = &sentinel_object;
    g_proximity_objects[39] = &sentinel_object;
    g_proximity_objects[40] = &sentinel_object;
    g_proximity_objects[32767] = &sentinel_object;
}

static void check_calls(void) {
    assert(reference_calls == 1 && list_calls == 1);
}

static int64_t signed_word(uint32_t value) {
    return value < UINT32_C(0x80000000) ? (int64_t)value
                                        : (int64_t)value - INT64_C(0x100000000);
}

static uint32_t wrapped_absolute_difference(int32_t left, int32_t right) {
    uint32_t difference = (uint32_t)left - (uint32_t)right;
    return difference & UINT32_C(0x80000000) ? 0u - difference : difference;
}

static int expected_nearby(int32_t x, int32_t z, int32_t reference_x,
                           int32_t reference_z, int16_t radius) {
    uint32_t distance = wrapped_absolute_difference(x, reference_x) +
                        wrapped_absolute_difference(z, reference_z);
    return radius != 0 && signed_word(distance) < (int64_t)radius;
}

static void test_empty_and_null_reference(void) {
    ProximityNode second = {0};
    ProximityNode first = {0};

    reference_object.position = &reference_position;
    initialize(&reference_object, 0);
    func_80069758();
    check_calls();
    assert(g_proximity_count == 0 && assertion_calls == 0);
    assert(g_proximity_objects[0] == &sentinel_object);

    first.next = &second;
    initialize(0, &first);
    initial_count = 23;
    func_80069758();
    check_calls();
    assert(g_proximity_count == 23 && assertion_calls == 0);
    assert(g_proximity_objects[0] == &sentinel_object);
}

static void test_distance_boundaries(void) {
    static const int32_t coordinates[] = {
        INT32_MIN, INT32_MIN + 1, -32768, -1, 0, 1, 32767, INT32_MAX
    };
    static const int16_t radii[] = {INT16_MIN, -1, 0, 1, 2, INT16_MAX};
    ProximityPosition position = {0};
    ProximityObject object = {0};
    ProximityNode node = {0};
    size_t rx, rz, x, z, radius;

    reference_object.position = &reference_position;
    object.position = &position;
    node.object = &object;
    for (rx = 0; rx < sizeof(coordinates) / sizeof(coordinates[0]); ++rx) {
        for (rz = 0; rz < sizeof(coordinates) / sizeof(coordinates[0]); ++rz) {
            for (x = 0; x < sizeof(coordinates) / sizeof(coordinates[0]); ++x) {
                for (z = 0; z < sizeof(coordinates) / sizeof(coordinates[0]); ++z) {
                    for (radius = 0; radius < sizeof(radii) / sizeof(radii[0]); ++radius) {
                        int expected;
                        reference_position.x = coordinates[rx];
                        reference_position.z = coordinates[rz];
                        position.x = coordinates[x];
                        position.z = coordinates[z];
                        object.radius = radii[radius];
                        expected = expected_nearby(position.x, position.z,
                                                   reference_position.x, reference_position.z,
                                                   object.radius);
                        initialize(&reference_object, &node);
                        func_80069758();
                        check_calls();
                        assert(g_proximity_count == (uint16_t)expected && assertion_calls == 0);
                        assert(g_proximity_objects[0] == (expected ? &object : &sentinel_object));
                    }
                }
            }
        }
    }
}

static void test_count_thresholds_and_cached_reference(void) {
    static const int counts[] = {38, 39, 40, 32767};
    ProximityPosition position = {0};
    ProximityObject object = {0};
    ProximityNode node = {0};
    size_t index;

    reference_object.position = &reference_position;
    object.position = &position;
    object.radius = 1;
    node.object = &object;
    for (index = 0; index < sizeof(counts) / sizeof(counts[0]); ++index) {
        reference_position.x = position.x = 17;
        reference_position.z = position.z = -9;
        initialize(&reference_object, &node);
        initial_count = counts[index];
        mutate_reference_at_list = 1;
        func_80069758();
        check_calls();
        assert(g_proximity_count == (uint16_t)(counts[index] + 1));
        assert(g_proximity_objects[counts[index]] == &object);
        assert(assertion_calls == (unsigned int)(counts[index] == 39 || counts[index] == 40));
        if (assertion_calls != 0) {
            assert(assertion_counts[0] == (uint16_t)(counts[index] + 1));
        }
    }
}

static void test_assertion_mutations(void) {
    ProximityPosition positions[3] = {{0}};
    ProximityObject objects[3] = {{0}};
    ProximityNode first = {0}, ignored = {0}, replacement = {0};
    ProximityNode other_current = {0};
    size_t index;
    int mode;

    reference_object.position = &reference_position;
    for (index = 0; index < 3; ++index) {
        positions[index].x = 17;
        positions[index].z = -9;
        objects[index].position = &positions[index];
        objects[index].radius = 1;
    }
    first.object = &objects[0];
    ignored.object = &objects[1];
    replacement.object = &objects[2];
    other_current.next = &replacement;

    for (mode = 1; mode <= 2; ++mode) {
        first.next = &ignored;
        reference_position.x = 17;
        reference_position.z = -9;
        initialize(&reference_object, &first);
        initial_count = 39;
        assertion_mutation = mode;
        replacement_count = 5;
        replacement_current = &other_current;
        replacement_next = &replacement;
        func_80069758();
        check_calls();
        assert(assertion_calls == 1 && assertion_counts[0] == 40);
        assert(g_proximity_count == 6);
        assert(g_proximity_objects[39] == &objects[0]);
        assert(g_proximity_objects[5] == &objects[2]);
    }
}

int main(void) {
    test_empty_and_null_reference();
    test_distance_boundaries();
    test_count_thresholds_and_cached_reference();
    test_assertion_mutations();
    puts("proximity-list tests passed");
    return 0;
}
