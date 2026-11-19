#include "heading_callback_gates.h"
#include "heading_common_gates.h"
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
    MUTATE_PRE_MODE = 1 << 0,
    MUTATE_PRE_COMPONENT = 1 << 1,
    MUTATE_QUEUE_SHARED = 1 << 2,
    MUTATE_QUEUE_ALLOW = 1 << 3,
    MUTATE_QUEUE_BLOCK = 1 << 4,
    MUTATE_QUEUE_RUNTIME = 1 << 5,
    MUTATE_QUEUE_STOP = 1 << 6,
    MUTATE_SECOND_STATUS = 1 << 7,
    MUTATE_SEQUENCE_STATE = 1 << 8,
    MUTATE_RESET_ACTOR = 1 << 9
};

enum {
    CALL_LOOKUP = 1,
    CALL_CONTEXT,
    CALL_PRE_UPDATE,
    CALL_MODE_UPDATE,
    CALL_DECELERATION,
    CALL_QUEUE_4_1,
    CALL_QUEUE_5_3,
    CALL_QUEUE_5_4,
    CALL_SEQUENCE_4_3,
    CALL_ACTOR_RESET
};

typedef struct ProbeInput {
    uint32_t event_id, changed, state, flags, actor_01, actor_2c;
    uint32_t shared_a, shared_b, runtime_mode, deceleration_active;
    uint32_t runtime_state, stop, status, deceleration_state, event_byte, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t proceed, actor_same, transform_same, shared_initial, context_same;
    uint32_t event_id, state, flags, actor_01, actor_2c, shared_replaced;
    uint32_t runtime_mode, deceleration_active, runtime_state, stop, status;
    uint32_t deceleration_state, deceleration_stop, stride_override, event_byte;
    uint32_t call_count, calls[12];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 16 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 33 * sizeof(uint32_t), "probe output layout");

volatile uint8_t g_grounded_case_event;
uint8_t g_runtime_mode;
uint8_t g_runtime_state_61;
uint8_t g_deceleration_active_64B8;
uint8_t g_deceleration_state_6464;
uint8_t g_deceleration_stop_649C;
uint8_t g_stride_override;
uint8_t g_heading_status_6478;
uint8_t g_heading_stop_byte;

static HeadingActorView actor;
static HeadingSharedView shared_a;
static HeadingSharedView shared_b;
static HeadingComponentView component;
static GameTransitionPayload event;
static uint32_t mutations;
static uint32_t calls[12];
static uint32_t call_count;
static int transform_token;
static int context_token;

static void record_call(uint32_t call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

void *func_80028764(int32_t index) {
    assert(index == -3);
    record_call(CALL_LOOKUP);
    return &actor.actor;
}

void *func_80026648(void *actor_argument, void *context_argument) {
    assert(actor_argument == 0 && context_argument == 0);
    record_call(CALL_CONTEXT);
    return &context_token;
}

void func_80039AB4(BubsyGroundedActor *actor_argument, void *transform) {
    assert(actor_argument == &actor.actor && transform == &transform_token);
    record_call(CALL_PRE_UPDATE);
    if (mutations & MUTATE_PRE_MODE) {
        g_runtime_mode = 1;
        g_deceleration_active_64B8 = 1;
    }
    if (mutations & MUTATE_PRE_COMPONENT) {
        actor.actor.component_1C = 0;
    }
}

void func_800395E8(BubsyGroundedActor *actor_argument, void *transform) {
    assert(actor_argument == &actor.actor && transform == &transform_token);
    record_call(CALL_MODE_UPDATE);
}

void func_80038844(BubsyGroundedActor *actor_argument) {
    assert(actor_argument == &actor.actor);
    record_call(CALL_DECELERATION);
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    assert(flags == 0xFFFE && heading == 0xFFFE);
    assert(argument_4 == &actor.actor.heading_0C);
    assert(argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    if (event_id == 4 && mode == 1) {
        record_call(CALL_QUEUE_4_1);
        if (mutations & MUTATE_QUEUE_SHARED) {
            actor.actor.shared_28 = &shared_b.shared;
        }
        if (mutations & MUTATE_QUEUE_ALLOW) {
            actor.actor.state_10 = 0x80000;
            event.event_id = 27;
        }
        if (mutations & MUTATE_QUEUE_BLOCK) {
            actor.actor.state_10 = 0x80000;
            event.event_id = 8;
        }
        if (mutations & MUTATE_QUEUE_RUNTIME) {
            g_runtime_state_61 = 5;
            actor.state_2C = 1;
        }
        if (mutations & MUTATE_QUEUE_STOP) {
            g_heading_stop_byte = 1;
        }
    } else {
        assert(event_id == 5 && (mode == 3 || mode == 4));
        record_call(mode == 3 ? CALL_QUEUE_5_3 : CALL_QUEUE_5_4);
        if (mutations & MUTATE_SECOND_STATUS) {
            g_heading_status_6478 = 7;
        }
    }
}

int32_t func_8001C700(int32_t index, int32_t state) {
    assert(index == 4 && state == 3);
    record_call(CALL_SEQUENCE_4_3);
    if (mutations & MUTATE_SEQUENCE_STATE) {
        g_deceleration_state_6464 = 1;
    }
    return 0;
}

void func_8003539C(void *actor_argument, int32_t mode) {
    assert(actor_argument == &actor.actor && mode == 1);
    record_call(CALL_ACTOR_RESET);
    if (mutations & MUTATE_RESET_ACTOR) {
        actor.actor.state_10 = 0x1000;
    }
}

static ProbeOutput run_probe(const ProbeInput *input) {
    HeadingCommonDecision decision;
    ProbeOutput output;

    memset(&actor, 0, sizeof(actor));
    memset(&shared_a, 0, sizeof(shared_a));
    memset(&shared_b, 0, sizeof(shared_b));
    memset(&component, 0, sizeof(component));
    memset(&event, 0, sizeof(event));
    memset(calls, 0, sizeof(calls));
    memset(&output, 0, sizeof(output));
    memcpy(&shared_a.state_7C, &input->shared_a, sizeof(shared_a.state_7C));
    memcpy(&shared_b.state_7C, &input->shared_b, sizeof(shared_b.state_7C));
    component.transform_14 = &transform_token;
    actor.actor.component_1C = &component;
    actor.actor.shared_28 = &shared_a.shared;
    actor.actor.state_10 = input->state;
    actor.actor.flags_04 = input->flags;
    actor.actor.state_01 = (uint8_t)input->actor_01;
    actor.state_2C = (uint8_t)input->actor_2c;
    event.event_id = (uint16_t)input->event_id;
    event.heading = -3;
    g_runtime_mode = (uint8_t)input->runtime_mode;
    g_deceleration_active_64B8 = (uint8_t)input->deceleration_active;
    g_runtime_state_61 = (uint8_t)input->runtime_state;
    g_heading_stop_byte = (uint8_t)input->stop;
    g_heading_status_6478 = (uint8_t)input->status;
    g_deceleration_state_6464 = (uint8_t)input->deceleration_state;
    g_grounded_case_event = (uint8_t)input->event_byte;
    g_deceleration_stop_649C = 9;
    g_stride_override = 7;
    mutations = input->mutations;
    call_count = 0;

    decision = heading_common_gates(&event, (uint8_t)input->changed);
    output.proceed = decision.proceed;
    output.actor_same = decision.actor == &actor.actor;
    output.transform_same = decision.transform == &transform_token;
    output.shared_initial = decision.shared == &shared_a.shared;
    output.context_same = decision.context == &context_token;
    output.event_id = event.event_id;
    output.state = actor.actor.state_10;
    output.flags = actor.actor.flags_04;
    output.actor_01 = actor.actor.state_01;
    output.actor_2c = actor.state_2C;
    output.shared_replaced = actor.actor.shared_28 == &shared_b.shared;
    output.runtime_mode = g_runtime_mode;
    output.deceleration_active = g_deceleration_active_64B8;
    output.runtime_state = g_runtime_state_61;
    output.stop = g_heading_stop_byte;
    output.status = g_heading_status_6478;
    output.deceleration_state = g_deceleration_state_6464;
    output.deceleration_stop = g_deceleration_stop_649C;
    output.stride_override = g_stride_override;
    output.event_byte = g_grounded_case_event;
    output.call_count = call_count;
    memcpy(output.calls, calls, sizeof(calls));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.event_id = 8;
    input.changed = 1;
    input.actor_01 = 7;
    output = run_probe(&input);
    assert(output.proceed == 1 && output.actor_same && output.transform_same);
    assert(output.shared_initial && output.context_same && output.event_byte == 1);
    assert(output.call_count == 5 && output.calls[3] == CALL_QUEUE_4_1);
    assert(output.calls[4] == CALL_QUEUE_5_4);

    input.shared_b = 0xFFFFFFFFU;
    input.mutations = MUTATE_QUEUE_SHARED | MUTATE_SECOND_STATUS;
    output = run_probe(&input);
    assert(output.shared_initial && output.shared_replaced);
    assert(output.calls[4] == CALL_QUEUE_5_3 && output.status == 0);

    input.mutations = MUTATE_QUEUE_BLOCK;
    output = run_probe(&input);
    assert(output.proceed == 0 && output.state == 0x80000 && output.event_byte == 0);

    input.mutations = MUTATE_QUEUE_ALLOW;
    output = run_probe(&input);
    assert(output.proceed == 1 && output.event_id == 27 && output.event_byte == 1);

    input.changed = 0;
    input.state = 0x40;
    input.flags = 0x1000;
    input.mutations = MUTATE_SEQUENCE_STATE | MUTATE_RESET_ACTOR;
    output = run_probe(&input);
    assert(output.calls[3] == CALL_SEQUENCE_4_3 && output.calls[4] == CALL_ACTOR_RESET);
    assert(output.actor_01 == 1 && output.state == 0x1000 && output.event_byte == 0);
    assert(output.deceleration_stop == 0 && output.stride_override == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("heading common gate tests passed");
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
