#include "controller_bitmap_gate.h"
#include "controller_input_route.h"

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

#define EMPTY_VALUE UINT32_MAX

typedef struct ProbeInput {
    uint32_t buttons, mode, blocked_12, blocked_2b, blocked_29;
    uint32_t toggle, refreshed_toggle, flags, hold, timeout;
    uint32_t action, lifecycle, first_interval, second_interval, cycle_result;
    uint32_t cursor, ready, mutations, replacement_hold, entry_seed;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t dispatch, blocked_29, toggle, refreshed_toggle, hold, timeout;
    uint32_t action, lifecycle, cursor, flags, interval_calls, entry_words[4], call_count;
    uint32_t calls[10];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 20 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 26 * sizeof(uint32_t), "probe output layout");

static ControllerGameContext context, refreshed;
static ControllerRuntimeFlags flags;
ControllerRuntimeFlags *volatile g_controller_runtime_flags = &flags;
volatile uint8_t g_controller_button_hold;
volatile uint8_t g_controller_timeout_ticks;
volatile uint8_t g_controller_action_latched;
volatile uint32_t g_controller_lifecycle_state;
volatile int32_t g_controller_cycle_cursor;
const char g_controller_cycle_condition[] = "condition";
const char g_controller_cycle_source[] = "source";
static ProbeInput current;
static ProbeOutput output;

static void record_call(uint32_t call) {
    assert(output.call_count < 10);
    output.calls[output.call_count++] = call;
}

int32_t func_80047050(int32_t value) {
    record_call(1);
    assert(value == 1 && g_controller_lifecycle_state == 2);
    if (current.mutations & 1U) {
        g_controller_button_hold = (uint8_t)current.replacement_hold;
    }
    return 0;
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t event_flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    record_call(2);
    assert(event_id == 1 && mode == 5 && event_flags == 0xFFFF && heading == 0xFFFF);
    assert(argument_4 == 0 && argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    if (current.mutations & 2U) {
        g_controller_button_hold ^= 0x55U;
    }
}

uint8_t func_80023A30(int32_t interval) {
    uint32_t result;
    record_call(3);
    assert(interval == 60 && output.interval_calls < 2);
    result = output.interval_calls == 0 ? current.first_interval : current.second_interval;
    output.interval_calls++;
    if (current.mutations & 4U) {
        context.blocked_29 ^= 1U;
    }
    return (uint8_t)result;
}

int32_t func_8001A280(volatile int32_t *index, GameCyclicEntry *entry) {
    unsigned int i;
    record_call(4);
    assert(index == &g_controller_cycle_cursor);
    if (!(current.mutations & 128U)) {
        for (i = 0; i < 4; i++) {
            entry->words[i] = current.entry_seed + i;
        }
    }
    if (current.mutations & 8U) {
        *index ^= 0x23;
    }
    return (int32_t)current.cycle_result;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    record_call(5);
    assert(failed == 0 && condition == g_controller_cycle_condition);
    assert(source == g_controller_cycle_source && line == 0xA19);
}

void func_80038430(GameCyclicEntry *entry) {
    unsigned int i;
    record_call(6);
    for (i = 0; i < 4; i++) {
        output.entry_words[i] = entry->words[i];
    }
    if (current.mutations & 16U) {
        context.blocked_29 ^= 1U;
    }
}

int32_t func_800222E0(void) {
    record_call(7);
    if (current.mutations & 256U) {
        g_controller_action_latched ^= 0x55U;
    }
    return (int32_t)current.ready;
}

void func_80023A84(void) {
    record_call(8);
    assert(g_controller_action_latched == 1);
    if (current.mutations & 32U) {
        g_controller_action_latched ^= 0x80U;
    }
}

int32_t func_8003AA1C(void **output_context) {
    record_call(9);
    *output_context = (current.mutations & 64U) ? (void *)&context : (void *)&refreshed;
    return 1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    GameCyclicEntry cycle_entry;
    unsigned int i;

    current = *input;
    memset(&context, 0, sizeof(context));
    memset(&refreshed, 0, sizeof(refreshed));
    memset(&output, 0, sizeof(output));
    memset(output.calls, 0xFF, sizeof(output.calls));
    memset(output.entry_words, 0xFF, sizeof(output.entry_words));
    context.blocked_12 = (uint8_t)input->blocked_12;
    context.blocked_2b = (uint8_t)input->blocked_2b;
    context.blocked_29 = (uint8_t)input->blocked_29;
    context.toggle_13 = (uint8_t)input->toggle;
    refreshed.toggle_13 = (uint8_t)input->refreshed_toggle;
    flags.buttons_2a2a = (uint8_t)input->flags;
    g_controller_button_hold = (uint8_t)input->hold;
    g_controller_timeout_ticks = (uint8_t)input->timeout;
    g_controller_action_latched = (uint8_t)input->action;
    g_controller_lifecycle_state = input->lifecycle;
    g_controller_cycle_cursor = (int32_t)input->cursor;
    for (i = 0; i < 4; i++) {
        cycle_entry.words[i] = input->entry_seed ^ 0xA5A50000U ^ i;
    }
    output.dispatch = (uint32_t)controller_gate_bitmap(
        &context, (uint8_t)input->mode, input->buttons, &cycle_entry
    );
    output.blocked_29 = context.blocked_29;
    output.toggle = context.toggle_13;
    output.refreshed_toggle = refreshed.toggle_13;
    output.hold = g_controller_button_hold;
    output.timeout = g_controller_timeout_ticks;
    output.action = g_controller_action_latched;
    output.lifecycle = g_controller_lifecycle_state;
    output.cursor = (uint32_t)g_controller_cycle_cursor;
    output.flags = flags.buttons_2a2a;
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.hold = 7;
    result = run_probe(&input);
    assert(result.dispatch == 1 && result.hold == 0 && result.call_count == 0);
    input.timeout = 1;
    result = run_probe(&input);
    assert(result.dispatch == 0 && result.hold == 0 && result.timeout == 0);
    input.timeout = 0;
    input.buttons = 1U << 11;
    input.hold = 61;
    input.mutations = 3;
    input.replacement_hold = 255;
    result = run_probe(&input);
    assert(result.dispatch == 0 && result.hold == ((255U ^ 0x55U) + 1U));
    assert(result.lifecycle == 2 && result.call_count == 2);
    input.buttons = (1U << 8) | (1U << 4);
    input.flags = 8;
    input.first_interval = 1;
    input.refreshed_toggle = 4;
    input.mutations = 0;
    result = run_probe(&input);
    assert(result.refreshed_toggle == 0 && result.calls[0] == 3 && result.calls[1] == 9);

    input.buttons = (1U << 11) | (1U << 15);
    input.flags = 4;
    input.hold = 0;
    input.cycle_result = 0;
    input.mutations = 128;
    input.entry_seed = 0x12345678;
    result = run_probe(&input);
    assert(result.calls[0] == 3 && result.calls[1] == 4 && result.calls[2] == 5);
    assert(result.calls[3] == 6 && result.entry_words[0] == (0x12345678U ^ 0xA5A50000U));
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("controller bitmap-gate tests passed");
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
        result = run_probe(&input);
        if (fwrite(&result, sizeof(result), 1, stdout) != 1) {
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
