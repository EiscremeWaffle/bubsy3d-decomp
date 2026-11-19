#include "controller_callback_helpers.h"
#include "heading_common_gates.h"
#include "heading_effects.h"
#include "heading_entry_gates.h"

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
    MUTATE_SECOND_HANDLE = 1 << 0,
    MUTATE_ACTIVE = 1 << 1,
    MUTATE_PREVIOUS = 1 << 2,
    MUTATE_SEQUENCE_PREVIOUS = 1 << 3,
    MUTATE_SEQUENCE_MODE = 1 << 4,
    MUTATE_POSITION_TIMER = 1 << 5,
    MUTATE_POSITION_TRANSFORM = 1 << 6,
    MUTATE_CLOCK_TIMER = 1 << 7,
    MUTATE_SEQUENCE_COMPONENT = 1 << 8,
    TIMED_EVENT_PRESENT = 1 << 9,
    MUTATE_TIMED_STOP = 1 << 10,
    MUTATE_TIMED_START = 1 << 11,
    MUTATE_TIMED_CREATE = 1 << 12,
    MUTATE_TIMED_REMOVE = 1 << 13
};

typedef struct ProbeInput {
    uint32_t effect, previous, latched, started, duration, mode, runtime_state;
    uint32_t gate, now, handles, position_y, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t effect, previous, latched, started, mode, second_handle, actor_component;
    uint32_t position_x, position_y, position_z, gate, timed_event, call_count, calls[24];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 12 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 37 * sizeof(uint32_t), "probe output layout");

uint32_t g_heading_effect_started;
uint32_t g_heading_effect_duration;
uint8_t g_heading_effect_previous;
uint8_t g_heading_effect_latched;
uint8_t g_heading_active_byte;
uint8_t g_runtime_mode;
uint8_t g_runtime_state_61;
#ifdef BUBSY3D_TEST_REAL_TIMED_RECORDS
HeadingTimedRecord g_heading_timed_records[18];
#else
uint32_t g_unknown_8009F58C;
#endif
void *g_actor_handles[2];

static BubsyGroundedActor actor;
static HeadingComponentView handles[3];
static int handle_transforms[4];
static int actor_components[2];
static int input_transform;
static uint32_t mutations;
static uint32_t now;
static uint32_t input_y;
static uint32_t position[3];
static uint32_t calls[24];
static uint32_t call_count;
static uint32_t reset_count;
static uint32_t sequence_count;

struct ComponentBasisView;
struct ComponentTransformView;

static void record_call(uint32_t call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

static uint32_t handle_code(const void *handle) {
    unsigned index;
    if (handle == 0) {
        return 0;
    }
    for (index = 0; index < 3; ++index) {
        if (handle == &handles[index]) {
            return index + 1;
        }
    }
    assert(0);
    return 0;
}

static uint32_t transform_code(const void *transform) {
    unsigned index;
    for (index = 0; index < 4; ++index) {
        if (transform == &handle_transforms[index]) {
            return index + 1;
        }
    }
    assert(0);
    return 0;
}

int32_t func_80052BDC(void *handle, int32_t mode) {
    record_call(0x01000000U | (handle_code(handle) << 8) | (uint32_t)mode);
    if (reset_count == 0 && (mutations & MUTATE_SECOND_HANDLE)) {
        g_actor_handles[1] = &handles[2];
    }
    if (reset_count == 1) {
        if (mutations & MUTATE_ACTIVE) {
            g_heading_active_byte = 4;
        }
        if (mutations & MUTATE_PREVIOUS) {
            g_heading_effect_previous = 3;
        }
    }
    ++reset_count;
    return 0;
}

int32_t func_8001C700(int32_t index, int32_t state) {
    record_call(0x02000000U | ((uint32_t)index << 8) | (uint32_t)state);
    if (sequence_count++ == 0) {
        if (mutations & MUTATE_SEQUENCE_PREVIOUS) {
            g_heading_effect_previous = 3;
        }
        if (mutations & MUTATE_SEQUENCE_MODE) {
            g_runtime_mode = 1;
        }
        if (mutations & MUTATE_SEQUENCE_COMPONENT) {
            actor.component_1C = &actor_components[1];
        }
    }
    return 0;
}

int32_t func_8001C6BC(int32_t index, int32_t state, int32_t value) {
    assert(value == -1);
    record_call(0x09000000U | ((uint32_t)index << 8) | (uint32_t)state);
    return 0;
}

int32_t func_800532CC(void *component, int32_t first, int32_t second, int32_t third, uint32_t flags) {
    assert(first == 1 && second == 0 && third == 0);
    assert(component == &actor_components[0] || component == &actor_components[1]);
    assert(flags == 0 || flags == 0x40000000U);
    record_call(0x03000000U | (component == &actor_components[1] ? 2U : 1U) |
                (flags != 0 ? 0x100U : 0));
    return 0;
}

#ifndef BUBSY3D_TEST_REAL_TIMED_RECORDS
void func_8002CB2C(int32_t value) {
    assert(value == 5);
    record_call(0x04000000U | (uint32_t)value);
}

void func_8002CA88(int32_t value, int32_t duration) {
    assert(value == 5 && duration == 20);
    record_call(0x05000000U | ((uint32_t)value << 8) | (uint32_t)duration);
}
#else
void func_8002C708(int16_t index) {
    assert(index == 5);
    record_call(0x0A000005U);
    if (mutations & MUTATE_TIMED_STOP) {
        g_heading_timed_records[5].duration_30 = -2;
        g_heading_timed_records[5].owner_04 = (void *)(uintptr_t)0x22222222U;
    }
}

void func_8002C604(int16_t index) {
    assert(index == 5);
    record_call(0x0B000005U);
    if (mutations & MUTATE_TIMED_START) {
        g_heading_timed_records[5].owner_04 = (void *)(uintptr_t)0x33333333U;
        g_heading_timed_records[5].duration_30 = 0x12345678;
        g_heading_timed_records[5].event_0C = (GroundedEventRecord *)(uintptr_t)0x33333333U;
    }
}

int32_t func_8004FF50(void *owner, void *value, GroundedEventRecord **event_out) {
    assert(owner == g_heading_timed_records[5].owner_04 && (uintptr_t)value == 0x3C);
    assert(event_out == &g_heading_timed_records[5].event_0C);
    record_call(0x0C000005U);
    if (mutations & MUTATE_TIMED_CREATE) {
        *event_out = (GroundedEventRecord *)(uintptr_t)0x44444444U;
    }
    return 3;
}

int32_t func_8004FFEC(GroundedEventRecord *event) {
    assert((uintptr_t)event == 0x11111111U);
    record_call(0x0D000005U);
    if (mutations & MUTATE_TIMED_REMOVE) {
        g_heading_timed_records[5].duration_30 = 0x55555555;
        g_heading_timed_records[5].event_0C = (GroundedEventRecord *)(uintptr_t)0x66666666U;
    }
    return 1;
}
#endif

int32_t func_80054B20(
    const struct ComponentTransformView *transform,
    const struct ComponentBasisView *basis,
    int32_t output[3]
) {
    unsigned index;
    assert((const void *)transform == &input_transform && basis == 0);
    record_call(0x06000000U);
    output[0] = 0x1234;
    memcpy(&output[1], &input_y, sizeof(input_y));
    output[2] = -3;
    if (mutations & MUTATE_POSITION_TIMER) {
        g_heading_effect_started = 7;
    }
    if (mutations & MUTATE_POSITION_TRANSFORM) {
        for (index = 0; index < 3; ++index) {
            handles[index].transform_14 = &handle_transforms[3];
        }
    }
    return 0;
}

int32_t func_80054C7C(
    struct ComponentTransformView *transform,
    const struct ComponentBasisView *basis,
    const int32_t input[3]
) {
    assert(basis == 0);
    record_call(0x07000000U | transform_code(transform));
    memcpy(position, input, sizeof(position));
    return 0;
}

void func_8001AE88(uint32_t *output) {
    int timer_output = output == &g_heading_effect_started;
    record_call(0x08000000U | (uint32_t)timer_output);
    *output = now;
    if (!timer_output && (mutations & MUTATE_CLOCK_TIMER)) {
        g_heading_effect_started = 7;
    }
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeOutput output;
    unsigned index;

    memset(&actor, 0, sizeof(actor));
    memset(handles, 0, sizeof(handles));
    memset(position, 0, sizeof(position));
    memset(calls, 0, sizeof(calls));
    memset(&output, 0, sizeof(output));
#ifdef BUBSY3D_TEST_REAL_TIMED_RECORDS
    memset(g_heading_timed_records, 0, sizeof(g_heading_timed_records));
    g_heading_timed_records[5].owner_04 = (void *)(uintptr_t)0x10101010U;
    if (input->mutations & TIMED_EVENT_PRESENT) {
        g_heading_timed_records[5].event_0C = (GroundedEventRecord *)(uintptr_t)0x11111111U;
    }
#endif
    for (index = 0; index < 3; ++index) {
        handles[index].transform_14 = &handle_transforms[index];
    }
    actor.component_1C = &actor_components[0];
    g_actor_handles[0] = (input->handles & 1) ? &handles[0] : 0;
    g_actor_handles[1] = (input->handles & 2) ? &handles[1] : 0;
    g_heading_active_byte = (uint8_t)input->effect;
    g_heading_effect_previous = (uint8_t)input->previous;
    g_heading_effect_latched = (uint8_t)input->latched;
    g_heading_effect_started = input->started;
    g_heading_effect_duration = input->duration;
    g_runtime_mode = (uint8_t)input->mode;
    g_runtime_state_61 = (uint8_t)input->runtime_state;
    g_unknown_8009F58C = input->gate;
    now = input->now;
    input_y = input->position_y;
    mutations = input->mutations;
    call_count = 0;
    reset_count = 0;
    sequence_count = 0;

    func_80039AB4(&actor, &input_transform);
    output.effect = g_heading_active_byte;
    output.previous = g_heading_effect_previous;
    output.latched = g_heading_effect_latched;
    output.started = g_heading_effect_started;
    output.mode = g_runtime_mode;
    output.second_handle = handle_code(g_actor_handles[1]);
    output.actor_component = actor.component_1C == &actor_components[1] ? 2 : 1;
    output.position_x = position[0];
    output.position_y = position[1];
    output.position_z = position[2];
    output.gate = g_unknown_8009F58C;
#ifdef BUBSY3D_TEST_REAL_TIMED_RECORDS
    output.timed_event = (uint32_t)(uintptr_t)g_heading_timed_records[5].event_0C;
#endif
    output.call_count = call_count;
    memcpy(output.calls, calls, sizeof(calls));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.effect = 2;
    input.previous = 2;
    input.handles = 3;
    input.now = 100;
    input.position_y = 0x7FFFFFFFU;
    output = run_probe(&input);
    assert(output.effect == 2 && output.started == 100);
    assert(output.position_y == 0x8000001FU);
    assert(output.calls[0] == 0x01000101U && output.calls[1] == 0x01000201U);
#ifdef BUBSY3D_TEST_REAL_TIMED_RECORDS
    assert(output.call_count == 10);
    assert(output.calls[8] == 0x02000505U && output.calls[9] == 0x09000505U);
#else
    assert(output.call_count == 9);
    assert(output.calls[7] == 0x02000505U && output.calls[8] == 0x09000505U);
#endif

    input.started = 1;
    output = run_probe(&input);
    assert(output.effect == 0 && output.started == 0);
    assert(output.calls[output.call_count - 1] == 0x02000505U);

    input.effect = 1;
    input.previous = 1;
    input.started = 100;
    input.duration = 20;
    input.now = 120;
    output = run_probe(&input);
    assert(output.effect == 1 && output.started == 100);
    input.now = 121;
    output = run_probe(&input);
    assert(output.effect == 0 && output.started == 0);

    input.effect = 4;
    input.previous = 4;
    input.started = 0;
    input.mode = 0;
    output = run_probe(&input);
    assert(output.position_y == 0x8000003FU);
    input.mode = 1;
    output = run_probe(&input);
    assert(output.position_y == 0x8000007FU);

    input.effect = 1;
    input.previous = 1;
    input.latched = 1;
    output = run_probe(&input);
    assert(output.effect == 1 && output.latched == 0 && output.started == 0);
    assert(output.call_count == 3 && output.calls[2] == 0x01000101U);

    input.effect = 0;
    input.previous = 3;
    input.latched = 0;
    input.started = 1;
    input.mutations = MUTATE_SEQUENCE_COMPONENT;
    output = run_probe(&input);
    assert(output.started == 0 && output.actor_component == 2);
    assert(output.calls[3] == 0x03000002U);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("heading effect tests passed");
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
