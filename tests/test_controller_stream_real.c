#include "controller_input.h"
#include "game_transition.h"
#include "sequence_queries.h"
#include "texture_font_slots.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

uint8_t g_input_stream_mode;
int32_t g_input_stream_limit;
uint32_t g_input_stream_index;
int32_t g_input_stream_words[2];
uint8_t g_input_effect_active;
int8_t g_input_effect_channel;
uint8_t g_input_effect_context[1];
uint8_t g_texture_font_loaded;
TextureFontSlot g_texture_font_slots[TEXTURE_FONT_SLOT_COUNT];
uint32_t g_texture_font_count;
const char g_texture_font_error_condition[] = "font operation";
const char g_texture_font_state_condition[] = "font loaded";
const char g_texture_font_source_path[] = "texture font";

enum Event {
    START_EFFECT, CONFIGURE_EFFECT, BEGIN_1, BEGIN_3, BEGIN_49, ASSERT_BEGIN,
    END_1, END_3, END_49, ASSERT_END, PUBLISH_SAMPLE, DISPATCH
};
static enum Event events[32];
static unsigned int event_count;
static uint16_t expected_buttons;
static int32_t expected_sample;

static void record(enum Event event) {
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count++] = event;
}

void func_8004E8E0(void) {
    assert(!"unexpected stream stop");
}

int32_t func_8001C6BC(int32_t first, int32_t second, int32_t third) {
    (void)first;
    (void)second;
    (void)third;
    assert(!"unexpected hardware-button transition");
    return 0;
}

int32_t func_8004FCF8(uint32_t *buttons_out) {
    *buttons_out = 0;
    return CONTROLLER_NONE;
}

void func_800486D0(void *context, int32_t first, int32_t second, int8_t *channel) {
    assert(context == g_input_effect_context && first == 0x23 && second == 0x1A9);
    assert(g_input_effect_active == 1 && channel == &g_input_effect_channel);
    *channel = 5;
    record(START_EFFECT);
}

void func_800481D4(int32_t channel, int32_t first, int32_t second) {
    assert(channel == 5 && first == 0x1000 && second == 0x1000);
    record(CONFIGURE_EFFECT);
}

static int slot_index(void *payload) {
    int index;
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
        if (payload == g_texture_font_slots[index].payload) {
            return index;
        }
    }
    assert(!"unexpected font payload");
    return -1;
}

int32_t func_80050534(void *payload) {
    int index = slot_index(payload);
    assert(g_input_effect_active == 1);
    if (index == 1) record(BEGIN_1);
    else if (index == 3) record(BEGIN_3);
    else {
        assert(index == 49);
        record(BEGIN_49);
    }
    return index == 3 ? -1 : 0;
}

int32_t func_80050650(void *payload) {
    int index = slot_index(payload);
    assert(g_input_effect_active == 0);
    if (index == 1) record(END_1);
    else if (index == 3) record(END_3);
    else {
        assert(index == 49);
        record(END_49);
    }
    return index == 49 ? -1 : 0;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0 && condition == g_texture_font_error_condition);
    assert(source == g_texture_font_source_path);
    if (line == 0x1C8) record(ASSERT_BEGIN);
    else {
        assert(line == 0x1FA);
        record(ASSERT_END);
    }
}

int32_t func_8004FF28(int32_t sample) {
    assert(sample == expected_sample);
    record(PUBLISH_SAMPLE);
    return 0;
}

void func_8001B558(int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
                   void *buttons, void *port_1, void *port_2, void *unused) {
    assert(event_id == 3 && mode == 0 && flags == 0xFFFE && heading == 0xFFFE);
    assert(*(uint32_t *)buttons == expected_buttons);
    assert(*(uint8_t *)port_1 == 0 && *(uint8_t *)port_2 == 0 && unused == 0);
    record(DISPATCH);
}

int main(void) {
    static const enum Event expected[] = {
        START_EFFECT, CONFIGURE_EFFECT, BEGIN_1, BEGIN_3, ASSERT_BEGIN, BEGIN_49,
        PUBLISH_SAMPLE, DISPATCH, END_1, END_3, END_49, ASSERT_END, PUBLISH_SAMPLE, DISPATCH
    };
    unsigned int index;
    memset(g_texture_font_slots, 0, sizeof(g_texture_font_slots));
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
        g_texture_font_slots[index].tag = -1;
    }
    g_texture_font_slots[1].tag = 5;
    g_texture_font_slots[3].tag = 5;
    g_texture_font_slots[49].tag = 5;
    g_texture_font_count = 9;
    g_texture_font_loaded = 1;
    g_input_stream_mode = 1;
    g_input_stream_limit = 10;
    g_input_stream_words[0] = 0x00021234;
    g_input_stream_words[1] = 0x0022ABCD;

    expected_sample = 2;
    expected_buttons = 0x1234;
    func_8001A6E0();
    assert(g_input_effect_active == 1 && g_input_stream_index == 1);
    assert(g_texture_font_count == 9);

    expected_sample = 0x22;
    expected_buttons = 0xABCD;
    func_8001A6E0();
    assert(g_input_effect_active == 0 && g_input_stream_index == 2);
    assert(g_texture_font_count == 6);
    assert(g_texture_font_slots[1].tag == -1);
    assert(g_texture_font_slots[3].tag == -1);
    assert(g_texture_font_slots[49].tag == -1);
    assert(event_count == sizeof(expected) / sizeof(expected[0]));
    for (index = 0; index < event_count; ++index) {
        assert(events[index] == expected[index]);
    }
    puts("Controller stream with real texture-font helpers passed.");
    return 0;
}
