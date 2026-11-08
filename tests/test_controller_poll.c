#include "controller_input.h"
#include "game_transition.h"
#include "sequence_queries.h"
#include "texture_font_slots.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

volatile uint8_t g_controller_packets[2][8];
uint8_t g_input_stream_mode;
int32_t g_input_stream_limit;
uint32_t g_input_stream_index;
int32_t g_input_stream_words[4];
uint8_t g_input_effect_active;
int8_t g_input_effect_channel;
uint8_t g_input_effect_context[1];

GameTransitionEventRecord g_transition_event_records[100];
int32_t g_transition_event_count;
GameTransitionStaticHandlers g_transition_static_handlers[256];
const GameTransitionModeHandlers *g_transition_mode_handlers;
const GameTransitionHeadingTable *g_transition_heading_handlers;
const char g_transition_overflow_condition[] = "transition event slots";
const char g_transition_source_path[] = "../f/event.c";
const GameTransitionModeHandlers g_default_transition_mode_handlers[256] = {
    [3] = {0, {0}, func_80023A0C}
};

enum {
    CALL_STOP_STREAM = 1,
    CALL_SEQUENCE,
    CALL_EFFECT_START,
    CALL_EFFECT_CONFIGURE,
    CALL_EFFECT_ENABLE,
    CALL_EFFECT_STOP,
    CALL_EFFECT_RELEASE,
    CALL_SAMPLE
};

static int calls[16];
static unsigned int call_count;
static int stop_changes_mode;
static int mutate_channel;
static int mutate_index;
static int32_t last_sample;
static int32_t configured_channel;
static int32_t enabled_channel;
static int32_t stopped_channel;
static int32_t released_channel;

static void record_call(int call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

void func_8004E8E0(void) {
    record_call(CALL_STOP_STREAM);
    if (stop_changes_mode) {
        g_input_stream_mode = 0;
    }
}

int32_t func_8001C6BC(int32_t first, int32_t second, int32_t third) {
    assert(first == 4 && second == 5 && third == 1);
    record_call(CALL_SEQUENCE);
    return 0;
}

void func_800486D0(void *context, int32_t first, int32_t second, int8_t *channel) {
    assert(context == g_input_effect_context && first == 0x23 && second == 0x1A9);
    assert(channel == &g_input_effect_channel && g_input_effect_active == 1);
    record_call(CALL_EFFECT_START);
    if (mutate_channel) {
        *channel = -9;
    }
}

void func_800481D4(int32_t channel, int32_t first, int32_t second) {
    assert(first == 0x1000 && second == 0x1000);
    configured_channel = channel;
    record_call(CALL_EFFECT_CONFIGURE);
    if (mutate_channel) {
        g_input_effect_channel = 7;
    }
}

int32_t func_800483C0(int32_t channel) {
    enabled_channel = channel;
    record_call(CALL_EFFECT_ENABLE);
    return 1;
}

int32_t func_80048504(int32_t channel) {
    assert(g_input_effect_active == 0);
    stopped_channel = channel;
    record_call(CALL_EFFECT_STOP);
    if (mutate_channel) {
        g_input_effect_channel = -3;
    }
    return 0;
}

int32_t func_80048C48(int32_t channel) {
    released_channel = channel;
    record_call(CALL_EFFECT_RELEASE);
    return 1;
}

int32_t func_8004FF28(int32_t value) {
    last_sample = value;
    record_call(CALL_SAMPLE);
    if (mutate_index) {
        g_input_stream_index = 1;
    }
    return 0;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(!"unexpected event assertion");
}

static int32_t stream_word(int16_t sample, uint16_t buttons) {
    uint32_t bits = ((uint32_t)(uint16_t)sample << 16) | buttons;
    int32_t result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

static void set_packet(unsigned int port, int present, uint16_t pressed) {
    uint16_t raw = (uint16_t)~pressed;
    g_controller_packets[port][0] = present ? 0 : 0xFF;
    g_controller_packets[port][1] = CONTROLLER_DIGITAL_TYPE;
    g_controller_packets[port][2] = (uint8_t)(raw >> 8);
    g_controller_packets[port][3] = (uint8_t)raw;
}

static void reset_state(void) {
    unsigned int port;
    unsigned int index;
    for (port = 0; port < 2; ++port) {
        for (index = 0; index < 8; ++index) {
            g_controller_packets[port][index] = 0;
        }
        set_packet(port, 0, 0);
    }
    memset(g_transition_event_records, 0, sizeof(g_transition_event_records));
    memset(g_transition_static_handlers, 0, sizeof(g_transition_static_handlers));
    memset(g_input_stream_words, 0, sizeof(g_input_stream_words));
    memset(calls, 0, sizeof(calls));
    g_transition_event_count = 0;
    g_transition_heading_handlers = 0;
    g_input_stream_mode = 0;
    g_input_stream_limit = 10;
    g_input_stream_index = 0;
    g_input_effect_active = 0;
    g_input_effect_channel = 5;
    call_count = 0;
    stop_changes_mode = 0;
    mutate_channel = 0;
    mutate_index = 0;
    last_sample = 0;
    configured_channel = 0;
    enabled_channel = 0;
    stopped_channel = 0;
    released_channel = 0;
    func_8002278C();
}

static void assert_event(uint32_t buttons, uint8_t port_1, uint8_t port_2) {
    GameTransitionEventRecord *record = &g_transition_event_records[0];
    assert(g_transition_event_count == 1);
    assert(record->event_id == 3 && record->mode == 0);
    assert(record->flags == 0xFFFE && record->heading == 0xFFFE);
    assert(record->payload.input.buttons == buttons);
    assert(record->payload.input.digital_pad_1 == port_1);
    assert(record->payload.input.digital_pad_2 == port_2);
}

static void test_digital_poll(void) {
    unsigned int present_1;
    unsigned int present_2;
    for (present_1 = 0; present_1 < 2; ++present_1) {
        for (present_2 = 0; present_2 < 2; ++present_2) {
            uint32_t expected = (present_1 ? 0x2001u : 0u) |
                (present_2 ? 0x80020000u : 0u);
            reset_state();
            set_packet(0, present_1, 0x2001);
            set_packet(1, present_2, 0x8002);
            func_8001A6E0();
            assert_event(expected, (uint8_t)present_1, (uint8_t)present_2);
            assert(g_input_stream_index == 0 && call_count == 0);
        }
    }
}

static void test_stream_boundaries(void) {
    static const int16_t samples[] = {-32768, -1, 0, 4, 5, 31, 32, 36, 37, 63, 32767};
    unsigned int i;
    unsigned int active;
    for (i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        for (active = 0; active < 2; ++active) {
            int command = samples[i] & 0x3F;
            reset_state();
            set_packet(0, 1, 0);
            g_input_stream_mode = 1;
            g_input_effect_active = (uint8_t)active;
            g_input_stream_words[0] = stream_word(samples[i], 0xA53C);
            func_8001A6E0();
            assert_event(0xA53C, 1, 0);
            assert(g_input_stream_index == 1 && last_sample == samples[i]);
            if (command < 5 && !active) {
                assert(call_count == 4);
                assert(calls[0] == CALL_EFFECT_START && calls[1] == CALL_EFFECT_CONFIGURE);
                assert(calls[2] == CALL_EFFECT_ENABLE && calls[3] == CALL_SAMPLE);
                assert(g_input_effect_active == 1);
            } else if (command >= 0x20 && command < 0x25 && active) {
                assert(call_count == 3);
                assert(calls[0] == CALL_EFFECT_STOP && calls[1] == CALL_EFFECT_RELEASE);
                assert(calls[2] == CALL_SAMPLE && g_input_effect_active == 0);
            } else {
                assert(call_count == 1 && calls[0] == CALL_SAMPLE);
                assert(g_input_effect_active == active);
            }
        }
    }
}

static void test_mutations(void) {
    reset_state();
    set_packet(0, 1, 0x2000);
    g_input_stream_mode = 1;
    g_input_stream_limit = 1;
    stop_changes_mode = 1;
    func_8001A6E0();
    assert_event(0x2000, 1, 0);
    assert(call_count == 1 && calls[0] == CALL_STOP_STREAM);
    assert(g_input_stream_mode == 0 && g_input_stream_index == 0);

    reset_state();
    set_packet(0, 1, 0x8000);
    g_input_stream_mode = 1;
    stop_changes_mode = 1;
    func_8001A6E0();
    assert_event(0x8000, 1, 0);
    assert(call_count == 2 && calls[0] == CALL_SEQUENCE && calls[1] == CALL_STOP_STREAM);
    assert(g_input_stream_index == 0);

    reset_state();
    set_packet(0, 1, 0);
    g_input_stream_mode = 1;
    g_input_stream_words[0] = stream_word(4, 0x1234);
    g_input_stream_words[1] = stream_word(37, 0xABCD);
    mutate_channel = 1;
    mutate_index = 1;
    func_8001A6E0();
    assert_event(0xABCD, 1, 0);
    assert(last_sample == 4 && g_input_stream_index == 2);
    assert(configured_channel == -9 && enabled_channel == 7);
    assert(call_count == 4 && calls[3] == CALL_SAMPLE);

    reset_state();
    set_packet(0, 1, 0);
    g_input_stream_mode = 1;
    g_input_effect_active = 1;
    g_input_effect_channel = 5;
    g_input_stream_words[0] = stream_word(32, 0x1234);
    mutate_channel = 1;
    func_8001A6E0();
    assert_event(0x1234, 1, 0);
    assert(stopped_channel == 5 && released_channel == -3);
    assert(call_count == 3 && calls[2] == CALL_SAMPLE);

    reset_state();
    set_packet(0, 1, 0x2000);
    g_input_stream_mode = 1;
    g_input_stream_words[0] = stream_word(37, 0xABCD);
    func_8001A6E0();
    assert_event(0xABCD, 1, 0);
    assert(call_count == 3);
    assert(calls[0] == CALL_SEQUENCE && calls[1] == CALL_STOP_STREAM && calls[2] == CALL_SAMPLE);

    reset_state();
    set_packet(0, 1, 0x2000);
    g_input_stream_mode = 2;
    func_8001A6E0();
    assert_event(0x2000, 1, 0);
    assert(call_count == 2 && calls[0] == CALL_SEQUENCE && calls[1] == CALL_STOP_STREAM);
    assert(g_input_stream_index == 0);
}

int main(void) {
    test_digital_poll();
    test_stream_boundaries();
    test_mutations();
    puts("Controller poll: both ports, stream boundaries and callback mutations passed.");
    return 0;
}
