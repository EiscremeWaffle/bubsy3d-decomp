#include "actor_context.h"
#include "component_rotation.h"
#include "game_transition.h"
#include "grounded_case37.h"
#include "level_records.h"
#include "movement_reset.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(offsetof(GroundedCase37ComponentView, transform_14) == 0x14, "transform offset");
_Static_assert(offsetof(GroundedCase37ComponentView, owner_1C) == 0x1C, "owner offset");
#endif

int32_t g_level_record_count;
GameLevelRecord *volatile g_level_records;
const ComponentRotationMatrix g_level_default_rotation = {
    {0x1000, 0, 0, 0, 0x1000, 0, 0, 0, 0x1000}, 0, {0, 0, 0}
};

static GameLevelRecord records[3];
static GameLevelPosition positions[3];
static GroundedCase37ComponentView objects[3], owners[3];
static ComponentRotationMatrix transforms[3];
static uint8_t context[0x0D];
static int32_t expected_index;
static int32_t count_after_getter;
static int change_count;
static int mutate_on_translation;
static unsigned calls[8], call_count;
static const int32_t translation[3] = {INT32_MIN, 0x12345678, -1};

static void record_call(unsigned call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

void *func_80026648(void *actor, void *unused) {
    (void)unused;
    record_call(1);
    assert((uintptr_t)actor == (uintptr_t)(uint32_t)expected_index);
    if (change_count) {
        g_level_record_count = count_after_getter;
    }
    return context;
}

void *func_80040FC0(int32_t index) {
    (void)index;
    assert(!"unexpected fallback in native tests");
    return NULL;
}

int32_t func_80054B20(void *component, int32_t mode, int32_t *output) {
    record_call(2);
    assert(component == &transforms[expected_index]);
    assert(mode == 0);
    assert(records[expected_index].flag_20 == 0);
    memcpy(output, translation, sizeof(translation));
    if (mutate_on_translation) {
        context[0x0C] = 0;
        records[expected_index].flag_20 = 0x5A;
        g_level_record_count = 0;
    }
    return 0x18;
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    record_call(3);
    assert(event_id == 0x1A && mode == 4 && flags == 0xFFFE && heading == 0xFFFE);
    assert(*(int32_t *)argument_4 == 11);
    assert((uint8_t *)argument_6 == (uint8_t *)argument_5 + sizeof(int32_t));
    assert((uint8_t *)argument_7 == (uint8_t *)argument_6 + sizeof(int32_t));
    assert(*(int32_t *)argument_5 == translation[0]);
    assert(*(int32_t *)argument_6 == translation[1]);
    assert(*(int32_t *)argument_7 == translation[2]);
}

struct ComponentTransformView;
struct ComponentBasisView;

int32_t func_80054C7C(
    struct ComponentTransformView *component,
    const struct ComponentBasisView *basis,
    const int32_t input[3]
) {
    record_call(4);
    assert((void *)component == &transforms[expected_index]);
    assert(basis == NULL);
    assert(memcmp(input, positions[expected_index].words, 3 * sizeof(*input)) == 0);
    return 0;
}

int32_t func_800549E8(ComponentRotationMatrix *component, const ComponentRotationMatrix *basis) {
    record_call(5);
    assert(component == &transforms[expected_index]);
    assert(memcmp(basis->elements, g_level_default_rotation.elements, sizeof(basis->elements)) == 0);
    assert(memcmp(basis->translation, positions[expected_index].words, sizeof(basis->translation)) == 0);
    return 0;
}

static void reset(void) {
    unsigned index;

    memset(records, 0, sizeof(records));
    memset(positions, 0, sizeof(positions));
    memset(objects, 0, sizeof(objects));
    memset(owners, 0, sizeof(owners));
    memset(transforms, 0, sizeof(transforms));
    memset(context, 0, sizeof(context));
    for (index = 0; index < 3; index++) {
        records[index].object_1C = &objects[index];
        records[index].position_0C = &positions[index];
        records[index].flag_20 = 0xA5;
        objects[index].owner_1C = &owners[index];
        owners[index].transform_14 = &transforms[index];
        positions[index].words[0] = (int32_t)index - 77;
        positions[index].words[1] = (int32_t)index + 123;
        positions[index].words[2] = INT32_MAX - (int32_t)index;
    }
    g_level_record_count = 3;
    g_level_records = records;
    context[0x0C] = 1;
    change_count = 0;
    mutate_on_translation = 0;
    call_count = 0;
}

static void test_invalid_indices(void) {
    static const int32_t invalid[] = {-1, INT32_MIN, 3, INT32_MAX};
    unsigned index;

    for (index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        reset();
        expected_index = invalid[index];
        assert(func_800288D8(expected_index) == 0);
        assert(call_count == 1 && calls[0] == 1);
        assert(records[0].flag_20 == 0xA5 && records[1].flag_20 == 0xA5);
        assert(records[2].flag_20 == 0xA5);
    }
}

static void test_valid_indices_and_context(void) {
    unsigned flag;

    for (expected_index = 0; expected_index < 3; expected_index++) {
        for (flag = 0; flag < 256; flag++) {
            reset();
            records[expected_index].flag_20 = (uint8_t)flag;
            context[0x0C] = 0;
            records[expected_index].object_1C = NULL;
            assert(func_800288D8(expected_index) == 1);
            assert(records[expected_index].flag_20 == 0);
            assert(call_count == 1 && calls[0] == 1);
        }
        reset();
        assert(func_800288D8(expected_index) == 1);
        assert(records[expected_index].flag_20 == 0);
        assert(call_count == 3 && calls[0] == 1 && calls[1] == 2 && calls[2] == 3);
    }
}

static void test_callback_mutations(void) {
    reset();
    expected_index = 1;
    g_level_record_count = 0;
    change_count = 1;
    count_after_getter = 3;
    mutate_on_translation = 1;
    assert(func_800288D8(expected_index) == 1);
    assert(call_count == 3 && calls[0] == 1 && calls[1] == 2 && calls[2] == 3);
    assert(records[1].flag_20 == 0x5A);
    assert(context[0x0C] == 0 && g_level_record_count == 0);

    reset();
    expected_index = 1;
    change_count = 1;
    count_after_getter = 0;
    assert(func_800288D8(expected_index) == 0);
    assert(call_count == 1 && calls[0] == 1 && records[1].flag_20 == 0xA5);
}

static void test_movement_reset_integration(void) {
    MovementResetObject object;
    MovementResetComponent component;

    reset();
    expected_index = 2;
    memset(&object, 0, sizeof(object));
    memset(&component, 0, sizeof(component));
    object.record_id_0C = expected_index;
    component.transform_14 = &transforms[expected_index];
    component.field_10 = 0xDEADBEEF;
    component.flags_04 = 0xFF;
    func_80028600(&object, &component);
    assert(call_count == 5);
    assert(calls[0] == 4 && calls[1] == 5 && calls[2] == 1);
    assert(calls[3] == 2 && calls[4] == 3);
    assert(component.field_10 == 0 && component.flags_04 == 0xDF);
    assert(records[expected_index].flag_20 == 0);
}

int main(void) {
    test_invalid_indices();
    test_valid_indices_and_context();
    test_callback_mutations();
    test_movement_reset_integration();
    puts("Level-record reset: bounds, event, mutations and movement integration passed");
    return 0;
}
