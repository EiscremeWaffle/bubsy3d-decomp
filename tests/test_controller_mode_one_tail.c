#include "controller_mode_one_tail.h"

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
    uint32_t mode, state, enabled, value, direction, selection;
    uint32_t tag_a, tag_b, prompt, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, state, enabled, value, direction, selection, tag_a, tag_b, prompt;
    uint32_t prompt_call_tag, color_call_tag, color[3], call_count, calls[24];
    uint32_t flags_flash, auxiliary_value, cleared_state, blocked_2b;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 10 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 43 * sizeof(uint32_t), "probe output layout");

volatile uint8_t g_controller_pulse_enabled;
volatile uint8_t g_controller_pulse_value;
volatile uint8_t g_controller_pulse_direction;
volatile uint8_t g_controller_pulse_selection;
volatile uint8_t g_controller_action_latched;
volatile uint32_t g_controller_lifecycle_state;
volatile uint8_t g_controller_timeout_ticks;
int8_t g_controller_prompt_tag;
int8_t g_controller_pulse_tag_a;
int8_t g_controller_pulse_tag_b;
int8_t g_controller_button_tag;
uint8_t g_controller_prompt_text[4];
int32_t g_unknown_800BDF10;
int32_t g_unknown_800BDF14;
int32_t g_unknown_800BE400;
volatile uint8_t g_unknown_800BE680;
static ControllerRuntimeFlags runtime_flags;
ControllerRuntimeFlags *volatile g_controller_runtime_flags = &runtime_flags;
static ControllerGameContext context;
static ProbeInput current;
static ProbeOutput output;
static uint32_t interval_callback_count;

static void record_call(uint32_t call) {
    assert(output.call_count < 24);
    output.calls[output.call_count++] = call;
}

void func_800486D0(void *text, int32_t x, int32_t y, int8_t *tag) {
    if (current.state == 3) {
        assert((x == 305 && y == 35 && tag == &g_controller_pulse_tag_a)
            || (x == 345 && y == 60 && tag == &g_controller_pulse_tag_b));
        record_call(x == 305 ? 12 : 13);
    } else {
        record_call(1);
        assert(text == g_controller_prompt_text && x == 180 && y == 100);
        assert(tag == &g_controller_prompt_tag);
    }
    *tag = (int8_t)(uint8_t)(current.prompt ^ 0x55U);
    if (current.state != 3 && (current.mutations & 1U)) {
        context.mode_6d = 9;
        g_controller_pulse_enabled ^= 1U;
        g_controller_pulse_value ^= 0x80U;
    }
}

int32_t func_800483C0(int32_t tag) {
    record_call(2);
    output.prompt_call_tag = (uint32_t)tag;
    if (current.mutations & 2U) {
        g_controller_pulse_enabled ^= 1U;
        g_controller_pulse_direction ^= 1U;
        g_controller_pulse_selection ^= 1U;
    }
    return 0;
}

int32_t func_800482D0(int32_t tag, const uint8_t color[3]) {
    record_call(3);
    output.color_call_tag = (uint32_t)tag;
    output.color[0] = color[0];
    output.color[1] = color[1];
    output.color[2] = color[2];
    if (current.mutations & 4U) {
        g_controller_pulse_value ^= 0xAAU;
        g_controller_pulse_selection ^= 1U;
    }
    return 0;
}

uint8_t func_80023A30(int32_t interval) {
    assert(interval == 30);
    return (uint8_t)current.mutations;
}

void func_80047050(int32_t value) {
    (void)value;
}

int32_t func_80062890(void) {
    return 0;
}

void func_8001BC28(void) {
}

void func_8002C370(void) {
}

void func_80023C98(void) {
    interval_callback_count++;
}

int32_t func_80048504(int32_t tag) {
    record_call(4);
    assert(tag == (int8_t)(uint8_t)current.prompt);
    return 0;
}

int32_t func_80048C48(int32_t tag) {
    record_call(5);
    assert(tag == (int8_t)(uint8_t)current.prompt);
    return 0;
}

void func_80027F40(void) {
    record_call(6);
}

int32_t func_80061A64(int32_t value) {
    record_call(7);
    assert(value == 30);
    g_unknown_800BE400 = value;
    return 0;
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    const int32_t *event_data = (const int32_t *)argument_4;
    record_call(8);
    assert(event_id == 4 && mode == 5 && flags == 0xFFFF && heading == 0xFFFF);
    assert(event_data[0] == g_unknown_800BDF10 && event_data[1] == g_unknown_800BDF14);
    assert(argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
}

uint32_t func_8006BB84(int32_t value) {
    record_call(9);
    assert(value == 0);
    return 0;
}

void func_8004F688(void) {
    record_call(10);
}

void func_80027114(int32_t first, int32_t second) {
    record_call(11);
    assert(first == 1 && second == 1);
}

void func_8001BC08(void) {
}

void func_8002C9D4(int32_t value) {
    (void)value;
}

int32_t func_800481D4(int32_t tag, int32_t first, int32_t second) {
    (void)first;
    (void)second;
    return tag;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    current = *input;
    memset(&context, 0, sizeof(context));
    memset(&output, 0, sizeof(output));
    memset(output.calls, 0xFF, sizeof(output.calls));
    memset(output.color, 0xFF, sizeof(output.color));
    output.prompt_call_tag = EMPTY_VALUE;
    output.color_call_tag = EMPTY_VALUE;
    context.mode_6d = (uint8_t)input->state;
    g_controller_pulse_enabled = (uint8_t)input->enabled;
    g_controller_pulse_value = (uint8_t)input->value;
    g_controller_pulse_direction = (uint8_t)input->direction;
    g_controller_pulse_selection = (uint8_t)input->selection;
    g_controller_pulse_tag_a = (int8_t)(uint8_t)input->tag_a;
    g_controller_pulse_tag_b = (int8_t)(uint8_t)input->tag_b;
    g_controller_prompt_tag = (int8_t)(uint8_t)input->prompt;
    g_unknown_800BDF10 = (int32_t)input->tag_a;
    g_unknown_800BDF14 = (int32_t)input->tag_b;
    g_unknown_800BE400 = 42;
    g_unknown_800BE680 = 0x5A;
    runtime_flags.flash_byte_0052 = 0;
    output.result = (uint32_t)controller_mode_one_tail(&context, (uint8_t)input->mode);
    output.state = context.mode_6d;
    output.enabled = g_controller_pulse_enabled;
    output.value = g_controller_pulse_value;
    output.direction = g_controller_pulse_direction;
    output.selection = g_controller_pulse_selection;
    output.tag_a = (uint8_t)g_controller_pulse_tag_a;
    output.tag_b = (uint8_t)g_controller_pulse_tag_b;
    output.prompt = (uint8_t)g_controller_prompt_tag;
    output.flags_flash = runtime_flags.flash_byte_0052;
    output.auxiliary_value = (uint32_t)g_unknown_800BE400;
    output.cleared_state = g_unknown_800BE680;
    output.blocked_2b = context.blocked_2b;
    return output;
}

static void self_test(void) {
    ProbeInput input = {1, 0, 1, 32, 0, 1, 255, 0, 128, 0};
    ProbeOutput result = run_probe(&input);

    assert(result.result == CONTROLLER_MODE_ONE_EXIT);
    assert(result.value == 40 && result.direction == 1 && result.color_call_tag == UINT32_MAX);
    assert(result.color[0] == 40 && result.call_count == 1 && result.calls[0] == 3);
    input.state = 1;
    result = run_probe(&input);
    assert(result.state == 2 && result.call_count == 3 && result.calls[0] == 1);
    input.state = 3;
    result = run_probe(&input);
    assert(result.result == CONTROLLER_MODE_ONE_EXIT && result.state == 4);
    assert(result.enabled == 1 && result.selection == 1 && result.direction == 0);
    assert(result.value == 120);
    assert(result.call_count == 19 && result.flags_flash == 255);

    g_controller_pulse_enabled = 1;
    g_controller_pulse_value = 16;
    g_controller_pulse_selection = 0;
    g_controller_pulse_tag_a = 0x80;
    g_controller_pulse_tag_b = 0xFF;
    output.call_count = 0;
    output.color_call_tag = EMPTY_VALUE;
    assert(controller_mode_one_button_color(1U << 14) == CONTROLLER_MODE_ONE_EXIT);
    assert(g_controller_pulse_value == 128 && g_controller_pulse_selection == 0);
    assert(output.color_call_tag == UINT32_MAX && output.color[0] == 128);

    output.call_count = 0;
    output.color_call_tag = EMPTY_VALUE;
    assert(controller_mode_one_button_color((1U << 14) | (1U << 12)) == CONTROLLER_MODE_ONE_EXIT);
    assert(g_controller_pulse_selection == 1 && output.color_call_tag == 0xFFFFFF80U);

    current.mutations = 1;
    g_controller_pulse_enabled = 0;
    g_controller_action_latched = 1;
    interval_callback_count = 0;
    assert(controller_mode_one_disabled_pulse_button(1U << 11) == CONTROLLER_MODE_ONE_EXIT);
    assert(g_controller_action_latched == 0 && interval_callback_count == 1);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("controller mode-one tail tests passed");
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
