#include "controller_bitmap_gate.h"
#include "controller_grounded_input_path.h"
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
    uint32_t buttons, status_1, status_2, latch_1, latch_2, mirror, port_mode, selector;
    uint32_t mode, sample, timeout_gate, reference, timeout, blocked_12, blocked_2b;
    uint32_t hold, bitmap, mutations, replacement;
    uint32_t mode_state, pulse_value, pulse_direction, pulse_selection, tag_a, tag_b, prompt;
    uint32_t interval_30, flash_enabled;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t outcome, buttons, selected, mode, latch_1, latch_2, port_mode, selector;
    uint32_t timeout, hold, bitmap, row_1, pressed_2_4, released_2_5, secondary, event_count, lifecycle;
    uint32_t events[16], snapshots[16];
    uint32_t mode_state, pulse_enabled, pulse_value, pulse_direction, pulse_selection, prompt;
    uint32_t prompt_call_tag, color_call_tag, color_value, tail_call_count, tail_calls[3];
    uint32_t flash_enabled, interval_callback_count;
    uint32_t flags_flash_value, ui_call_count, ui_calls[16];
    uint32_t display_tags[3];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 28 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 85 * sizeof(uint32_t), "probe output layout");

volatile uint8_t g_controller_second_port_mode;
volatile int16_t g_controller_port_selection;
volatile uint8_t g_controller_timeout_ticks;
volatile uint8_t g_controller_button_hold;
volatile uint8_t g_controller_action_latched;
volatile uint32_t g_controller_lifecycle_state;
volatile int32_t g_controller_cycle_cursor;
volatile uint8_t g_controller_pulse_enabled;
volatile uint8_t g_controller_pulse_value;
volatile uint8_t g_controller_pulse_direction;
volatile uint8_t g_controller_pulse_selection;
volatile uint8_t g_controller_flash_enabled;
int8_t g_controller_prompt_tag;
int8_t g_controller_pulse_tag_a;
int8_t g_controller_pulse_tag_b;
int8_t g_controller_button_tag;
int32_t g_unknown_800BDF10;
int32_t g_unknown_800BDF14;
volatile uint8_t g_unknown_800BE680;
uint8_t g_controller_prompt_text[4];
const char g_controller_cycle_condition[] = "condition";
const char g_controller_cycle_source[] = "source";
static ControllerRuntimeFlags runtime_flags;
ControllerRuntimeFlags *volatile g_controller_runtime_flags = &runtime_flags;
static ControllerGameContext game_context;
static GameTransitionEventRecord event;
static ProbeInput current;
static ProbeOutput output;
static uint32_t bitmap, pressed[4][16], released[4][16];
static int32_t rows[4];
static int ui_setup_callback;
static uint32_t ui_tag_index;

static void record_tail_call(uint32_t call) {
    assert(output.tail_call_count < 3);
    output.tail_calls[output.tail_call_count++] = call;
}

static void record_ui_call(uint32_t call) {
    assert(output.ui_call_count < 16);
    output.ui_calls[output.ui_call_count++] = call;
}

static int32_t expected_ui_tag(void) {
    uint32_t tag = current.prompt;
    uint32_t tag_index = ui_tag_index / 2;

    if (!ui_setup_callback && current.mode == 1 && current.mirror != 0
        && (current.buttons & ((1U << 6) | (1U << 11))) != 0
        && (current.buttons & ((1U << 14) | (1U << 12))) == 0
        && current.pulse_selection == 0 && current.mode_state != 1
        && current.mode_state != 3
        && current.interval_30 != 0) {
        if (tag_index == 1) {
            tag = current.tag_a;
        } else if (tag_index >= 2) {
            tag = current.tag_b;
        }
    }

    ui_tag_index++;
    return (int8_t)(uint8_t)tag;
}

void func_800486D0(void *text, int32_t x, int32_t y, int8_t *tag) {
    if (ui_setup_callback) {
        record_ui_call(5);
        assert(tag != 0);
        *tag = (int8_t)(uint8_t)(current.prompt ^ 0x55U);
        return;
    }
    record_tail_call(1);
    assert(text == g_controller_prompt_text && x == 180 && y == 100);
    assert(tag == &g_controller_prompt_tag);
    *tag = (int8_t)(uint8_t)(current.prompt ^ 0x55U);
    if (current.mutations & 512U) {
        game_context.mode_6d = 9;
        g_controller_pulse_enabled ^= 1U;
        g_controller_pulse_value ^= 0x80U;
    }
}

int32_t func_80048504(int32_t tag) {
    record_ui_call(1);
    assert(tag == expected_ui_tag());
    return 0;
}

int32_t func_80048C48(int32_t tag) {
    record_ui_call(2);
    assert(tag == expected_ui_tag());
    return 0;
}

void func_8001BC08(void) {
    record_ui_call(3);
}

void func_8002C9D4(int32_t value) {
    record_ui_call(4);
    assert(value == 0 || value == 1);
}

void func_8001BC28(void) {
    record_ui_call(8);
}

void func_8002C370(void) {
    record_ui_call(10);
}

int32_t func_800481D4(int32_t tag, int32_t first, int32_t second) {
    record_ui_call(6);
    assert(first == 0x1000 && second == 0x1000);
    return tag;
}

void func_80027F40(void) {
    assert(!"state-three setup is outside this integrated fixture");
}

int32_t func_80061A64(int32_t value) {
    (void)value;
    assert(!"state-three setup is outside this integrated fixture");
    return 0;
}

void func_8006BB84(int32_t value) {
    (void)value;
    assert(!"state-three setup is outside this integrated fixture");
}

void func_8004F688(void) {
    assert(!"state-three setup is outside this integrated fixture");
}

void func_80027114(int32_t first, int32_t second) {
    record_ui_call(5);
    assert((first == 0 && second == 1) || (first == 1 && second == 1));
}

int32_t func_800483C0(int32_t tag) {
    if (ui_setup_callback) {
        record_ui_call(7);
        output.prompt_call_tag = (uint32_t)tag;
        return 0;
    }
    record_tail_call(2);
    output.prompt_call_tag = (uint32_t)tag;
    if (current.mutations & 1024U) {
        g_controller_pulse_enabled ^= 1U;
        g_controller_pulse_direction ^= 1U;
        g_controller_pulse_selection ^= 1U;
    }
    return 0;
}

int32_t func_800482D0(int32_t tag, const uint8_t color[3]) {
    record_tail_call(3);
    assert(color[0] == color[1] && color[1] == color[2]);
    output.color_call_tag = (uint32_t)tag;
    output.color_value = color[0];
    if (current.mutations & 2048U) {
        g_controller_pulse_value ^= 0xAAU;
        g_controller_pulse_selection ^= 1U;
    }
    return 0;
}

int32_t func_8003AA1C(void **context) {
    *context = &game_context;
    if (current.mutations & 1U) {
        event.payload.input.buttons ^= 0x11111111U;
        g_controller_second_port_mode ^= 1U;
        rows[2] = (rows[2] & ~0xFF) | g_controller_second_port_mode;
    }
    return 1;
}

uint8_t func_8001BCE4(void) {
    if (current.mutations & 2U) {
        g_controller_port_selection ^= 1;
    }
    return (uint8_t)current.mode;
}

void func_8001AE88(uint32_t *sample) {
    *sample = current.sample;
    if (current.mutations & 4U) {
        event.payload.input.buttons = current.replacement;
    }
}

uint8_t func_8001BCF0(void) {
    return (uint8_t)current.timeout_gate;
}

uint32_t func_8001BD00(void) {
    if (current.mutations & 8U) {
        event.payload.input.buttons ^= 0x80000000U;
    }
    return current.reference;
}

int32_t func_8001C04C(void) {
    assert(g_controller_timeout_ticks == 40);
    if (current.mutations & 32U) {
        g_controller_timeout_ticks ^= 0x7FU;
        event.payload.input.buttons ^= 0x33333333U;
    }
    return 0;
}

void func_80049454(uint32_t buttons) {
    output.secondary = buttons;
    if (current.mutations & 16U) {
        event.payload.input.buttons ^= 0x22222222U;
    }
}

int32_t func_80047050(int32_t value) {
    assert((value == 1 && (g_controller_lifecycle_state == 1
                           || g_controller_lifecycle_state == 2
                           || g_controller_lifecycle_state == 3))
        || (value == 0 && (g_controller_lifecycle_state == 2
                           || g_controller_lifecycle_state == 3)));
    if (current.mutations & 128U) {
        g_controller_button_hold = 255;
    }
    return 0;
}

void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t event_flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    assert(output.event_count < 16);
    assert(argument_6 == 0 && argument_7 == 0);
    output.events[output.event_count] = (uint32_t)event_id;
    output.snapshots[output.event_count++] = bitmap;
    if (mode == 5) {
        assert(event_flags == 0xFFFF && heading == 0xFFFF);
        if (event_id == 5) {
            assert(*(int32_t *)argument_4 == 0x200 && *(int32_t *)argument_5 == 0x1E0);
        } else {
            assert(event_id == 1 && argument_4 == 0 && argument_5 == 0);
        }
        if (current.mutations & 256U) {
            g_controller_button_hold ^= 0x55U;
        }
    } else {
        assert(mode == 4 && event_flags == 0xFFFE && heading == 0);
        assert(argument_4 == 0 && argument_5 == 0);
        if ((current.mutations & 64U) && output.event_count == 1) {
            bitmap |= 1U << 5;
            g_controller_port_selection = 1;
            rows[1] = 2;
            pressed[2][4] = 0x1F1;
            released[2][5] = 0x222;
        }
    }
}

int32_t func_80062890(void) {
    return 0;
}

uint8_t func_80023A30(int32_t interval) {
    if (interval == 30) {
        return (uint8_t)current.interval_30;
    }
    assert(interval == 60);
    return 0;
}

void func_80023C98(void) {
    output.interval_callback_count++;
}

int32_t func_8001A280(volatile int32_t *index, GameCyclicEntry *entry) {
    (void)index;
    (void)entry;
    assert(!"cyclic entries are outside this fixture's bounded path");
    return 0;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(!"assertion dispatch is outside this fixture's bounded path");
}

void func_80038430(GameCyclicEntry *entry) {
    (void)entry;
    assert(!"display update is outside this fixture's bounded path");
}

int32_t func_800222E0(void) {
    assert(!"readiness check is outside this fixture's bounded path");
    return 0;
}

void func_80023A84(void) {
    assert(!"action callback is outside this fixture's bounded path");
}

static void emit_event(
    void *context, int32_t event_id, int32_t mode, uintptr_t event_flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    assert(context == 0);
    func_8001B558(event_id, mode, event_flags, heading, argument_4, argument_5, argument_6, argument_7);
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ControllerInputPrepareState prepare;
    ControllerInputRouteResult route;
    ControllerBitmapDispatch dispatch;
    GameCyclicEntry cycle_entry = {{0}};
    unsigned int row, bit;

    assert(input->selector < 2);
    current = *input;
    memset(&event, 0, sizeof(event));
    memset(&game_context, 0, sizeof(game_context));
    memset(&output, 0, sizeof(output));
    memset(output.events, 0xFF, sizeof(output.events));
    memset(output.snapshots, 0xFF, sizeof(output.snapshots));
    memset(output.tail_calls, 0xFF, sizeof(output.tail_calls));
    memset(output.ui_calls, 0xFF, sizeof(output.ui_calls));
    event.payload.input.buttons = input->buttons;
    event.payload.input.digital_pad_1 = (uint8_t)input->status_1;
    event.payload.input.digital_pad_2 = (uint8_t)input->status_2;
    prepare.port_1_latched = (uint8_t)input->latch_1;
    prepare.port_2_latched = (uint8_t)input->latch_2;
    prepare.second_port_enabled = (uint8_t)input->port_mode;
    prepare.mirror_extra_buttons = (uint8_t)input->mirror;
    game_context.blocked_12 = (uint8_t)input->blocked_12;
    game_context.blocked_2b = (uint8_t)input->blocked_2b;
    game_context.mode_6d = (uint8_t)input->mode_state;
    runtime_flags.buttons_2a2a = 0;
    g_controller_second_port_mode = (uint8_t)input->port_mode;
    g_controller_port_selection = (int16_t)input->selector;
    g_controller_timeout_ticks = (uint8_t)input->timeout;
    g_controller_button_hold = (uint8_t)input->hold;
    g_controller_lifecycle_state = 9;
    g_controller_pulse_enabled = (uint8_t)input->mirror;
    g_controller_pulse_value = (uint8_t)input->pulse_value;
    g_controller_pulse_direction = (uint8_t)input->pulse_direction;
    g_controller_pulse_selection = (uint8_t)input->pulse_selection;
    g_controller_flash_enabled = (uint8_t)input->flash_enabled;
    g_controller_pulse_tag_a = (int8_t)(uint8_t)input->tag_a;
    g_controller_pulse_tag_b = (int8_t)(uint8_t)input->tag_b;
    g_controller_prompt_tag = (int8_t)(uint8_t)input->prompt;
    g_controller_button_tag = (int8_t)(uint8_t)input->prompt;
    runtime_flags.flash_byte_0052 = 0;
    ui_setup_callback = input->mode == 1 && (input->buttons & (1U << 8)) != 0
        && input->mirror == 0 && input->flash_enabled != 0 && input->interval_30 != 0
        && input->mode_state != 1 && input->mode_state != 3;
    bitmap = input->bitmap;
    for (row = 0; row < 4; row++) {
        rows[row] = (int32_t)row;
        for (bit = 0; bit < 16; bit++) {
            pressed[row][bit] = (row + bit) % 3 == 0 ? 0 : 0x100U + row * 32U + bit;
            released[row][bit] = (row + 2U * bit) % 4 == 0 ? 0 : 0x200U + row * 32U + bit;
        }
    }
    rows[2] = g_controller_second_port_mode;
    route.buttons = EMPTY_VALUE;
    output.secondary = EMPTY_VALUE;
    output.prompt_call_tag = EMPTY_VALUE;
    output.color_call_tag = EMPTY_VALUE;
    output.color_value = EMPTY_VALUE;
    ui_tag_index = 0;
    dispatch.bitmap = &bitmap;
    dispatch.selector = &g_controller_port_selection;
    dispatch.rows = rows;
    dispatch.pressed_events = pressed;
    dispatch.released_events = released;
    dispatch.emit = emit_event;
    dispatch.context = 0;
    output.outcome = (uint32_t)controller_grounded_input_path(
        &event, &prepare, &route, &dispatch, &cycle_entry
    );
    output.buttons = event.payload.input.buttons;
    output.selected = route.buttons;
    output.mode = route.mode;
    output.latch_1 = prepare.port_1_latched;
    output.latch_2 = prepare.port_2_latched;
    output.port_mode = g_controller_second_port_mode;
    output.selector = (uint16_t)g_controller_port_selection;
    output.timeout = g_controller_timeout_ticks;
    output.hold = g_controller_button_hold;
    output.bitmap = bitmap;
    output.row_1 = (uint32_t)rows[1];
    output.pressed_2_4 = pressed[2][4];
    output.released_2_5 = released[2][5];
    output.lifecycle = g_controller_lifecycle_state;
    output.mode_state = game_context.mode_6d;
    output.pulse_enabled = g_controller_pulse_enabled;
    output.pulse_value = g_controller_pulse_value;
    output.pulse_direction = g_controller_pulse_direction;
    output.pulse_selection = g_controller_pulse_selection;
    output.prompt = (uint8_t)g_controller_prompt_tag;
    output.flash_enabled = g_controller_flash_enabled;
    output.flags_flash_value = runtime_flags.flash_byte_0052;
    output.display_tags[0] = (uint8_t)g_controller_prompt_tag;
    output.display_tags[1] = (uint8_t)g_controller_pulse_tag_a;
    output.display_tags[2] = (uint8_t)g_controller_pulse_tag_b;
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.buttons = 1U << 13;
    input.status_1 = 1;
    input.mode = 2;
    result = run_probe(&input);
    assert(result.outcome == 2 && result.selected == (1U << 13));
    assert(result.bitmap == (1U << 13) && result.event_count == 1);
    input.timeout = 1;
    result = run_probe(&input);
    assert(result.outcome == 1 && result.timeout == 0 && result.bitmap == 0);
    input.timeout = 0;
    input.mode = 1;
    result = run_probe(&input);
    assert(result.outcome == CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT);
    assert(result.selected == (1U << 13) && result.bitmap == 0 && result.event_count == 0);
    input.buttons = 0;
    result = run_probe(&input);
    assert(result.outcome == CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT);
    input.mode_state = 2;
    result = run_probe(&input);
    assert(result.outcome == CONTROLLER_GROUNDED_INPUT_DISPATCHED);
    input.mode_state = 0;
    input.buttons = 1U << 8;
    input.interval_30 = 1;
    input.flash_enabled = 1;
    result = run_probe(&input);
    assert(result.outcome == CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT);
    assert(result.pulse_enabled == 1 && result.pulse_value == 128);
    assert(result.ui_call_count == 13 && result.flags_flash_value == 255);

    input.buttons = 1U << 6;
    input.mirror = 1;
    input.flash_enabled = 0;
    input.pulse_selection = 1;
    input.mode_state = 0;
    result = run_probe(&input);
    assert(result.outcome == CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT);
    assert(result.lifecycle == 1 && result.event_count == 1 && result.events[0] == 1);

    input.pulse_selection = 0;
    result = run_probe(&input);
    assert(result.outcome == CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT);
    assert(result.pulse_enabled == 0 && result.timeout == 20);
    assert(result.event_count == 0 && result.ui_call_count == 10);

    input.mode_state = 2;
    input.pulse_selection = 1;
    result = run_probe(&input);
    assert(result.outcome == CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT);
    assert(result.mode_state == 2 && result.lifecycle == 3);
    assert(result.event_count == 2 && result.events[0] == 5 && result.events[1] == 1);

    input.pulse_selection = 0;
    result = run_probe(&input);
    assert(result.outcome == CONTROLLER_GROUNDED_INPUT_MODE_ONE_EXIT);
    assert(result.mode_state == 0 && result.lifecycle == 2 && result.timeout == 20);
    assert(result.event_count == 2 && result.events[0] == 5 && result.events[1] == 1);
    assert(result.pulse_enabled == 0 && result.ui_call_count == 10);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("integrated grounded-controller input tests passed");
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
