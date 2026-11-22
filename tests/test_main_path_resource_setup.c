#include "game_transition.h"
#include "grounded_transform.h"
#include "input_events.h"
#include "level_records.h"
#include "reset_resources.h"
#include "texture_font_slots.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct TestObject {
    uint8_t bytes[0x40];
} TestObject;

extern void func_80023A84(void);

int32_t g_level_record_count;
GameLevelRecord *volatile g_level_records;
int32_t g_unknown_800BDEE0;
int32_t g_unknown_800BDEE4;
int32_t g_unknown_800BDEE8;
int8_t g_unknown_800BE478;
int8_t g_resource_output_800BE478;
const char g_unknown_8001211C[] = "resource";

static GameLevelRecord level_records[1];
static int32_t object_indices[1];
static int32_t object_count;
static TestObject actor;
static TestObject owner;
static TestObject component;
static TestObject reference_object;
static TestObject level_object;
static TestObject auxiliary_object;
static int32_t event_payload;
static int32_t calls[40];
static size_t call_count;
static int32_t event_count;
static int32_t transformed_direction[3];
static int32_t component_translation[3];
static int32_t transform_origin;
static int32_t transform_vertical_offset;
static int32_t transform_value;

static void record_call(int32_t call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

static void reset_fixture(void) {
    void *owner_pointer = &owner;
    void *component_pointer = &component;
    void *payload_pointer = &event_payload;

    memset(&actor, 0, sizeof(actor));
    memset(&owner, 0, sizeof(owner));
    memset(&component, 0, sizeof(component));
    memset(&reference_object, 0, sizeof(reference_object));
    memset(&level_object, 0, sizeof(level_object));
    memset(&auxiliary_object, 0, sizeof(auxiliary_object));
    memcpy(actor.bytes + 0x1C, &owner_pointer, sizeof(owner_pointer));
    memcpy(owner.bytes + 0x14, &component_pointer, sizeof(component_pointer));
    memset(level_records, 0, sizeof(level_records));
    memset(calls, 0, sizeof(calls));
    memset(transformed_direction, 0, sizeof(transformed_direction));
    memset(component_translation, 0, sizeof(component_translation));
    call_count = 0;
    event_count = 0;
    object_count = 1;
    object_indices[0] = 0;
    g_level_record_count = 1;
    g_level_records = level_records;
    g_unknown_800BDEE0 = 0x1234;
    g_unknown_800BDEE4 = 0x2345;
    g_unknown_800BDEE8 = 0x3456;
    g_unknown_800BE478 = 7;
    g_resource_output_800BE478 = 0;
    level_records[0].object_1C = &level_object;
    level_object.bytes[0x2D] = 0x40;
    memcpy(level_object.bytes + 0x0C, &payload_pointer, sizeof(payload_pointer));
}

void *func_80028764(int32_t index) {
    assert(index == 0);
    record_call(1);
    return &actor;
}

void func_8001BC08(void) { record_call(2); }
void func_8002DDFC(void) { record_call(3); }
void func_80048488(void) { record_call(4); }
void func_80028E50(void) { record_call(5); }
void func_8005FBE4(void) { record_call(6); }

void func_8002C9D4(int32_t value) {
    assert(value == 1);
    record_call(7);
}

void func_800486D0(const char *text, int32_t width, int32_t height, void *output) {
    assert(text == g_unknown_8001211C);
    assert(width == 0xC8 && height == 0xF0);
    assert(output == &g_resource_output_800BE478);
    record_call(8);
    *(int8_t *)output = 9;
    g_unknown_800BE478 = 9;
}

int32_t func_800481D4(int32_t tag, int32_t width, int32_t height) {
    assert(tag == 9);
    assert(width == 0x1000 && height == 0x1000);
    record_call(9);
    return 0;
}

int32_t func_800483C0(int32_t tag) {
    assert(tag == 9);
    record_call(10);
    return 0;
}

void func_8002C370(void) { record_call(11); }
void func_8002BCD4(void) { record_call(12); }

void func_8002BAD4(int32_t index) {
    assert(index == 0xF);
    record_call(13);
}

void func_8002D778(void) { record_call(14); }

int32_t func_800516F8(void **output) {
    record_call(15);
    *output = &reference_object;
    return 0;
}

int32_t func_80054AF0(const int16_t *input, int32_t *output) {
    assert((const void *)input == &component);
    record_call(16);
    output[0] = 11;
    output[1] = -22;
    output[2] = 33;
    return 0;
}

int32_t func_80054B20(void *input, int32_t mode, int32_t *output) {
    assert(input == &component && mode == 0);
    record_call(17);
    output[0] = 100;
    output[1] = 200;
    output[2] = 300;
    return 0;
}

void func_8002E858(
    const GroundedTransformVector *direction,
    int32_t origin,
    uint32_t vertical_offset,
    GroundedTransformVector *output
) {
    assert(direction->values[0] == 11 && direction->values[1] == -22 && direction->values[2] == 33);
    record_call(18);
    transform_origin = origin;
    transform_vertical_offset = vertical_offset;
    output->values[0] = 7;
    output->values[1] = 8;
    output->values[2] = 9;
}

int32_t func_80054C7C(void *target, int32_t mode, const int32_t *input) {
    assert(target == &reference_object && mode == 0);
    record_call(19);
    memcpy(transformed_direction, input, sizeof(transformed_direction));
    return 0;
}

void func_8002E954(GroundedTransformVector *translation, uint32_t value) {
    record_call(20);
    memcpy(component_translation, translation->values, sizeof(component_translation));
    transform_value = (int32_t)value;
}

void func_8002EA80(void) { record_call(21); }
void func_80045094(void) { record_call(22); }
void func_8001B0A0(void) { record_call(23); }

void func_800430E8(int32_t **indices, int32_t *count) {
    record_call(24);
    *indices = object_indices;
    *count = object_count;
}

void *func_80040FC0(int32_t index) {
    assert(index == 1);
    return &auxiliary_object;
}

void func_8001B558(
    int32_t event_id,
    int32_t mode,
    uintptr_t flags,
    int32_t heading,
    void *argument_4,
    void *argument_5,
    void *argument_6,
    void *argument_7
) {
    assert(event_id == 0x31 && mode == 5 && flags == 0xFFFE);
    assert((uint32_t)heading == (uint32_t)(uintptr_t)&event_payload);
    assert(argument_4 == 0 && argument_5 == 0);
    assert(argument_6 == 0 && argument_7 == 0);
    record_call(25);
    event_count++;
}

void func_8005F374(void) { record_call(26); }

static void test_resource_transform_and_event_path(void) {
    const int32_t expected_calls[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 23, 23, 23, 24, 25, 26
    };

    reset_fixture();
    func_80023A84();

    assert(transform_origin == g_unknown_800BDEE0);
    assert(transform_vertical_offset == g_unknown_800BDEE4 + 200);
    assert(transformed_direction[0] == 7);
    assert(transformed_direction[1] == 8);
    assert(transformed_direction[2] == 9);
    assert(component_translation[0] == 100);
    assert(component_translation[1] == 200);
    assert(component_translation[2] == 300);
    assert(transform_value == g_unknown_800BDEE8);
    assert(event_count == 1);
    assert(call_count == sizeof(expected_calls) / sizeof(expected_calls[0]));
    assert(memcmp(calls, expected_calls, sizeof(expected_calls)) == 0);
}

static void test_empty_object_scan(void) {
    const int32_t expected_calls[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 23, 23, 23, 24, 26
    };

    reset_fixture();
    object_count = 0;
    func_80023A84();

    assert(event_count == 0);
    assert(call_count == sizeof(expected_calls) / sizeof(expected_calls[0]));
    assert(memcmp(calls, expected_calls, sizeof(expected_calls)) == 0);
}

static void test_auxiliary_object_scan(void) {
    void *payload_pointer = &event_payload;

    reset_fixture();
    object_indices[0] = 1;
    auxiliary_object.bytes[0x2D] = 0x40;
    memcpy(auxiliary_object.bytes + 0x0C, &payload_pointer, sizeof(payload_pointer));
    func_80023A84();
    assert(event_count == 1);

    reset_fixture();
    object_indices[0] = 1;
    func_80023A84();
    assert(event_count == 0);
}

int main(void) {
    test_resource_transform_and_event_path();
    test_empty_object_scan();
    test_auxiliary_object_scan();
    puts("main-path resource setup tests passed");
    return 0;
}
