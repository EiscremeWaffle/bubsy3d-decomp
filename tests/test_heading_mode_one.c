#include "component_rotation.h"
#include "game_transition.h"
#include "heading_common_gates.h"
#include "heading_mode_one.h"
#include "mode_camera_helpers.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

enum {
    MUTATE_CLOCK_HANDLE = 1 << 0,
    MUTATE_POSITION_TIMER = 1 << 1,
    MUTATE_QUEUE_POSITION = 1 << 2,
    MUTATE_PROPERTY_HANDLE = 1 << 3,
    MUTATE_ALLOCATION_SEED = 1 << 4,
    MUTATE_ROTATION_CAMERA = 1 << 5,
    MUTATE_ORIENTATION_CAMERA = 1 << 6,
    MUTATE_ASSERT_CAMERA = 1 << 7,
    TIMED_EVENT_PRESENT = 1 << 8,
    MUTATE_TIMED_STOP = 1 << 9,
    MUTATE_TIMED_START = 1 << 10,
    MUTATE_TIMED_CREATE = 1 << 11,
    MUTATE_TIMED_REMOVE = 1 << 12
};

typedef struct ProbeInput {
    uint32_t paused, budget, previous_phase, now, previous_tick, interval;
    uint32_t handle_flags, position_x, position_y, position_z, counter, camera_present;
    uint32_t direction, oscillation, sequence_active, random_a, random_b;
    uint32_t allocation_result, mutations, seed;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t previous_phase, previous_tick, interval, counter, direction, oscillation;
    uint32_t actor_state, handle, camera, actor_position[3], mode_position[3];
    uint32_t rotation[3], camera_position[3], camera_seed, final_values[3], queue[4];
    uint32_t camera_slot[3], channels[3], budget, timed_event;
    uint32_t call_count, calls[32];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 20 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 70 * sizeof(uint32_t), "probe output layout");

void *g_mode_one_handle;
#ifdef BUBSY3D_TEST_REAL_TIMED_RECORDS
HeadingTimedRecord g_heading_timed_records[18];
#else
volatile int32_t g_unknown_8009F7BC;
#endif
int32_t g_interaction_previous_phase;
void *g_heading_mode_camera_component;
uint32_t g_heading_mode_interval;
uint32_t g_heading_mode_previous_tick;
HeadingModeCameraSeed g_heading_mode_camera_seed;
uint8_t g_heading_mode_direction;
uint32_t g_heading_mode_counter;
int32_t g_heading_mode_oscillation;
const char g_heading_mode_allocation_condition[] = "condition";
const char g_heading_mode_source_path[] = "source";

#ifdef BUBSY3D_TEST_REAL_MODE_CAMERA
int32_t g_unknown_801D89C0[3];
uint8_t g_unknown_800BE5A4[3];
CameraTransformRecord g_camera_transforms[3];
#endif

static BubsyGroundedActor actor;
static HeadingComponentView handles[2];
static ComponentRotationMatrix cameras[2];
static int transforms[3];
static uint32_t mutations, paused, now, sequence_active, random_values[2], random_count;
static uint32_t allocation_result, input_position[3], actor_position[3], mode_position[3];
static uint32_t rotation[3], camera_position[3], camera_seed, final_values[3], queue[4];
static uint32_t calls[32], call_count;

struct ComponentBasisView;
struct ComponentTransformView;

static void record_call(uint32_t call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

static uint32_t handle_code(void *handle) {
    assert(handle == &handles[0] || handle == &handles[1]);
    return handle == &handles[0] ? 1U : 2U;
}

static uint32_t camera_code(const void *camera) {
    if (camera == 0) {
        return 0;
    }
    assert(camera == &cameras[0] || camera == &cameras[1]);
    return camera == &cameras[0] ? 1U : 2U;
}

void func_8001AE88(uint32_t *output) {
    record_call(1);
    *output = now;
    if (mutations & MUTATE_CLOCK_HANDLE) {
        g_mode_one_handle = &handles[1];
    }
}

uint8_t func_8001BCE4(void) {
    record_call(2);
    return (uint8_t)paused;
}

#ifndef BUBSY3D_TEST_REAL_TIMED_RECORDS
void func_8002CB2C(int32_t value) {
    assert(value == 15);
    record_call(3);
}

void func_8002CA88(int32_t value, int32_t duration) {
    assert(value == 15 && duration == 999);
    record_call(4);
}
#else
void func_8002C708(int16_t index) {
    assert(index == 15);
    record_call(29);
    if (mutations & MUTATE_TIMED_STOP) {
        g_heading_timed_records[15].duration_30 = -2;
        g_heading_timed_records[15].owner_04 = (void *)(uintptr_t)0x22222222U;
    }
}

void func_8002C604(int16_t index) {
    assert(index == 15);
    record_call(30);
    if (mutations & MUTATE_TIMED_START) {
        g_heading_timed_records[15].owner_04 = (void *)(uintptr_t)0x33333333U;
        g_heading_timed_records[15].duration_30 = 0x12345678;
        g_heading_timed_records[15].event_0C = (GroundedEventRecord *)(uintptr_t)0x33333333U;
    }
}

int32_t func_8004FF50(void *owner, void *value, GroundedEventRecord **event_out) {
    assert(owner == g_heading_timed_records[15].owner_04 && (uintptr_t)value == 0x3C);
    assert(event_out == &g_heading_timed_records[15].event_0C);
    record_call(31);
    if (mutations & MUTATE_TIMED_CREATE) {
        *event_out = (GroundedEventRecord *)(uintptr_t)0x44444444U;
    }
    return 3;
}

int32_t func_8004FFEC(GroundedEventRecord *event) {
    assert((uintptr_t)event == 0x11111111U);
    record_call(32);
    if (mutations & MUTATE_TIMED_REMOVE) {
        g_heading_timed_records[15].duration_30 = 0x55555555;
        g_heading_timed_records[15].event_0C = (GroundedEventRecord *)(uintptr_t)0x66666666U;
    }
    return 1;
}
#endif

uint8_t func_8001C7C4(int32_t index, int32_t state) {
    assert(index == 4 && state == 12);
    record_call(5);
    return (uint8_t)sequence_active;
}

int32_t func_8001C700(int32_t index, int32_t state) {
    assert(index == 4 && state == 12);
    record_call(6);
    return 0;
}

int32_t func_8001C6BC(int32_t index, int32_t state, int32_t value) {
    assert(index == 4 && state == 12 && value == -1);
    record_call(7);
    return 0;
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    assert(mode == 4);
    if (event_id == 26) {
        int32_t *position = argument_5;
        assert(flags == 0xFFFE && heading == 0xFFFE);
        assert(argument_4 != 0 && position != 0 && argument_6 != 0 && argument_7 == &position[2]);
        record_call(11);
        memcpy(&queue[0], argument_4, sizeof(queue[0]));
        memcpy(&queue[1], &position[0], sizeof(queue[1]));
        memcpy(&queue[2], argument_6, sizeof(queue[2]));
        memcpy(&queue[3], argument_7, sizeof(queue[3]));
        if (mutations & MUTATE_QUEUE_POSITION) {
            position[1] = 0x600;
            position[2] = -123;
        }
    } else {
        assert((event_id == 27 || event_id == 46) && flags == actor.heading_0C);
        assert((uint32_t)heading == actor.heading_0C);
        assert(argument_4 == 0 && argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
        record_call(event_id == 27 ? 8 : 18);
    }
}

int32_t func_80054B20(
    const struct ComponentTransformView *transform,
    const struct ComponentBasisView *basis,
    int32_t output[3]
) {
    assert((const void *)transform == &transforms[2] && basis == 0);
    record_call(9);
    memcpy(output, input_position, sizeof(input_position));
    if (mutations & MUTATE_POSITION_TIMER) {
        g_heading_mode_previous_tick = 7;
        g_heading_mode_interval = 10;
    }
    return 0;
}

int32_t func_80082ECC(void) {
    int32_t value;
    assert(random_count < 2);
    record_call(10);
    memcpy(&value, &random_values[random_count++], sizeof(value));
    return value;
}

int32_t func_80054C7C(
    struct ComponentTransformView *transform,
    const struct ComponentBasisView *basis,
    const int32_t input[3]
) {
    assert(basis == 0);
    if ((void *)transform == &transforms[0]) {
        record_call(12);
        memcpy(mode_position, input, sizeof(mode_position));
    } else {
        assert((void *)transform == &transforms[2]);
        record_call(13);
        memcpy(actor_position, input, sizeof(actor_position));
    }
    return 0;
}

int32_t func_80052AEC(void *handle, int32_t state) {
    record_call((state == 0 ? 140 : 150) + handle_code(handle));
    assert(state == 0 || state == 3);
    if (mutations & MUTATE_PROPERTY_HANDLE) {
        g_mode_one_handle = &handles[1];
    }
    return 0;
}

void func_80052E18(void *handle) {
    record_call(160 + handle_code(handle));
}

int32_t func_80052BDC(void *handle, int32_t mode) {
    assert(mode == 0);
    record_call(170 + handle_code(handle));
    return 0;
}

void func_80022650(BubsyGroundedActor *actor_argument, int32_t selector) {
    assert(actor_argument == &actor && selector == 20);
    record_call(19);
}

int32_t func_80054624(void **output) {
    assert(output == &g_heading_mode_camera_component);
    record_call(20);
    *output = &cameras[0];
    if (mutations & MUTATE_ALLOCATION_SEED) {
        memset(&g_heading_mode_camera_seed, 0xEE, sizeof(g_heading_mode_camera_seed));
    }
    return (int32_t)allocation_result;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0 && condition == g_heading_mode_allocation_condition);
    assert(source == g_heading_mode_source_path && line == 0xDDC);
    record_call(21);
    if (mutations & MUTATE_ASSERT_CAMERA) {
        g_heading_mode_camera_component = &cameras[1];
    }
}

int32_t func_800547C8(ComponentRotationMatrix *component, const int32_t values[3]) {
    record_call(220 + camera_code(component));
    memcpy(rotation, values, sizeof(rotation));
    if (mutations & MUTATE_ROTATION_CAMERA) {
        g_heading_mode_camera_component = &cameras[1];
    }
    return 0;
}

int32_t func_800548A0(ComponentRotationMatrix *component, const int32_t values[3]) {
    record_call(230 + camera_code(component));
    memcpy(rotation, values, sizeof(rotation));
    if (mutations & MUTATE_ROTATION_CAMERA) {
        g_heading_mode_camera_component = &cameras[1];
    }
    return 0;
}

int32_t func_80054AF0(const int16_t *component, int32_t *output) {
    record_call(240 + camera_code(component));
    output[0] = 11;
    output[1] = 22;
    output[2] = 33;
    if (mutations & MUTATE_ORIENTATION_CAMERA) {
        g_heading_mode_camera_component = &cameras[1];
    }
    return 0;
}

int32_t func_8004EC20(uint32_t index, const uint8_t seed[3], const int32_t position[3]) {
    assert(index == 2);
    record_call(25);
    camera_seed = (uint32_t)seed[0] | ((uint32_t)seed[1] << 8) | ((uint32_t)seed[2] << 16);
    memcpy(camera_position, position, sizeof(camera_position));
    return 0;
}

#ifndef BUBSY3D_TEST_REAL_MODE_CAMERA
int32_t func_8004EDEC(uint32_t index, const int32_t position[3]) {
    assert(index == 2);
    record_call(26);
    memcpy(camera_position, position, sizeof(camera_position));
    return 0;
}

void func_80028100(int32_t *first, int32_t *second, int32_t *third) {
    record_call(27);
    *first = 0x10;
    *second = 0x20;
    *third = 0x30;
}

int32_t func_8004EF10(int32_t first, int32_t second, int32_t third) {
    record_call(28);
    final_values[0] = (uint32_t)first;
    final_values[1] = (uint32_t)second;
    final_values[2] = (uint32_t)third;
    return 0;
}
#else
int32_t func_80072798(uint32_t index, CameraTransformRecord *record) {
    assert(index == 2 && record == &g_camera_transforms[2]);
    record_call(26);
    memcpy(camera_position, record->position, sizeof(camera_position));
    return 0;
}

void func_80072DF0(int32_t first, int32_t second, int32_t third) {
    record_call(28);
    final_values[0] = (uint32_t)first;
    final_values[1] = (uint32_t)second;
    final_values[2] = (uint32_t)third;
}
#endif

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeOutput output;
    int32_t budget;
    unsigned index;

    memset(&actor, 0, sizeof(actor));
    memset(handles, 0, sizeof(handles));
    memset(cameras, 0, sizeof(cameras));
    memset(actor_position, 0, sizeof(actor_position));
    memset(mode_position, 0, sizeof(mode_position));
    memset(rotation, 0, sizeof(rotation));
    memset(camera_position, 0, sizeof(camera_position));
    memset(final_values, 0, sizeof(final_values));
    memset(queue, 0, sizeof(queue));
    memset(calls, 0, sizeof(calls));
    memset(&output, 0, sizeof(output));
#ifdef BUBSY3D_TEST_REAL_TIMED_RECORDS
    memset(g_heading_timed_records, 0, sizeof(g_heading_timed_records));
    g_heading_timed_records[15].owner_04 = (void *)(uintptr_t)0x10101010U;
    if (input->mutations & TIMED_EVENT_PRESENT) {
        g_heading_timed_records[15].event_0C = (GroundedEventRecord *)(uintptr_t)0x11111111U;
    }
#endif
#ifdef BUBSY3D_TEST_REAL_MODE_CAMERA
    memset(g_camera_transforms, 0, sizeof(g_camera_transforms));
    memset(g_unknown_800BE5A4, 0, sizeof(g_unknown_800BE5A4));
    g_unknown_801D89C0[0] = 0x10;
    g_unknown_801D89C0[1] = 0x20;
    g_unknown_801D89C0[2] = 0x30;
#endif
    actor.heading_0C = 0x1234;
    handles[0].transform_14 = &transforms[0];
    handles[1].transform_14 = &transforms[1];
    handles[0].unknown_00[4] = (uint8_t)input->handle_flags;
    handles[1].unknown_00[4] = 1;
    g_mode_one_handle = &handles[0];
    memcpy(&budget, &input->budget, sizeof(budget));
    g_unknown_8009F7BC = budget;
    memcpy(&g_interaction_previous_phase, &input->previous_phase, sizeof(input->previous_phase));
    g_heading_mode_previous_tick = input->previous_tick;
    g_heading_mode_interval = input->interval;
    g_heading_mode_counter = input->counter;
    g_heading_mode_camera_component = input->camera_present ? &cameras[0] : 0;
    g_heading_mode_direction = (uint8_t)input->direction;
    memcpy(&g_heading_mode_oscillation, &input->oscillation, sizeof(input->oscillation));
    for (index = 0; index < 4; ++index) {
        g_heading_mode_camera_seed.bytes[index] = (uint8_t)(input->seed >> (index * 8));
    }
    input_position[0] = input->position_x;
    input_position[1] = input->position_y;
    input_position[2] = input->position_z;
    now = input->now;
    paused = input->paused;
    sequence_active = input->sequence_active;
    random_values[0] = input->random_a;
    random_values[1] = input->random_b;
    random_count = call_count = camera_seed = 0;
    allocation_result = input->allocation_result;
    mutations = input->mutations;

    func_800395E8(&actor, &transforms[2]);
    output.previous_phase = (uint32_t)g_interaction_previous_phase;
    output.previous_tick = g_heading_mode_previous_tick;
    output.interval = g_heading_mode_interval;
    output.counter = g_heading_mode_counter;
    output.direction = g_heading_mode_direction;
    output.oscillation = (uint32_t)g_heading_mode_oscillation;
    output.actor_state = actor.state_00;
    output.handle = handle_code(g_mode_one_handle);
    output.camera = camera_code(g_heading_mode_camera_component);
    memcpy(output.actor_position, actor_position, sizeof(actor_position));
    memcpy(output.mode_position, mode_position, sizeof(mode_position));
    memcpy(output.rotation, rotation, sizeof(rotation));
    memcpy(output.camera_position, camera_position, sizeof(camera_position));
    output.camera_seed = camera_seed;
    memcpy(output.final_values, final_values, sizeof(final_values));
    memcpy(output.queue, queue, sizeof(queue));
#ifdef BUBSY3D_TEST_REAL_MODE_CAMERA
    memcpy(output.camera_slot, g_camera_transforms[2].position, sizeof(output.camera_slot));
    for (index = 0; index < 3; ++index) {
        output.channels[index] = g_unknown_800BE5A4[index];
    }
#endif
    output.budget = (uint32_t)g_unknown_8009F7BC;
#ifdef BUBSY3D_TEST_REAL_TIMED_RECORDS
    output.timed_event = (uint32_t)(uintptr_t)g_heading_timed_records[15].event_0C;
#endif
    output.call_count = call_count;
    memcpy(output.calls, calls, sizeof(calls));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.budget = 1000;
    input.camera_present = 1;
    input.position_y = 0x581;
    input.handle_flags = 0;
    output = run_probe(&input);
    assert(output.previous_phase == 0 && output.actor_state == 2);
    assert(output.actor_position[1] == 0x580 && output.mode_position[1] == 0x580);
    assert(output.final_values[0] == 0x590 && output.final_values[2] == 0x5B0);
#ifdef BUBSY3D_TEST_REAL_MODE_CAMERA
    assert(g_unknown_800BE5A4[0] == 0x59 && g_unknown_800BE5A4[1] == 0x5A);
    assert(g_unknown_800BE5A4[2] == 0x5B);
#endif

    input.paused = 1;
    output = run_probe(&input);
    assert(output.call_count == 2 && output.calls[0] == 1 && output.calls[1] == 2);

    input.paused = 0;
    input.budget = 0;
    input.camera_present = 0;
    input.seed = 0x44332211;
    input.mutations = MUTATE_ALLOCATION_SEED;
    output = run_probe(&input);
    assert(output.previous_phase == 4 && output.camera == 1 && output.counter == 0);
    assert(output.camera_seed == 0x00332211 && output.rotation[0] == 0xFFFFFC00U);

    input.budget = 0xFFFFFFFFU;
    input.previous_phase = 4;
    input.sequence_active = 1;
    input.camera_present = 1;
    input.mutations = 0;
    output = run_probe(&input);
    assert(output.calls[2] == 5 && output.calls[3] == 7);

    input.budget = 1000;
    input.previous_phase = 0;
    input.position_y = 0x100;
    input.counter = 14;
    output = run_probe(&input);
    assert(output.counter == 0 && output.rotation[1] == 0xFFFFFFFFU);
    assert(output.camera_position[0] == 11 && output.camera_position[1] == 22);
    assert(output.camera_position[2] == 33);
#ifdef BUBSY3D_TEST_REAL_MODE_CAMERA
    assert(g_camera_transforms[2].position[0] == 11);
    assert(g_camera_transforms[2].position[1] == 22);
    assert(g_camera_transforms[2].position[2] == 33);
    assert(g_unknown_800BE5A4[0] == 0x11 && g_unknown_800BE5A4[1] == 0x12);
    assert(g_unknown_800BE5A4[2] == 0x13);
#endif
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("mode-one heading tests passed");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) {
        fputs("expected --binary or no arguments\n", stderr);
        return 2;
    }
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
        perror("set binary mode");
        return 2;
    }
#endif
    while ((bytes_read = fread(&input, 1, sizeof(input), stdin)) != 0) {
        if (bytes_read != sizeof(input)) {
            fputs("incomplete probe input\n", stderr);
            return 2;
        }
        output = run_probe(&input);
        if (fwrite(&output, sizeof(output), 1, stdout) != 1) {
            perror("write probe output");
            return 2;
        }
    }
    if (ferror(stdin)) {
        fputs("failed to read probe input\n", stderr);
        return 2;
    }
    return 0;
}
