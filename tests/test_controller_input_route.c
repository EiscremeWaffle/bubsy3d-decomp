#include "controller_input_route.h"
#include "controller_input_prepare.h"
#include "input_events.h"

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

#define CONTEXT_ADDRESS 0x801E4000U
#define EMPTY_VALUE UINT32_MAX

typedef struct ProbeInput {
    uint32_t buttons, mode, sample, gate, reference;
    uint32_t second_port, selector, timer, mutations, replacement;
    uint32_t prepare, status_1, status_2, latched_1, latched_2, mirror_extra;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t outcome, context, mode, selected, buttons;
    uint32_t second_port, selector, timer, live_mode, secondary_argument, latched_1, latched_2, call_count;
    uint32_t calls[8];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 16 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 21 * sizeof(uint32_t), "probe output layout");

volatile uint8_t g_controller_second_port_mode;
volatile int16_t g_controller_port_selection;
volatile uint8_t g_controller_timeout_ticks;
static GameTransitionEventRecord event;
static ProbeInput current;
static ProbeOutput output;
static uint8_t live_mode;

static void record_call(uint32_t call) {
    assert(output.call_count < 8);
    output.calls[output.call_count++] = call;
}

int32_t func_8003AA1C(void **context) {
    record_call(1);
    *context = (void *)(uintptr_t)CONTEXT_ADDRESS;
    if (current.mutations & 1U) {
        event.payload.input.buttons ^= 0x11111111U;
        g_controller_second_port_mode ^= 1U;
    }
    return 1;
}

uint8_t func_8001BCE4(void) {
    uint8_t result = live_mode;
    record_call(2);
    if (current.mutations & 2U) {
        g_controller_port_selection ^= 1;
    }
    return result;
}

void func_8001AE88(uint32_t *sample) {
    record_call(3);
    *sample = current.sample;
    if (current.mutations & 4U) {
        event.payload.input.buttons = current.replacement;
    }
}

uint8_t func_8001BCF0(void) {
    record_call(4);
    return (uint8_t)current.gate;
}

uint32_t func_8001BD00(void) {
    record_call(5);
    if (current.mutations & 8U) {
        event.payload.input.buttons ^= 0x80000000U;
        g_controller_second_port_mode = (uint8_t)(current.second_port ^ 1U);
    }
    return current.reference;
}

int32_t func_8001C04C(void) {
    record_call(6);
    assert(g_controller_timeout_ticks == 40);
    if (current.mutations & 32U) {
        g_controller_timeout_ticks ^= 0x7FU;
        event.payload.input.buttons ^= 0x33333333U;
    }
    return 0;
}

void func_80049454(uint32_t buttons) {
    record_call(7);
    output.secondary_argument = buttons;
    if (current.mutations & 16U) {
        event.payload.input.buttons ^= 0x22222222U;
        g_controller_second_port_mode ^= 1U;
        live_mode ^= 0xFFU;
    }
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ControllerInputRouteResult result;
    ControllerInputPrepareState prepare_state;

    assert(input->prepare <= 1);
    current = *input;
    memset(&event, 0, sizeof(event));
    memset(&output, 0, sizeof(output));
    memset(output.calls, 0xFF, sizeof(output.calls));
    result.buttons = EMPTY_VALUE;
    event.payload.input.buttons = input->buttons;
    event.payload.input.digital_pad_1 = (uint8_t)input->status_1;
    event.payload.input.digital_pad_2 = (uint8_t)input->status_2;
    prepare_state.port_1_latched = (uint8_t)input->latched_1;
    prepare_state.port_2_latched = (uint8_t)input->latched_2;
    prepare_state.second_port_enabled = (uint8_t)input->second_port;
    prepare_state.mirror_extra_buttons = (uint8_t)input->mirror_extra;
    live_mode = (uint8_t)input->mode;
    g_controller_second_port_mode = (uint8_t)input->second_port;
    g_controller_port_selection = (int16_t)(uint16_t)input->selector;
    g_controller_timeout_ticks = (uint8_t)input->timer;
    output.secondary_argument = EMPTY_VALUE;
    if (input->prepare) {
        controller_prepare_input_event(&event, &prepare_state);
    }
    output.outcome = (uint32_t)controller_route_input_event(&event, &result);
    output.context = (uint32_t)(uintptr_t)result.game_context;
    output.mode = result.mode;
    output.selected = result.buttons;
    output.buttons = event.payload.input.buttons;
    output.second_port = g_controller_second_port_mode;
    output.selector = (uint16_t)g_controller_port_selection;
    output.timer = g_controller_timeout_ticks;
    output.live_mode = live_mode;
    output.latched_1 = prepare_state.port_1_latched;
    output.latched_2 = prepare_state.port_2_latched;
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.buttons = 0xABCD1234U;
    input.mode = 3;
    input.second_port = 1;
    result = run_probe(&input);
    assert(result.outcome == 0 && result.mode == 3 && result.selected == 0x1234);
    assert(result.secondary_argument == 0xABCD && result.calls[4] == 7);
    input.selector = 1;
    result = run_probe(&input);
    assert(result.selected == 0xABCD && result.secondary_argument == 0x1234);

    input.gate = 1;
    input.reference = 0x7FFFFFF0U;
    input.sample = 0;
    input.timer = 7;
    result = run_probe(&input);
    assert(result.outcome == 1 && result.timer == 40 && result.selected == EMPTY_VALUE);
    assert(result.calls[4] == 5 && result.calls[5] == 6);

    input.gate = 0;
    input.selector = 0;
    input.prepare = 1;
    input.latched_1 = 1;
    result = run_probe(&input);
    assert(result.latched_1 == 0 && result.selected == 0x800);
    assert(result.secondary_argument == 0x800);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("controller input-routing tests passed");
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
