#include "camera_slots.h"
#include "grounded_case37.h"
#include "grounded_event_records.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int32_t func_80024EBC(void *request);
void func_800250CC(void);
int32_t func_80062E50(const int32_t *current, const int32_t *target, int32_t *output);

uint16_t g_move_request_count;
void *g_move_request_entries[32768];
const char g_move_request_count_condition[] = "capacity";
const char g_game_source_path[] = "game";
GroundedEventRecord *g_grounded_event_records;
uint8_t g_grounded_event_busy;
CameraTransformRecord g_camera_transforms[3];
uint8_t g_camera_slot_status[3];
uint8_t g_camera_mode;
int16_t g_grounded_case37_counter;
GroundedCase37ObjectView *g_grounded_case37_object;
uint8_t g_runtime_state_61;
int32_t g_level_value_801E8994;
GroundedCase37ComponentView *g_level_pointer_801E89A4;

typedef struct QueueObject {
    uint8_t padding[0x1C];
    void *component;
} QueueObject;

static QueueObject queue_object;
static GroundedCase37ObjectView player, selected, replacement;
static GroundedCase37ComponentView root, owner, current_component, selected_component;
static int current_transform, selected_transform, basis;
static void *lookup_object;
static int32_t sequence;
static int calls[32], call_count, assert_count, drained, queue_mutation;
static void *drained_entries[8];
static int camera_mutation, case_mutation;
static uint32_t camera_index;
static int32_t camera_mode_argument;
static GroundedEventRecord allocation, old_head, mutated_head;
static GroundedEventRecord *allocator_result;
static int allocator_calls, free_calls, free_mutation;

static void record_call(int call) {
    assert(call_count < (int)(sizeof(calls) / sizeof(calls[0])));
    calls[call_count++] = call;
}

void *func_80028764(int32_t value) {
    assert(value == 0);
    return lookup_object;
}

int32_t func_80052ABC(void *component, int32_t *output) {
    assert(component == queue_object.component);
    *output = sequence;
    return 0;
}

int32_t func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 1 && condition == g_move_request_count_condition);
    assert(source == g_game_source_path && line == 0xB01);
    assert_count++;
    if (queue_mutation) g_move_request_count = 1;
    return 0;
}

void func_80024F2C(void *object) {
    assert(drained < 8);
    drained_entries[drained++] = object;
    if (queue_mutation == 1) g_move_request_count = 1;
    if (queue_mutation == 2 && drained == 1) g_move_request_count = 3;
}

void *func_80082EEC(uint32_t size) {
    assert(size == sizeof(GroundedEventRecord));
    assert(g_grounded_event_busy == 1);
    allocator_calls++;
    g_grounded_event_records = &mutated_head;
    return allocator_result;
}

void func_80082EFC(void *record) {
    assert(record != NULL && g_grounded_event_busy == 1);
    free_calls++;
    if (free_mutation) {
        g_grounded_event_records = &mutated_head;
        g_grounded_event_busy = 99;
    }
}

int32_t func_80072798(uint32_t index, CameraTransformRecord *record) {
    assert(index == camera_index && record == &g_camera_transforms[index]);
    record_call(1);
    if (camera_mutation) {
        g_camera_mode = 255;
        record->position[0] = 1234;
        g_camera_slot_status[index] = 42;
    }
    return -1;
}

int32_t func_80072D14(int32_t mode) {
    camera_mode_argument = mode;
    record_call(2);
    if (camera_mutation) g_camera_slot_status[camera_index] = 77;
    return -1;
}

void *func_80040FC0(int32_t mode) {
    assert(mode == 0);
    record_call(3);
    return &owner;
}

int32_t func_80054B20(void *component, int32_t mode, int32_t *translation) {
    assert(component == &current_transform && mode == 0);
    record_call(4);
    translation[0] = -3;
    translation[1] = 4;
    translation[2] = INT_MAX;
    return 0;
}

int32_t func_80054C7C(void *component, int32_t mode, const int32_t *translation) {
    assert(component == &selected_transform && mode == 0);
    assert(translation[0] == -3 && translation[1] == 4 && translation[2] == INT_MAX);
    record_call(5);
    if (case_mutation) g_grounded_case37_object = &replacement;
    return 0;
}

int32_t func_800516F8(void **output) {
    record_call(6);
    *output = &basis;
    return 0;
}

int32_t func_800550A4(void *component, const void *value) {
    assert(component == &selected_transform && value == &basis);
    record_call(7);
    return 0;
}

void func_800288D8(void *object) {
    assert(object == g_grounded_case37_object->event_0C);
    record_call(8);
}

void func_80052BDC(void *component, int32_t mode) {
    assert(component == g_grounded_case37_object->component_1C && mode == 0);
    record_call(9);
}

void func_8002BB64(int32_t value) {
    assert(value == 16);
    record_call(10);
    if (case_mutation) g_grounded_case37_counter = 77;
}

void func_8001B558(int32_t group, int32_t event, void *object,
    int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7) {
    assert(group == 23 && event == 5 && object == g_grounded_case37_object->event_0C);
    assert(a3 == 0 && a4 == 0 && a5 == 0 && a6 == 0 && a7 == 0);
    record_call(11);
}

static void test_queue(void) {
    int objects[4];
    unsigned int index;

    lookup_object = &queue_object;
    queue_object.component = objects;
    queue_mutation = 0;
    assert_count = 0;
    for (index = 0; index < 31; index++) {
        g_move_request_count = (uint16_t)index;
        assert(func_80024EBC(objects) == (int32_t)index * 4);
        assert(g_move_request_count == index + 1);
        assert(g_move_request_entries[index] == objects);
    }
    assert(assert_count == 1);
    g_move_request_count = 32767;
    assert(func_80024EBC(objects) == 32767 * 4);
    assert(g_move_request_count == 32768);
    queue_mutation = 1;
    g_move_request_count = 30;
    assert(func_80024EBC(objects) == 4);
    assert(g_move_request_count == 2 && g_move_request_entries[1] == objects);

    for (index = 0; index < 4; index++) g_move_request_entries[index] = &objects[index];
    sequence = 0x31F;
    g_move_request_count = 3;
    drained = 0;
    func_800250CC();
    assert(drained == 0 && g_move_request_count == 0);
    sequence = 0;
    for (index = 0; index < 3; index++) {
        queue_mutation = (int)index;
        g_move_request_count = index == 2 ? 1 : 3;
        drained = 0;
        func_800250CC();
        assert(drained == (index == 1 ? 1 : 3) && g_move_request_count == 0);
        for (int item = 0; item < drained; item++) assert(drained_entries[item] == &objects[item]);
    }
    g_move_request_count = 65535;
    drained = 0;
    func_800250CC();
    assert(drained == 0 && g_move_request_count == 0);
}

static void test_records(void) {
    GroundedEventRecord *output = &old_head;
    int actor, value;

    allocator_calls = 0;
    g_grounded_event_busy = 7;
    assert(func_8004FF50(NULL, &value, &output) == 1);
    assert(func_8004FF50(&actor, NULL, &output) == 1);
    assert(output == &old_head && allocator_calls == 0 && g_grounded_event_busy == 7);
    for (int failure = 0; failure < 2; failure++) {
        allocator_result = failure ? (GroundedEventRecord *)(uintptr_t)UINT32_MAX : NULL;
        assert(func_8004FF50(&actor, &value, &output) == 3);
        assert(g_grounded_event_busy == 0 && output == &old_head);
    }
    allocator_result = &allocation;
    g_grounded_event_records = &old_head;
    assert(func_8004FF50(&actor, &value, &output) == 0);
    assert(output == &allocation && g_grounded_event_records == output);
    assert(output->actor == &actor && output->value_04 == &value && output->value_08 == &value);
    assert(output->next == &mutated_head && g_grounded_event_busy == 0);

    g_grounded_event_busy = 7;
    assert(func_8004FFEC(NULL) == 1 && g_grounded_event_busy == 7);
    for (int position = 0; position < 4; position++) {
        GroundedEventRecord list[3] = {{0}};
        list[0].next = &list[1];
        list[1].next = &list[2];
        g_grounded_event_records = &list[0];
        free_calls = 0;
        free_mutation = 0;
        assert(func_8004FFEC(position == 3 ? &old_head : &list[position]) == (position == 3));
        assert(free_calls == (position != 3) && g_grounded_event_busy == 0);
        assert(g_grounded_event_records == &list[position == 0 ? 1 : 0]);
        if (position == 1) assert(list[0].next == &list[2]);
        if (position == 2) assert(list[1].next == NULL);
    }
    g_grounded_event_records = NULL;
    assert(func_8004FFEC(&old_head) == 1 && g_grounded_event_busy == 0);
    old_head.next = NULL;
    g_grounded_event_records = &old_head;
    free_mutation = 1;
    assert(func_8004FFEC(&old_head) == 0);
    assert(g_grounded_event_records == &mutated_head && g_grounded_event_busy == 0);
}

static void test_cameras(void) {
    const uint8_t rotation[3] = {0, 128, 255};
    const int32_t position[3] = {INT_MIN, 0, INT_MAX};
    const uint32_t invalid[] = {3, 4, UINT32_MAX};

    for (size_t item = 0; item < sizeof(invalid) / sizeof(invalid[0]); item++) {
        call_count = 0;
        assert(func_8004EC20(invalid[item], NULL, NULL) == 1);
        assert(func_8004ECFC(invalid[item]) == 1);
        assert(func_8004ED78(invalid[item], NULL) == 1);
        assert(call_count == 0);
    }
    for (camera_index = 0; camera_index < 3; camera_index++) {
        for (camera_mutation = 0; camera_mutation < 2; camera_mutation++) {
            for (int mode = 0; mode < 3; mode++) {
                memset(g_camera_transforms, 0x5A, sizeof(g_camera_transforms));
                memset(g_camera_slot_status, 0x5A, sizeof(g_camera_slot_status));
                g_camera_mode = (uint8_t)(mode == 2 ? 255 : mode);
                call_count = 0;
                assert(func_8004EC20(camera_index, rotation, position) == 0);
                assert(call_count == 2 && calls[0] == 1 && calls[1] == 2);
                assert(camera_mode_argument == (camera_mutation || mode != 0));
                assert(g_camera_slot_status[camera_index] == 1);
                assert(g_camera_transforms[camera_index].position[0] == (camera_mutation ? 1234 : INT_MIN));
                assert(memcmp(g_camera_transforms[camera_index].rotation, rotation, 3) == 0);
                for (uint32_t other = 0; other < 3; other++) {
                    if (other != camera_index) {
                        assert(g_camera_transforms[other].rotation[0] == 0x5A);
                        assert(g_camera_slot_status[other] == 0x5A);
                    }
                }
                call_count = 0;
                assert(func_8004ECFC(camera_index) == 0);
                assert(call_count == 1 && g_camera_slot_status[camera_index] == 0);
                assert(g_camera_transforms[camera_index].rotation[0] == 0);
                assert(g_camera_transforms[camera_index].rotation[1] == 0);
                assert(g_camera_transforms[camera_index].rotation[2] == 0);
                assert(func_8004ED78(camera_index, rotation) == 0);
                assert(memcmp(g_camera_transforms[camera_index].rotation, rotation, 3) == 0);
            }
        }
    }
}

static void reset_case(void) {
    memset(&player, 0, sizeof(player));
    memset(&selected, 0, sizeof(selected));
    memset(&replacement, 0, sizeof(replacement));
    root.owner_1C = &owner;
    owner.owner_1C = &current_component;
    current_component.transform_14 = &current_transform;
    selected_component.transform_14 = &selected_transform;
    selected_component.flags_04 = 0xFB;
    selected.component_1C = &selected_component;
    selected.event_0C = &basis;
    selected.state_2C = 1;
    selected.word_28 = 99;
    replacement = selected;
    lookup_object = &player;
    g_grounded_case37_object = &selected;
    g_level_pointer_801E89A4 = &root;
    g_runtime_state_61 = 1;
    g_grounded_case37_counter = 5;
    call_count = 0;
}

static void test_case37(void) {
    for (int gate = 0; gate < 6; gate++) {
        reset_case();
        if (gate == 0) g_runtime_state_61 = 0;
        if (gate == 1) g_runtime_state_61 = 255;
        if (gate == 2) g_grounded_case37_counter = 0;
        if (gate == 3) selected.state_2C = 0;
        if (gate == 4) player.flags_04 = 4;
        if (gate == 5) player.state_10 = 1;
        func_80037B30(NULL);
        assert(call_count == 0 && selected.word_28 == 99);
    }
    reset_case();
    func_80037B30(&replacement);
    assert(g_grounded_case37_counter == 10 && g_grounded_case37_object == &replacement);
    assert(call_count == 0);
    for (int positive = 0; positive < 2; positive++) {
        for (case_mutation = 0; case_mutation < 2; case_mutation++) {
            reset_case();
            g_level_value_801E8994 = positive;
            func_80037B30(NULL);
            assert(call_count == (positive ? 8 : 9));
            for (int item = 0; item < call_count; item++) assert(calls[item] == item + (positive ? 4 : 3));
            assert(g_grounded_case37_counter == (case_mutation ? 77 : 4));
            GroundedCase37ObjectView *result = case_mutation ? &replacement : &selected;
            assert(result->state_2C == 0 && result->state_01 == 2);
            assert(result->word_28 == 0 && result->state_2E == 0xD20);
            assert(result->component_1C->flags_04 == 0xF7);
            if (case_mutation) assert(selected.state_2C == 1 && selected.word_28 == 99);
        }
    }
}

static int32_t distance_oracle(const int32_t left[3], const int32_t right[3]) {
    int32_t values[3];
    for (int item = 0; item < 3; item++) {
        int64_t difference = (int64_t)left[item] - right[item];
        uint32_t bits = (uint32_t)difference;
        int32_t wrapped;
        memcpy(&wrapped, &bits, sizeof(wrapped));
        values[item] = wrapped == INT_MIN ? INT_MIN : (wrapped < 0 ? -wrapped : wrapped);
    }
    for (int a = 0; a < 3; a++) {
        for (int b = a + 1; b < 3; b++) {
            if (values[a] < values[b]) {
                int32_t swap = values[a];
                values[a] = values[b];
                values[b] = swap;
            }
        }
    }
    uint32_t bits = (uint32_t)values[1] + (uint32_t)values[2];
    int32_t sum;
    memcpy(&sum, &bits, sizeof(sum));
    int32_t quarter = sum / 4 - (sum < 0 && sum % 4 != 0);
    bits = (uint32_t)values[0] + (uint32_t)quarter;
    memcpy(&sum, &bits, sizeof(sum));
    return sum;
}

static void test_distance(void) {
    const int32_t values[] = {INT_MIN, INT_MIN + 1, -5, -1, 0, 1, 5, INT_MAX};
    for (uint32_t combination = 0; combination < 262144; combination++) {
        uint32_t digits = combination;
        int32_t left[3], right[3], output;
        for (int item = 0; item < 3; item++) {
            left[item] = values[digits & 7];
            digits >>= 3;
            right[item] = values[digits & 7];
            digits >>= 3;
        }
        assert(func_80062E50(left, right, &output) == 0);
        assert(output == distance_oracle(left, right));
    }
}

int main(void) {
    test_queue();
    test_records();
    test_cameras();
    test_case37();
    test_distance();
    puts("Readable matching C: queue, event records, camera, case-37 mutations and 262144 distance cases passed");
    return 0;
}
