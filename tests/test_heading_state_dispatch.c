#include "controller_callback_helpers.h"
#include "heading_effects.h"
#include "heading_entry_gates.h"
#include "heading_mode_one.h"
#include "heading_state_dispatch.h"
#include "heading_timed_records.h"
#include "sequence_limit.h"
#include "sequence_queries.h"

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
    MUTATE_CLOCK = 1 << 0,
    MUTATE_QUERY = 1 << 1,
    MUTATE_START = 1 << 2,
    MUTATE_STOP = 1 << 3,
    MUTATE_DURATION = 1 << 4,
    MUTATE_COMPONENT = 1 << 5
};

typedef struct ProbeInput {
    uint32_t event_id, event_flags, actor_state, actor_flags, heading;
    uint32_t runtime_mode, runtime_state, active, gate5, budget15;
    uint32_t stop_tick, started, saved5, saved15, sequence_started, sequence_active;
    uint32_t lock, context_gate, config_flags, query_mode, resource_result, now, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t actor[5], shared[15], globals[21], arguments[4], call_count, calls[16];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 23 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 62 * sizeof(uint32_t), "probe output layout");

uint32_t g_heading_dispatch_stop_tick;
int16_t g_heading_saved_effect5, g_heading_saved_effect15;
uint8_t g_heading_sequence_started;
uint16_t g_heading_context_6450, g_heading_context_6452;
int16_t g_heading_context_gate_6454;
int32_t g_heading_context_645C;
uint8_t g_heading_state_6462, g_heading_flag_647A, g_heading_lock_6479;
HeadingDispatchLevelView *g_heading_level_config;
const char g_heading_diagnostic_48[] = "diagnostic";
const char g_heading_resource_mode_zero[] = "resource-zero";
const char g_heading_resource_mode_other[] = "resource-other";
const char g_heading_resource_condition[] = "condition";
const char g_heading_resource_source[] = "source";
uint32_t g_heading_effect_started, g_heading_effect_duration;
uint8_t g_heading_effect_previous, g_heading_effect_latched, g_heading_active_byte;
uint8_t g_runtime_mode, g_runtime_state_61, g_deceleration_active_64B8;
uint8_t g_deceleration_state_6464, g_deceleration_stop_649C, g_stride_override;
uint8_t g_heading_status_6478, g_heading_stop_byte;
int32_t g_interaction_previous_phase;
uint32_t g_unknown_8009F58C;
volatile int32_t g_unknown_8009F7BC;

static GameTransitionPayload event;
static BubsyGroundedActor actor;
static HeadingDispatchSharedView shared;
static HeadingDispatchLevelView level;
static int components[2], transform_token, context_token;
static uint32_t now, mutations, sequence_active, query_mode, resource_result;
static uint32_t calls[16], call_count, arguments[4];

static void record_call(uint32_t call) {
    assert(call_count < 16);
    calls[call_count++] = call;
}

static uint32_t component_code(const void *component) {
    assert(component == &components[0] || component == &components[1]);
    return component == &components[0] ? 1U : 2U;
}

void func_8001AE88(uint32_t *output) {
    record_call(1);
    *output = now;
    if (mutations & MUTATE_CLOCK) {
        g_heading_active_byte = 3;
        g_unknown_8009F58C = 0x3456;
        g_unknown_8009F7BC = 0x5678;
        g_runtime_mode = 1;
        g_heading_effect_started = 7;
        g_heading_dispatch_stop_tick = 9;
    }
}

uint8_t func_8001C7C4(int32_t index, int32_t state) {
    assert(index == 5 && state == 1);
    record_call(2);
    if (mutations & MUTATE_QUERY) {
        g_heading_sequence_started = 7;
    }
    return (uint8_t)sequence_active;
}

int32_t func_8005E708(int32_t index, int32_t state) {
    assert(index == 5 && state == 1);
    record_call(3);
    if (mutations & MUTATE_START) {
        g_heading_sequence_started = 8;
    }
    return 0;
}

int32_t func_8005E760(int32_t index, int32_t state) {
    assert(index == 5 && state == 1);
    record_call(4);
    if (mutations & MUTATE_STOP) {
        g_heading_saved_effect5 = 11;
        g_heading_saved_effect15 = 12;
    }
    return 0;
}

void func_8002CA88(int32_t index, int32_t duration) {
    assert(index == 5 || index == 15);
    record_call(0x05000000U | ((uint32_t)index << 16) | (uint16_t)duration);
    if (index == 5 && (mutations & MUTATE_DURATION)) {
        g_heading_saved_effect15 = 13;
    }
}

void func_80055680(const char *message) {
    assert(message == g_heading_diagnostic_48);
    record_call(6);
}

void func_8003697C(BubsyGroundedActor *actor_value, void **transform, BubsyGroundedShared *shared_value) {
    assert(actor_value == &actor && *transform == &transform_token && shared_value == &shared.shared);
    record_call(7);
}

int32_t func_80052AEC(void *component, int32_t value) {
    record_call(8);
    arguments[0] = (uint32_t)value;
    arguments[1] = component_code(component);
    if (mutations & MUTATE_COMPONENT) {
        actor.component_1C = &components[1];
        g_runtime_mode = 7;
    }
    return 0;
}

void func_80052E18(void *component) {
    record_call(9);
    arguments[2] = component_code(component);
}

void func_8001B9C8(void) {
    record_call(10);
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    assert(argument_4 == 0 && argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    record_call(11);
    arguments[0] = (uint32_t)event_id;
    arguments[1] = (uint32_t)mode;
    arguments[2] = (uint32_t)flags;
    arguments[3] = (uint32_t)heading;
}

void func_8003AC60(void) {
    record_call(12);
}

uint8_t func_800283E0(void) {
    record_call(13);
    return (uint8_t)query_mode;
}

uint8_t func_80029548(int32_t kind, const char *resource, int32_t length, int16_t position[2]) {
    assert(kind == 2);
    assert(resource == g_heading_resource_mode_zero || resource == g_heading_resource_mode_other);
    record_call(14);
    arguments[0] = resource == g_heading_resource_mode_zero ? 1 : 2;
    arguments[1] = (uint32_t)length;
    arguments[2] = (uint16_t)position[0];
    arguments[3] = (uint16_t)position[1];
    return (uint8_t)resource_result;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0 && condition == g_heading_resource_condition && source == g_heading_resource_source);
    record_call(15);
    arguments[0] = (uint32_t)failed;
    arguments[1] = line;
}

void func_80039F34(GameTransitionPayload *event_value, BubsyGroundedActor *actor_value, void *transform) {
    assert(event_value == &event && actor_value == &actor && transform == &transform_token);
    record_call(16);
}

void func_8003737C(BubsyGroundedActor *actor_value, void *transform) {
    assert(actor_value == &actor && transform == &transform_token);
    record_call(17);
}

void func_8003539C(BubsyGroundedActor *actor_value, int32_t mode) {
    assert(actor_value == &actor);
    record_call(18);
    arguments[0] = (uint32_t)mode;
}

void func_80037D14(BubsyGroundedActor *actor_value, void *transform, int32_t flags) {
    assert(actor_value == &actor && transform == &transform_token);
    record_call(19);
    arguments[0] = (uint32_t)flags;
}

void func_8004D26C(int32_t mode, BubsyGroundedActor *actor_value, void *transform) {
    assert(actor_value == &actor && transform == &transform_token);
    record_call(20);
    arguments[0] = (uint32_t)mode;
}

void func_8004D994(int32_t mode, BubsyGroundedActor *actor_value, void *transform) {
    assert(actor_value == &actor && transform == &transform_token);
    record_call(21);
    arguments[0] = (uint32_t)mode;
}

void func_800347EC(BubsyGroundedActor *actor_value, void *transform, int32_t mode) {
    assert(actor_value == &actor && transform == &transform_token);
    record_call(22);
    arguments[0] = (uint32_t)mode;
}

static void assert_route(
    GameTransitionPayload *event_value, BubsyGroundedActor *actor_value, void *transform,
    void *context, BubsyGroundedShared *shared_value
) {
    assert(event_value == &event && actor_value == &actor && transform == &transform_token);
    assert(context == &context_token && shared_value == &shared.shared);
}

void func_8004B35C(
    GameTransitionPayload *event_value, BubsyGroundedActor *actor_value, void *transform,
    void *context, BubsyGroundedShared *shared_value
) {
    assert_route(event_value, actor_value, transform, context, shared_value);
    record_call(23);
}

void func_8004BFF8(
    GameTransitionPayload *event_value, BubsyGroundedActor *actor_value, void *transform,
    void *context, BubsyGroundedShared *shared_value
) {
    assert_route(event_value, actor_value, transform, context, shared_value);
    record_call(24);
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeOutput output;
    HeadingCommonDecision decision;
    int32_t budget;
    uint16_t half;
    unsigned index;

    memset(&output, 0, sizeof(output));
    memset(&event, 0, sizeof(event));
    memset(&actor, 0, sizeof(actor));
    memset(&shared, 0x55, sizeof(shared));
    memset(&level, 0, sizeof(level));
    memset(calls, 0, sizeof(calls));
    memset(arguments, 0, sizeof(arguments));
    event.event_id = (uint16_t)input->event_id;
    memcpy(&event.flags, &input->event_flags, sizeof(event.flags));
    actor.state_10 = input->actor_state;
    actor.flags_04 = input->actor_flags;
    actor.unknown_08 = 0xBEEFBEEFU;
    actor.state_01 = 7;
    actor.heading_0C = input->heading;
    actor.component_1C = &components[0];
    actor.shared_28 = &shared.shared;
    decision.actor = &actor;
    decision.transform = &transform_token;
    decision.shared = &shared.shared;
    decision.context = &context_token;
    decision.proceed = 1;
    g_runtime_mode = (uint8_t)input->runtime_mode;
    g_runtime_state_61 = (uint8_t)input->runtime_state;
    g_heading_active_byte = (uint8_t)input->active;
    g_unknown_8009F58C = input->gate5;
    memcpy(&budget, &input->budget15, sizeof(budget));
    g_unknown_8009F7BC = budget;
    g_heading_dispatch_stop_tick = input->stop_tick;
    g_heading_effect_started = input->started;
    half = (uint16_t)input->saved5;
    memcpy(&g_heading_saved_effect5, &half, sizeof(half));
    half = (uint16_t)input->saved15;
    memcpy(&g_heading_saved_effect15, &half, sizeof(half));
    g_heading_sequence_started = (uint8_t)input->sequence_started;
    g_heading_context_6450 = 0x1111;
    g_heading_context_6452 = 0x2222;
    half = (uint16_t)input->context_gate;
    memcpy(&g_heading_context_gate_6454, &half, sizeof(half));
    g_interaction_previous_phase = 0x1234;
    g_heading_context_645C = 0x2345;
    g_stride_override = 3;
    g_heading_state_6462 = 4;
    g_heading_flag_647A = 5;
    g_deceleration_state_6464 = 6;
    g_heading_status_6478 = 7;
    g_deceleration_active_64B8 = 8;
    g_deceleration_stop_649C = 9;
    g_heading_lock_6479 = (uint8_t)input->lock;
    level.flags_2A2A = (uint8_t)input->config_flags;
    g_heading_level_config = &level;
    sequence_active = input->sequence_active;
    query_mode = input->query_mode;
    resource_result = input->resource_result;
    now = input->now;
    mutations = input->mutations;
    call_count = 0;

    heading_state_dispatch(&event, &decision);
    output.actor[0] = actor.state_10;
    output.actor[1] = actor.flags_04;
    output.actor[2] = actor.unknown_08;
    output.actor[3] = actor.state_01;
    output.actor[4] = component_code(actor.component_1C);
    output.shared[0] = (uint32_t)shared.shared.state_04;
    output.shared[1] = (uint32_t)shared.shared.stride_30;
    output.shared[2] = (uint32_t)shared.shared.stride_delta_34;
    for (index = 0; index < 3; ++index) {
        output.shared[3 + index] = (uint32_t)shared.shared.pending_38[index];
        output.shared[6 + index] = (uint32_t)shared.vector_48[index];
        output.shared[9 + index] = (uint32_t)shared.vector_78[index];
        output.shared[12 + index] = (uint32_t)shared.vector_98[index];
    }
    output.globals[0] = g_runtime_mode;
    output.globals[1] = g_runtime_state_61;
    output.globals[2] = g_heading_active_byte;
    output.globals[3] = g_unknown_8009F58C;
    output.globals[4] = (uint32_t)g_unknown_8009F7BC;
    output.globals[5] = g_heading_dispatch_stop_tick;
    output.globals[6] = g_heading_effect_started;
    output.globals[7] = (uint16_t)g_heading_saved_effect5;
    output.globals[8] = (uint16_t)g_heading_saved_effect15;
    output.globals[9] = g_heading_sequence_started;
    output.globals[10] = g_heading_context_6450;
    output.globals[11] = g_heading_context_6452;
    output.globals[12] = (uint32_t)g_interaction_previous_phase;
    output.globals[13] = (uint32_t)g_heading_context_645C;
    output.globals[14] = g_stride_override;
    output.globals[15] = g_heading_state_6462;
    output.globals[16] = g_heading_flag_647A;
    output.globals[17] = g_deceleration_state_6464;
    output.globals[18] = g_heading_status_6478;
    output.globals[19] = g_deceleration_active_64B8;
    output.globals[20] = g_deceleration_stop_649C;
    memcpy(output.arguments, arguments, sizeof(arguments));
    output.call_count = call_count;
    memcpy(output.calls, calls, sizeof(calls));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.event_id = 43;
    input.runtime_mode = 1;
    input.mutations = MUTATE_COMPONENT;
    output = run_probe(&input);
    assert(output.actor[0] == 0 && output.actor[2] == 0 && output.actor[4] == 2);
    assert(output.shared[0] == 0xFFFFFFFEU && output.shared[9] == 0);
    assert(output.arguments[0] == 0x9A && output.arguments[1] == 1 && output.arguments[2] == 2);
    assert(output.call_count == 3 && output.calls[0] == 8 && output.calls[2] == 10);

    input.event_id = 37;
    input.runtime_mode = 0;
    input.runtime_state = 5;
    input.mutations = 0;
    output = run_probe(&input);
    assert(output.call_count == 1 && output.calls[0] == 23);
    input.event_id = 38;
    input.event_flags = 0xFFFE;
    output = run_probe(&input);
    assert(output.call_count == 0);

    input.event_id = 23;
    input.actor_state = 0x01000000U;
    output = run_probe(&input);
    assert(output.call_count == 1 && output.calls[0] == 19);
    input.actor_state = 0x00100000U;
    output = run_probe(&input);
    assert(output.call_count == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("heading state-dispatch model tests passed");
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
