#include "controller_callback_helpers.h"
#include "heading_callback_gates.h"
#include "heading_common_gates.h"
#include "heading_effects.h"
#include "heading_entry_gates.h"
#include "heading_state_dispatch.h"
#include "sequence_limit.h"

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
    MUTATE_EVENT_46 = 1 << 1,
    MUTATE_EVENT_8 = 1 << 2,
    MUTATE_ACTIVE_PREVIOUS = 1 << 3,
    MUTATE_SEQUENCE_MODE = 1 << 4,
    MUTATE_SEQUENCE_COMPONENT = 1 << 5,
    MUTATE_POSITION_TIMER = 1 << 6,
    MUTATE_POSITION_TRANSFORMS = 1 << 7,
    MUTATE_QUEUE_SHARED = 1 << 8,
    MUTATE_ENVIRONMENT_STOP = 1 << 9,
    MUTATE_CLOCK_TIMER = 1 << 10,
    MUTATE_RESET_ACTOR = 1 << 11
};

typedef struct ProbeInput {
    uint32_t event_id, counter, previous_counter, environment, context_gate, stop;
    uint32_t effect, previous_effect, latched, started, duration, mode, runtime_state;
    uint32_t gate, now, handles, position_y, actor_state, actor_flags, actor_2c;
    uint32_t shared_a, shared_b, status, event_byte, deceleration_state, mutations;
    uint32_t event_flags, context_lock, context_gate_54, level_flags;
    uint32_t resource_query, resource_result, sequence_result, saved5, saved15;
    uint32_t sequence_started, stop_tick, budget15;
    uint32_t sequence_scan_enabled, sequence_limit_count, sequence_key, sequence_limit;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t proceed, changed, previous_counter, event_id;
    uint32_t effect, previous_effect, latched, started, mode, stop, second_handle;
    uint32_t actor_component, actor_state, actor_flags, actor_01, actor_2c;
    uint32_t shared_replaced, status, event_byte, deceleration_active;
    uint32_t deceleration_stop, stride_override, position_y;
    uint32_t actor_captured, transform_captured, shared_captured, context_captured;
    uint32_t call_count, calls[32];
    uint32_t actor_08, shared_04, stride_30, stride_delta_34, pending_38[3];
    uint32_t vector_48[3], vector_78[3], vector_98[3];
    uint32_t dispatch_tick, saved5, saved15, sequence_started;
    uint32_t previous_phase, context_6450, context_6452, context_645c, state_6462, flag_647a;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 42 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 86 * sizeof(uint32_t), "probe output layout");

volatile uint8_t g_grounded_case_event;
int16_t g_actor_environment;
uint8_t g_heading_context_gate;
const char g_heading_gate_condition[] = "condition";
const char g_heading_gate_source_path[] = "source";
uint32_t g_unknown_8009F58C;
uint32_t global_0x800BDE98;
uint32_t g_heading_previous_counter;
uint8_t g_heading_stop_byte;
uint8_t g_heading_active_byte;
void *g_actor_handles[2];
uint32_t g_heading_effect_started;
uint32_t g_heading_effect_duration;
uint8_t g_heading_effect_previous;
uint8_t g_heading_effect_latched;
uint8_t g_runtime_mode;
uint8_t g_runtime_state_61;
uint8_t g_deceleration_active_64B8;
uint8_t g_deceleration_state_6464;
uint8_t g_deceleration_stop_649C;
uint8_t g_stride_override;
uint8_t g_heading_status_6478;

static HeadingActorView actor;
typedef union SharedFixture {
    HeadingSharedView common;
    HeadingDispatchSharedView dispatch;
} SharedFixture;

static SharedFixture shared_a;
static SharedFixture shared_b;
static HeadingComponentView components[2];
static HeadingComponentView handles[3];
static GameTransitionPayload event;
static int transform_tokens[5];
static int context_token;
static uint32_t mutations, now, input_y, position_y;
static uint32_t calls[32], call_count, reset_count, sequence_count;

#ifdef BUBSY3D_TEST_FULL_HEADING
volatile int32_t g_unknown_8009F7BC;
int32_t g_interaction_previous_phase;
uint32_t g_heading_dispatch_stop_tick;
int16_t g_heading_saved_effect5, g_heading_saved_effect15;
uint8_t g_heading_sequence_started;
uint16_t g_heading_context_6450, g_heading_context_6452;
int16_t g_heading_context_gate_6454;
int32_t g_heading_context_645C;
uint8_t g_heading_state_6462, g_heading_flag_647A, g_heading_lock_6479;
static HeadingDispatchLevelView level_config;
HeadingDispatchLevelView *g_heading_level_config = &level_config;
static uint8_t resource_query, resource_result, sequence_result;
#ifdef BUBSY3D_TEST_REAL_SEQUENCE_WRAPPERS
uint8_t g_sequence_limit_scan_enabled;
int16_t g_sequence_limit_count;
SequenceLimitEntry g_sequence_limit_entries[2];
#endif
const char g_heading_diagnostic_48[] = "diagnostic";
const char g_heading_resource_mode_zero[] = "mode-zero";
const char g_heading_resource_mode_other[] = "mode-other";
const char g_heading_resource_condition[] = "resource-condition";
const char g_heading_resource_source[] = "resource-source";
#endif

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
    for (index = 0; index < 5; ++index) {
        if (transform == &transform_tokens[index]) {
            return index + 1;
        }
    }
    assert(0);
    return 0;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
#ifdef BUBSY3D_TEST_FULL_HEADING
    if (failed == 0 && condition == g_heading_resource_condition &&
        source == g_heading_resource_source && line == 0x6EE) {
        record_call(0x25000000U);
        return;
    }
#endif
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(0);
}

void func_8004733C(int32_t mode) {
    assert(mode == 1);
    record_call(0x10000001U);
    if (mutations & MUTATE_ENVIRONMENT_STOP) {
        g_heading_stop_byte = 1;
    }
}

void *func_80028764(int32_t heading) {
    assert(heading == -3);
    record_call(0x0B000000U);
    return &actor.actor;
}

void *func_80026648(void *actor_argument, void *context_argument) {
    assert(actor_argument == 0 && context_argument == 0);
    record_call(0x0A000000U);
    return &context_token;
}

int32_t func_80052BDC(void *handle, int32_t mode) {
    record_call(0x01000000U | (handle_code(handle) << 8) | (uint32_t)mode);
    if (reset_count == 0) {
        if (mutations & MUTATE_SECOND_HANDLE) {
            g_actor_handles[1] = &handles[2];
        }
        if (mutations & MUTATE_EVENT_46) {
            event.event_id = 46;
        }
        if (mutations & MUTATE_EVENT_8) {
            event.event_id = 8;
        }
    }
    if (reset_count == 1 && (mutations & MUTATE_ACTIVE_PREVIOUS)) {
        g_heading_active_byte = 4;
        g_heading_effect_previous = 3;
    }
    ++reset_count;
    return 0;
}

int32_t func_8001C700(int32_t index, int32_t state) {
    record_call(0x02000000U | ((uint32_t)index << 8) | (uint32_t)state);
    if (sequence_count++ == 0) {
        if (mutations & MUTATE_SEQUENCE_MODE) {
            g_runtime_mode = 1;
            g_deceleration_active_64B8 = 1;
        }
        if (mutations & MUTATE_SEQUENCE_COMPONENT) {
            actor.actor.component_1C = &components[1];
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
    assert(component == &components[0] || component == &components[1]);
    assert(flags == 0 || flags == 0x40000000U);
    record_call(0x03000000U | (component == &components[1] ? 2U : 1U) |
                (flags != 0 ? 0x100U : 0));
    return 0;
}

void func_8002CB2C(int32_t value) {
    assert(value == 5);
    record_call(0x04000005U);
}

void func_8002CA88(int32_t value, int32_t duration) {
    assert((value == 5 || value == 15) && duration >= 0 && duration <= 0x7FFF);
    record_call(0x05000000U | ((uint32_t)value << 16) | (uint32_t)duration);
}

int32_t func_80054B20(
    const struct ComponentTransformView *transform,
    const struct ComponentBasisView *basis,
    int32_t output[3]
) {
    unsigned index;
    assert((const void *)transform == &transform_tokens[0] && basis == 0);
    record_call(0x06000000U);
    output[0] = 0x1234;
    memcpy(&output[1], &input_y, sizeof(input_y));
    output[2] = -3;
    if (mutations & MUTATE_POSITION_TIMER) {
        g_heading_effect_started = 7;
    }
    if (mutations & MUTATE_POSITION_TRANSFORMS) {
        for (index = 0; index < 3; ++index) {
            handles[index].transform_14 = &transform_tokens[4];
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
    memcpy(&position_y, &input[1], sizeof(position_y));
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

void func_800395E8(BubsyGroundedActor *actor_argument, void *transform) {
    assert(actor_argument == &actor.actor && transform == &transform_tokens[0]);
    record_call(0x0C000000U);
}

void func_80038844(BubsyGroundedActor *actor_argument) {
    assert(actor_argument == &actor.actor);
    record_call(0x0D000000U);
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
#ifdef BUBSY3D_TEST_FULL_HEADING
    if (event_id == 40 || event_id == 22) {
        assert(argument_4 == 0 && argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
        assert((event_id == 40 && mode == 5 && flags == 0xFFFE && heading == 0xFFFE) ||
               (event_id == 22 && mode == 4 && flags == actor.actor.heading_0C &&
                (uint32_t)heading == actor.actor.heading_0C));
        record_call(0x11000000U | ((uint32_t)event_id << 8) | (uint32_t)mode);
        return;
    }
#endif
    assert(flags == 0xFFFE && heading == 0xFFFE && argument_4 == &actor.actor.heading_0C);
    assert(argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    assert((event_id == 4 && mode == 1) || (event_id == 5 && (mode == 3 || mode == 4)));
    record_call(0x0E000000U | ((uint32_t)event_id << 8) | (uint32_t)mode);
    if (event_id == 4 && (mutations & MUTATE_QUEUE_SHARED)) {
        actor.actor.shared_28 = &shared_b.common.shared;
        event.event_id = 27;
    }
}

void func_8003539C(void *actor_argument, int32_t mode) {
    assert(actor_argument == &actor.actor && (mode == 0 || mode == 1));
    record_call(0x0F000000U | (uint32_t)mode);
    if (mode == 1 && (mutations & MUTATE_RESET_ACTOR)) {
        actor.actor.state_10 = 0x1000;
    }
}

#ifdef BUBSY3D_TEST_FULL_HEADING
uint8_t func_8001C7C4(int32_t index, int32_t state) {
    assert(index == 5 && state == 1);
    record_call(0x12000000U);
    return sequence_result;
}

#ifdef BUBSY3D_TEST_REAL_SEQUENCE_WRAPPERS
int32_t func_8008DBF8(int32_t index, int32_t state) {
#else
int32_t func_8005E708(int32_t index, int32_t state) {
#endif
    assert(index == 5 && state == 1);
    record_call(0x13000000U);
    return 0;
}

#ifdef BUBSY3D_TEST_REAL_SEQUENCE_WRAPPERS
int32_t func_8008DF28(int32_t index, int32_t state) {
#else
int32_t func_8005E760(int32_t index, int32_t state) {
#endif
    assert(index == 5 && state == 1);
    record_call(0x14000000U);
    return 0;
}

void func_80055680(const char *message) {
    assert(message == g_heading_diagnostic_48);
    record_call(0x15000000U);
}

void func_8003697C(BubsyGroundedActor *value, void **transform, BubsyGroundedShared *shared) {
    assert(value == &actor.actor && *transform == &transform_tokens[0]);
    assert(shared == &shared_a.common.shared);
    record_call(0x16000000U);
}

int32_t func_80052AEC(void *component, int32_t value) {
    assert(component == &components[0] || component == &components[1]);
    assert(value == 0x35 || value == 0x9A);
    record_call(0x17000000U | ((component == &components[1] ? 2U : 1U) << 16) | (uint32_t)value);
    return 0;
}

void func_80052E18(void *component) {
    assert(component == &components[0] || component == &components[1]);
    record_call(0x18000000U | (component == &components[1] ? 2U : 1U));
}

void func_8001B9C8(void) { record_call(0x19000000U); }
void func_8003AC60(void) { record_call(0x1A000000U); }
uint8_t func_800283E0(void) { record_call(0x1B000000U); return resource_query; }

uint8_t func_80029548(int32_t kind, const char *resource, int32_t length, int16_t position[2]) {
    assert(kind == 2);
    assert((resource_query == 0 && resource == g_heading_resource_mode_zero && length == 0x11) ||
           (resource_query != 0 && resource == g_heading_resource_mode_other && length == 0x0F));
    assert(position[0] == 0x340 && position[1] == 0x100);
    record_call(0x1C000000U);
    return resource_result;
}

void func_80039F34(GameTransitionPayload *value, BubsyGroundedActor *argument, void *transform) {
    assert(value == &event && argument == &actor.actor && transform == &transform_tokens[0]);
    record_call(0x1D000000U);
}

void func_8003737C(BubsyGroundedActor *argument, void *transform) {
    assert(argument == &actor.actor && transform == &transform_tokens[0]);
    record_call(0x1E000000U);
}

void func_80037D14(BubsyGroundedActor *argument, void *transform, int32_t flags) {
    assert(argument == &actor.actor && transform == &transform_tokens[0] && flags == event.flags);
    record_call(0x1F000000U);
}

void func_8004D26C(int32_t mode, BubsyGroundedActor *argument, void *transform) {
    assert(argument == &actor.actor && transform == &transform_tokens[0] && (mode == 0 || mode == 1));
    record_call(0x20000000U | (uint32_t)mode);
}

void func_8004D994(int32_t mode, BubsyGroundedActor *argument, void *transform) {
    assert(argument == &actor.actor && transform == &transform_tokens[0] && (mode == 0 || mode == 1));
    record_call(0x21000000U | (uint32_t)mode);
}

void func_800347EC(BubsyGroundedActor *argument, void *transform, int32_t mode) {
    assert(argument == &actor.actor && transform == &transform_tokens[0] && mode == 1);
    record_call(0x22000000U);
}

void func_8004B35C(
    GameTransitionPayload *value, BubsyGroundedActor *argument, void *transform,
    void *context, BubsyGroundedShared *shared
) {
    assert(value == &event && argument == &actor.actor && transform == &transform_tokens[0]);
    assert(context == &context_token && shared == &shared_a.common.shared);
    record_call(0x23000000U);
}

void func_8004BFF8(
    GameTransitionPayload *value, BubsyGroundedActor *argument, void *transform,
    void *context, BubsyGroundedShared *shared
) {
    assert(value == &event && argument == &actor.actor && transform == &transform_tokens[0]);
    assert(context == &context_token && shared == &shared_a.common.shared);
    record_call(0x24000000U);
}
#endif

static ProbeOutput run_probe(const ProbeInput *input) {
#ifndef BUBSY3D_TEST_UNIFIED_HEADING
    HeadingEntryDecision entry;
    HeadingCommonDecision common = {0};
#endif
    ProbeOutput output;
    unsigned index;

    memset(&actor, 0, sizeof(actor));
    memset(&shared_a, 0, sizeof(shared_a));
    memset(&shared_b, 0, sizeof(shared_b));
    memset(components, 0, sizeof(components));
    memset(handles, 0, sizeof(handles));
    memset(&event, 0, sizeof(event));
    memset(calls, 0, sizeof(calls));
    memset(&output, 0, sizeof(output));
    memcpy(&shared_a.common.state_7C, &input->shared_a, sizeof(shared_a.common.state_7C));
    memcpy(&shared_b.common.state_7C, &input->shared_b, sizeof(shared_b.common.state_7C));
    components[0].transform_14 = &transform_tokens[0];
    components[1].transform_14 = &transform_tokens[4];
    for (index = 0; index < 3; ++index) {
        handles[index].transform_14 = &transform_tokens[index + 1];
    }
    actor.actor.component_1C = &components[0];
    actor.actor.shared_28 = &shared_a.common.shared;
    actor.actor.state_10 = input->actor_state;
    actor.actor.flags_04 = input->actor_flags;
    actor.actor.state_01 = 7;
    actor.state_2C = (uint8_t)input->actor_2c;
    event.event_id = (uint16_t)input->event_id;
    event.heading = -3;
    memcpy(&event.flags, &input->event_flags, sizeof(event.flags));
    global_0x800BDE98 = input->counter;
    g_heading_previous_counter = input->previous_counter;
    g_actor_environment = (int16_t)input->environment;
    g_heading_context_gate = (uint8_t)input->context_gate;
    g_heading_stop_byte = (uint8_t)input->stop;
    g_heading_active_byte = (uint8_t)input->effect;
    g_heading_effect_previous = (uint8_t)input->previous_effect;
    g_heading_effect_latched = (uint8_t)input->latched;
    g_heading_effect_started = input->started;
    g_heading_effect_duration = input->duration;
    g_runtime_mode = (uint8_t)input->mode;
    g_runtime_state_61 = (uint8_t)input->runtime_state;
    g_unknown_8009F58C = input->gate;
    g_actor_handles[0] = (input->handles & 1) ? &handles[0] : 0;
    g_actor_handles[1] = (input->handles & 2) ? &handles[1] : 0;
    g_heading_status_6478 = (uint8_t)input->status;
    g_grounded_case_event = (uint8_t)input->event_byte;
    g_deceleration_state_6464 = (uint8_t)input->deceleration_state;
    g_deceleration_active_64B8 = 0;
    g_deceleration_stop_649C = 9;
    g_stride_override = 7;
#ifdef BUBSY3D_TEST_FULL_HEADING
    actor.actor.unknown_08 = 0x11223344U;
    shared_a.dispatch.shared.state_04 = 0x12345678;
    shared_a.dispatch.shared.stride_30 = 7;
    shared_a.dispatch.shared.stride_delta_34 = -5;
    for (index = 0; index < 3; ++index) {
        shared_a.dispatch.shared.pending_38[index] = (int32_t)index + 1;
        shared_a.dispatch.vector_48[index] = (int32_t)index + 11;
        shared_a.dispatch.vector_98[index] = (int32_t)index + 31;
    }
    shared_a.dispatch.vector_78[0] = 21;
    shared_a.dispatch.vector_78[2] = 23;
    g_unknown_8009F7BC = (int32_t)input->budget15;
    g_heading_dispatch_stop_tick = input->stop_tick;
    g_heading_saved_effect5 = (int16_t)input->saved5;
    g_heading_saved_effect15 = (int16_t)input->saved15;
    g_heading_sequence_started = (uint8_t)input->sequence_started;
    g_heading_context_gate_6454 = (int16_t)input->context_gate_54;
    g_heading_lock_6479 = (uint8_t)input->context_lock;
    g_interaction_previous_phase = 7;
    g_heading_context_6450 = 0x1234;
    g_heading_context_6452 = 0x5678;
    g_heading_context_645C = 77;
    g_heading_state_6462 = 8;
    g_heading_flag_647A = 9;
    resource_query = (uint8_t)input->resource_query;
    resource_result = (uint8_t)input->resource_result;
    sequence_result = (uint8_t)input->sequence_result;
    memset(&level_config, 0, sizeof(level_config));
    level_config.flags_2A2A = (uint8_t)input->level_flags;
#ifdef BUBSY3D_TEST_REAL_SEQUENCE_WRAPPERS
    g_sequence_limit_scan_enabled = (uint8_t)input->sequence_scan_enabled;
    g_sequence_limit_count = (int16_t)input->sequence_limit_count;
    memset(g_sequence_limit_entries, 0, sizeof(g_sequence_limit_entries));
    g_sequence_limit_entries[0].key = (int16_t)input->sequence_key;
    g_sequence_limit_entries[0].limit = (int16_t)input->sequence_limit;
#endif
#endif
    mutations = input->mutations;
    now = input->now;
    input_y = input->position_y;
    position_y = call_count = reset_count = sequence_count = 0;

#ifdef BUBSY3D_TEST_UNIFIED_HEADING
    func_80036018(&event);
#else
    entry = heading_entry_gates(&event);
    if (entry.proceed) {
        common = heading_common_gates(&event, entry.counter_changed);
    }
#ifdef BUBSY3D_TEST_FULL_HEADING
    if (entry.proceed && common.proceed) {
        heading_state_dispatch(&event, &common);
    }
#endif
    output.proceed = entry.proceed && common.proceed;
    output.changed = entry.counter_changed;
#endif
    output.previous_counter = g_heading_previous_counter;
    output.event_id = event.event_id;
    output.effect = g_heading_active_byte;
    output.previous_effect = g_heading_effect_previous;
    output.latched = g_heading_effect_latched;
    output.started = g_heading_effect_started;
    output.mode = g_runtime_mode;
    output.stop = g_heading_stop_byte;
    output.second_handle = handle_code(g_actor_handles[1]);
    output.actor_component = actor.actor.component_1C == &components[1] ? 2 : 1;
    output.actor_state = actor.actor.state_10;
    output.actor_flags = actor.actor.flags_04;
    output.actor_01 = actor.actor.state_01;
    output.actor_2c = actor.state_2C;
    output.shared_replaced = actor.actor.shared_28 == &shared_b.common.shared;
    output.status = g_heading_status_6478;
    output.event_byte = g_grounded_case_event;
    output.deceleration_active = g_deceleration_active_64B8;
    output.deceleration_stop = g_deceleration_stop_649C;
    output.stride_override = g_stride_override;
    output.position_y = position_y;
#ifndef BUBSY3D_TEST_UNIFIED_HEADING
    output.actor_captured = common.actor == &actor.actor;
    output.transform_captured = common.transform == &transform_tokens[0];
    output.shared_captured = common.shared == &shared_a.common.shared;
    output.context_captured = common.context == &context_token;
#endif
    output.call_count = call_count;
    memcpy(output.calls, calls, sizeof(calls));
#ifdef BUBSY3D_TEST_FULL_HEADING
    output.actor_08 = actor.actor.unknown_08;
    output.shared_04 = (uint32_t)shared_a.dispatch.shared.state_04;
    output.stride_30 = (uint32_t)shared_a.dispatch.shared.stride_30;
    output.stride_delta_34 = (uint32_t)shared_a.dispatch.shared.stride_delta_34;
    for (index = 0; index < 3; ++index) {
        output.pending_38[index] = (uint32_t)shared_a.dispatch.shared.pending_38[index];
        output.vector_48[index] = (uint32_t)shared_a.dispatch.vector_48[index];
        output.vector_78[index] = (uint32_t)shared_a.dispatch.vector_78[index];
        output.vector_98[index] = (uint32_t)shared_a.dispatch.vector_98[index];
    }
    output.dispatch_tick = g_heading_dispatch_stop_tick;
    output.saved5 = (uint16_t)g_heading_saved_effect5;
    output.saved15 = (uint16_t)g_heading_saved_effect15;
    output.sequence_started = g_heading_sequence_started;
    output.previous_phase = (uint32_t)g_interaction_previous_phase;
    output.context_6450 = g_heading_context_6450;
    output.context_6452 = g_heading_context_6452;
    output.context_645c = (uint32_t)g_heading_context_645C;
    output.state_6462 = g_heading_state_6462;
    output.flag_647a = g_heading_flag_647A;
#endif
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.event_id = 8;
    input.counter = 1;
    input.effect = input.previous_effect = 2;
    input.handles = 3;
    input.now = 100;
    input.saved5 = input.saved15 = 0xFFFF;
    input.stop_tick = 5;
    input.budget15 = 20;
    input.resource_result = 1;
    input.sequence_scan_enabled = 1;
    input.sequence_limit_count = 1;
    input.sequence_key = 5;
    input.sequence_limit = 2;
    output = run_probe(&input);
    assert(output.effect == 2 && output.started == 100);
#ifndef BUBSY3D_TEST_UNIFIED_HEADING
    assert(output.proceed && output.changed);
    assert(output.actor_captured && output.transform_captured && output.shared_captured);
    assert(output.context_captured);
#endif
    assert(output.event_byte == 1);
#ifdef BUBSY3D_TEST_FULL_HEADING
    assert(output.call_count == 14 && output.calls[13] == 0x23000000U);
#else
    assert(output.call_count == 13);
#endif

    input.stop = 1;
    input.mutations = MUTATE_SECOND_HANDLE | MUTATE_EVENT_46;
    output = run_probe(&input);
    assert(output.event_id == 46 && output.effect == 0);
#ifndef BUBSY3D_TEST_UNIFIED_HEADING
    assert(output.proceed);
#endif
    assert(output.second_handle == 3 && output.calls[0] == 0x01000101U);

    input.stop = 0;
    input.environment = 19;
    input.context_gate = 1;
    input.mutations = MUTATE_ENVIRONMENT_STOP;
    output = run_probe(&input);
    assert(output.stop == 1 && output.call_count == 3);
#ifndef BUBSY3D_TEST_UNIFIED_HEADING
    assert(!output.proceed && !output.actor_captured);
#endif
    assert(output.calls[0] == 0x10000001U);

    input.environment = 0;
    input.effect = 3;
    input.previous_effect = 2;
    input.mutations = MUTATE_SEQUENCE_MODE | MUTATE_SEQUENCE_COMPONENT;
    output = run_probe(&input);
    assert(output.mode == 1 && output.deceleration_active == 1);
    assert(output.actor_component == 2);
#ifndef BUBSY3D_TEST_UNIFIED_HEADING
    assert(output.proceed && output.transform_captured);
#endif
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("integrated heading path tests passed");
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
