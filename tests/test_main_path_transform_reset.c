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
#include <stdio.h>
#include <string.h>

typedef struct TestObject {
    uint8_t bytes[0x40];
} TestObject;

typedef struct TestEvent {
    int32_t event_id;
    int32_t mode;
    uintptr_t flags;
    int32_t heading;
    void *arguments[4];
} TestEvent;

extern int32_t func_80054AF0(const int16_t *component, int32_t *output);
extern int32_t func_80054B20(void *component, int32_t mode, int32_t *output);
extern int32_t func_80054C7C(void *component, int32_t mode, const int32_t *input);
extern int32_t func_800516F8(void **output);
extern void func_80023C98(void);

int32_t g_level_record_count;
GameLevelRecord *volatile g_level_records;
int32_t g_unknown_800BDEE0;
int32_t g_unknown_800BDEE4;
int32_t g_unknown_800BDEE8;
int8_t g_unknown_800BE478;
uint8_t g_unknown_801D5F7C;

static GameLevelRecord level_records[1];
static int32_t object_indices[3];
static int32_t object_count;
static TestObject actor_object;
static TestObject owner_object;
static TestObject component_object;
static TestObject reference_object;
static TestObject level_object;
static TestObject auxiliary_objects[3];
static int32_t event_payloads[2];
static int32_t call_log[64];
static size_t call_count;
static TestEvent events[4];
static size_t event_count;
static int32_t transformed_direction[3];
static int32_t component_translation[3];
static int32_t transform_origin;
static int32_t transform_vertical_offset;
static int32_t transform_value;

static void record_call(int32_t call) {
    assert(call_count < sizeof(call_log) / sizeof(call_log[0]));
    call_log[call_count++] = call;
}

static void set_object(TestObject *object, uint8_t flags, void *payload) {
    memset(object, 0, sizeof(*object));
    object->bytes[0x2D] = flags;
    memcpy(object->bytes + 0x0C, &payload, sizeof(payload));
}

static void reset_fixture(void) {
    void *owner = &owner_object;
    void *component = &component_object;

    memset(&actor_object, 0, sizeof(actor_object));
    memset(&owner_object, 0, sizeof(owner_object));
    memcpy(actor_object.bytes + 0x1C, &owner, sizeof(owner));
    memcpy(owner_object.bytes + 0x14, &component, sizeof(component));
    memset(level_records, 0, sizeof(level_records));
    memset(auxiliary_objects, 0, sizeof(auxiliary_objects));
    memset(events, 0, sizeof(events));
    memset(call_log, 0, sizeof(call_log));
    memset(transformed_direction, 0, sizeof(transformed_direction));
    memset(component_translation, 0, sizeof(component_translation));
    call_count = 0;
    event_count = 0;
    object_count = 0;
    g_level_record_count = 1;
    g_level_records = level_records;
    g_unknown_800BDEE0 = 0x1234;
    g_unknown_800BDEE4 = 0x2345;
    g_unknown_800BDEE8 = 0x3456;
    g_unknown_800BE478 = 7;
    g_unknown_801D5F7C = 1;
    level_records[0].object_1C = &level_object;
}

void *func_80028764(int32_t index) {
    assert(index == 0);
    record_call(1);
    return &actor_object;
}

void func_80028E20(void) {
    record_call(2);
}

int32_t func_800516F8(void **output) {
    record_call(3);
    *output = &reference_object;
    return 0;
}

int32_t func_80054AF0(const int16_t *component, int32_t *output) {
    assert((const void *)component == &component_object);
    record_call(4);
    output[0] = 11;
    output[1] = -22;
    output[2] = 33;
    return 0;
}

int32_t func_80054B20(void *component, int32_t mode, int32_t *output) {
    assert(component == &component_object);
    assert(mode == 0);
    record_call(5);
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
    assert(direction->values[0] == -11 && direction->values[1] == 22 && direction->values[2] == -33);
    record_call(6);
    transform_origin = origin;
    transform_vertical_offset = vertical_offset;
    output->values[0] = 7;
    output->values[1] = 8;
    output->values[2] = 9;
}

int32_t func_80054C7C(void *component, int32_t mode, const int32_t *input) {
    assert(component == &reference_object);
    assert(mode == 0);
    record_call(7);
    memcpy(transformed_direction, input, sizeof(transformed_direction));
    return 0;
}

void func_8002E954(GroundedTransformVector *translation, uint32_t value) {
    record_call(8);
    memcpy(component_translation, translation->values, sizeof(component_translation));
    transform_value = (int32_t)value;
}

void func_8002EA80(void) { record_call(9); }
void func_80045094(void) { record_call(10); }
void func_8005F398(void) { record_call(11); }

int32_t func_80048504(int32_t value) {
    assert(value == g_unknown_800BE478);
    record_call(12);
    return 0;
}

int32_t func_80048C48(int32_t value) {
    assert(value == g_unknown_800BE478);
    record_call(13);
    return 0;
}

void func_8002C488(void) { record_call(14); }
void func_8002D8D0(void) { record_call(15); }

void func_8002C9D4(int32_t value) {
    assert(value == 0);
    record_call(16);
}

void func_80048488(void) { record_call(17); }
void func_8002C370(void) { record_call(18); }

void func_800430E8(int32_t **indices, int32_t *count) {
    record_call(19);
    *indices = object_indices;
    *count = object_count;
}

void *func_80040FC0(int32_t index) {
    assert(index > 0 && index < 3);
    record_call(20 + index);
    return &auxiliary_objects[index];
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
    assert(event_count < sizeof(events) / sizeof(events[0]));
    record_call(30);
    events[event_count].event_id = event_id;
    events[event_count].mode = mode;
    events[event_count].flags = flags;
    events[event_count].heading = heading;
    events[event_count].arguments[0] = argument_4;
    events[event_count].arguments[1] = argument_5;
    events[event_count].arguments[2] = argument_6;
    events[event_count].arguments[3] = argument_7;
    event_count++;
}

void func_8001BC28(void) { record_call(31); }

static void test_transform_and_queued_events(void) {
    const int32_t expected_calls[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 30, 21, 30, 22, 31
    };

    reset_fixture();
    object_indices[0] = 0;
    object_indices[1] = 1;
    object_indices[2] = 2;
    object_count = 3;
    set_object(&level_object, 0x40, &event_payloads[0]);
    set_object(&auxiliary_objects[1], 0x40, &event_payloads[1]);
    set_object(&auxiliary_objects[2], 0, 0);

    func_80023C98();

    assert(transform_origin == g_unknown_800BDEE0);
    assert(transform_vertical_offset == g_unknown_800BDEE4 + 200);
    assert(transformed_direction[0] == 7);
    assert(transformed_direction[1] == 8);
    assert(transformed_direction[2] == 9);
    assert(component_translation[0] == 100);
    assert(component_translation[1] == 200);
    assert(component_translation[2] == 300);
    assert(transform_value == g_unknown_800BDEE8);
    assert(event_count == 2);
    assert(events[0].event_id == 0x32 && events[0].mode == 5);
    assert(events[0].flags == 0xFFFE);
    assert((uint32_t)events[0].heading == (uint32_t)(uintptr_t)&event_payloads[0]);
    assert(events[0].arguments[0] == 0 && events[0].arguments[1] == 0);
    assert(events[0].arguments[2] == 0 && events[0].arguments[3] == 0);
    assert((uint32_t)events[1].heading == (uint32_t)(uintptr_t)&event_payloads[1]);
    assert(call_count == sizeof(expected_calls) / sizeof(expected_calls[0]));
    assert(memcmp(call_log, expected_calls, sizeof(expected_calls)) == 0);
}

static void test_disabled_scan_and_nonpositive_count(void) {
    const int32_t expected_calls[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 31
    };

    reset_fixture();
    g_unknown_801D5F7C = 0;
    object_count = -1;

    func_80023C98();

    assert(event_count == 0);
    assert(call_count == sizeof(expected_calls) / sizeof(expected_calls[0]));
    assert(memcmp(call_log, expected_calls, sizeof(expected_calls)) == 0);
}

int main(void) {
    test_transform_and_queued_events();
    test_disabled_scan_and_nonpositive_count();
    puts("main-path transform reset tests passed");
    return 0;
}
