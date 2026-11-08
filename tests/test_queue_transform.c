#include "level_records.h"
#include "queue_update.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

uint8_t g_unknown_800BE024;
uint8_t g_unknown_800BE0C8;
int32_t g_unknown_800BE0D4;
ComponentRotationMatrix g_unknown_80186154;

enum {
    GET_REFERENCE,
    COPY_BASIS,
    GET_ACTOR,
    READ_TRANSLATION,
    WRITE_TRANSLATION
};

static int calls[8];
static unsigned int call_count;
static MovePreparationActor actor;
static MovePreparationComponent component;
static MovePreparationComponent replacement_component;
static ComponentRotationMatrix reference;
static ComponentRotationMatrix actor_transform;
static ComponentRotationMatrix other_transform;
static ComponentTransformView *expected_transform;
static int32_t sampled_translation[3];
static int32_t written_translation[3];
static int actor_available;
static int clear_queue_gate_on_reference;
static int clear_adjustment_gate_on_actor;
static int replace_component_on_actor;
static int replace_transform_on_read;
static int change_offset_on_read;
static int change_offset_on_write;
static int change_basis_on_copy;
static int32_t offset_from_read;
static int32_t offset_from_write;

static void record(int call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

int32_t func_800516F8(void **output) {
    record(GET_REFERENCE);
    *output = &reference;
    if (clear_queue_gate_on_reference) {
        g_unknown_800BE024 = 0;
    }
    return -1;
}

int32_t func_800549E8(
    ComponentRotationMatrix *output, const ComponentRotationMatrix *input
) {
    record(COPY_BASIS);
    assert(output == &reference);
    assert(input == &g_unknown_80186154);
    if (change_basis_on_copy) {
        g_unknown_80186154.translation[0] = 123;
        g_unknown_80186154.translation[1] = INT32_MIN;
        g_unknown_80186154.translation[2] = -456;
    }
    output->elements[0] = 0x1234;
    output->translation[0] = -999;
    return -1;
}

void *func_80028764(int32_t index) {
    record(GET_ACTOR);
    assert(index == 0);
    if (clear_adjustment_gate_on_actor) {
        g_unknown_800BE0C8 = 0;
    }
    if (replace_component_on_actor) {
        actor.component_1C = &replacement_component;
    }
    return actor_available ? &actor : 0;
}

int32_t func_80054B20(
    const ComponentTransformView *transform,
    const ComponentBasisView *basis,
    int32_t output[3]
) {
    record(READ_TRANSLATION);
    assert(transform == expected_transform);
    assert(basis == 0);
    memcpy(output, sampled_translation, sizeof(sampled_translation));
    if (change_offset_on_read) {
        g_unknown_800BE0D4 = offset_from_read;
    }
    if (replace_transform_on_read) {
        actor.component_1C->transform_14 = &actor_transform;
    }
    return -1;
}

int32_t func_80054C7C(
    ComponentTransformView *transform,
    const ComponentBasisView *basis,
    const int32_t input[3]
) {
    record(WRITE_TRANSLATION);
    assert(transform == expected_transform);
    assert(basis == 0);
    assert(g_unknown_800BE0D4 == 0);
    memcpy(written_translation, input, sizeof(written_translation));
    if (change_offset_on_write) {
        g_unknown_800BE0D4 = offset_from_write;
    }
    return -1;
}

static void reset_fixture(void) {
    memset(&actor, 0, sizeof(actor));
    memset(&component, 0, sizeof(component));
    memset(&replacement_component, 0, sizeof(replacement_component));
    memset(&reference, 0, sizeof(reference));
    memset(&g_unknown_80186154, 0, sizeof(g_unknown_80186154));
    memset(written_translation, 0, sizeof(written_translation));
    actor.component_1C = &component;
    component.transform_14 = &actor_transform;
    replacement_component.transform_14 = &other_transform;
    expected_transform = (ComponentTransformView *)(void *)&actor_transform;
    sampled_translation[0] = -100;
    sampled_translation[1] = 33;
    sampled_translation[2] = 200;
    g_unknown_800BE024 = 0;
    g_unknown_800BE0C8 = 1;
    g_unknown_800BE0D4 = 7;
    actor_available = 1;
    clear_queue_gate_on_reference = 0;
    clear_adjustment_gate_on_actor = 0;
    replace_component_on_actor = 0;
    replace_transform_on_read = 0;
    change_offset_on_read = 0;
    change_offset_on_write = 0;
    change_basis_on_copy = 0;
    offset_from_read = 0;
    offset_from_write = 0;
    call_count = 0;
}

static void expect_calls(const int *expected, unsigned int count) {
    unsigned int index;
    assert(call_count == count);
    for (index = 0; index < count; ++index) {
        assert(calls[index] == expected[index]);
    }
}

static void test_adjustment_gates(void) {
    static const int expected[] = {GET_ACTOR, READ_TRANSLATION, WRITE_TRANSLATION};
    unsigned int flag;

    for (flag = 0; flag < 256; ++flag) {
        reset_fixture();
        g_unknown_800BE0C8 = (uint8_t)flag;
        assert(func_8003ABE8() == 1);
        if (flag == 0) {
            expect_calls(0, 0);
            assert(g_unknown_800BE0D4 == 7);
        } else {
            expect_calls(expected, 3);
            assert(written_translation[0] == -100);
            assert(written_translation[1] == 26);
            assert(written_translation[2] == 200);
            assert(g_unknown_800BE0D4 == 0);
        }
    }

    reset_fixture();
    actor_available = 0;
    assert(func_8003ABE8() == 1);
    expect_calls(expected, 1);
    assert(g_unknown_800BE0D4 == 7);
}

static void test_adjustment_wrap_and_mutations(void) {
    static const int expected[] = {GET_ACTOR, READ_TRANSLATION, WRITE_TRANSLATION};
    static const int32_t cases[][2] = {
        {0, 0}, {INT32_MAX, -1}, {INT32_MIN, 1},
        {-1, INT32_MIN}, {INT32_MIN, INT32_MIN}
    };
    unsigned int index;

    for (index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        reset_fixture();
        sampled_translation[1] = cases[index][0];
        g_unknown_800BE0D4 = cases[index][1];
        assert(func_8003ABE8() == 1);
        expect_calls(expected, 3);
        assert((uint32_t)written_translation[1] ==
               (uint32_t)cases[index][0] - (uint32_t)cases[index][1]);
    }

    reset_fixture();
    clear_adjustment_gate_on_actor = 1;
    replace_component_on_actor = 1;
    expected_transform = (ComponentTransformView *)(void *)&other_transform;
    replace_transform_on_read = 1;
    change_offset_on_read = 1;
    offset_from_read = INT32_MIN;
    change_offset_on_write = 1;
    offset_from_write = 42;
    sampled_translation[1] = INT32_MAX;
    assert(func_8003ABE8() == 1);
    expect_calls(expected, 3);
    assert(g_unknown_800BE0C8 == 0);
    assert(actor.component_1C->transform_14 == &actor_transform);
    assert((uint32_t)written_translation[1] ==
           (uint32_t)INT32_MAX - (uint32_t)INT32_MIN);
    assert(g_unknown_800BE0D4 == 42);

    reset_fixture();
    component.transform_14 = 0;
    expected_transform = 0;
    assert(func_8003ABE8() == 1);
    expect_calls(expected, 3);
}

static void test_copy_and_integration(void) {
    static const int copy_only[] = {COPY_BASIS};
    static const int full[] = {
        GET_REFERENCE, COPY_BASIS, GET_ACTOR, READ_TRANSLATION, WRITE_TRANSLATION
    };
    static const int without_adjustment[] = {GET_REFERENCE, COPY_BASIS};

    reset_fixture();
    reference.padding = 0xCAFE;
    change_basis_on_copy = 1;
    assert(func_80054A6C(&reference, &g_unknown_80186154) == 0);
    expect_calls(copy_only, 1);
    assert(reference.elements[0] == 0x1234);
    assert(reference.padding == 0xCAFE);
    assert(reference.translation[0] == 123);
    assert((uint32_t)reference.translation[1] == 0x80000000u);
    assert(reference.translation[2] == -456);

    reset_fixture();
    g_unknown_800BE024 = 1;
    g_unknown_80186154.translation[0] = 11;
    g_unknown_80186154.translation[1] = 12;
    g_unknown_80186154.translation[2] = 13;
    clear_queue_gate_on_reference = 1;
    assert(func_80032D7C() == 1);
    expect_calls(full, 5);
    assert(g_unknown_800BE024 == 0);
    assert(reference.translation[0] == 11);
    assert(reference.translation[1] == -12);
    assert(reference.translation[2] == 13);
    assert(written_translation[1] == 26);

    reset_fixture();
    g_unknown_800BE024 = 1;
    g_unknown_800BE0C8 = 0;
    assert(func_80032D7C() == 1);
    expect_calls(without_adjustment, 2);
    assert(g_unknown_800BE0D4 == 7);

    reset_fixture();
    assert(func_80032D7C() == 1);
    expect_calls(0, 0);
}

int main(void) {
    test_adjustment_gates();
    test_adjustment_wrap_and_mutations();
    test_copy_and_integration();
    puts("Queue transform: gates, wrapping, callback mutations and integrated order passed.");
    return 0;
}
