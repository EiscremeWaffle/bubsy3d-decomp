#include "texture_font_slots.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

uint8_t g_texture_font_loaded;
TextureFontSlot g_texture_font_slots[TEXTURE_FONT_SLOT_COUNT];
const char g_texture_font_error_condition[] = "operation";
const char g_texture_font_state_condition[] = "loaded";
const char g_texture_font_source_path[] = "source";

enum { VISIT = 1, ASSERT_STATE, ASSERT_OPERATION };
typedef struct Event {
    int kind;
    int index;
    uint32_t line;
} Event;

static Event events[2 * TEXTURE_FONT_SLOT_COUNT + 1];
static unsigned event_count;
static uint8_t color[3] = {1, 2, 3};
static int state_replacement;
static int mutate_on_first_visit;

static void record(int kind, int index, uint32_t line) {
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].index = index;
    events[event_count].line = line;
    ++event_count;
}

int32_t func_80055EA8(uint8_t *payload, const uint8_t input[3]) {
    int index;
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; index++) {
        if (payload == g_texture_font_slots[index].payload) {
            break;
        }
    }
    assert(index < TEXTURE_FONT_SLOT_COUNT);
    assert(input == color);
    record(VISIT, index, 0);
    if (mutate_on_first_visit) {
        mutate_on_first_visit = 0;
        g_texture_font_loaded = 0;
        g_texture_font_slots[49].tag = g_texture_font_slots[index].tag;
    }
    return index % 3 == 0 ? -1 : 0;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0 && source == g_texture_font_source_path);
    if (condition == g_texture_font_state_condition) {
        assert(line == 0x19C);
        record(ASSERT_STATE, -1, line);
        if (state_replacement >= 0) {
            g_texture_font_loaded = (uint8_t)state_replacement;
        }
    } else {
        assert(condition == g_texture_font_error_condition && line == 0x1A8);
        record(ASSERT_OPERATION, -1, line);
    }
}

static void reset_slots(void) {
    static const uint8_t tags[] = {0, 1, 127, 128, 255};
    int index;
    memset(g_texture_font_slots, 0xA5, sizeof(g_texture_font_slots));
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; index++) {
        g_texture_font_slots[index].tag = (int8_t)tags[index % 5];
    }
    event_count = 0;
    state_replacement = -1;
    mutate_on_first_visit = 0;
}

static void test_flags_and_tags(void) {
    static const int32_t tags[] = {0, 1, 127, 128, 255, 256, INT32_MIN + 128, -1};
    unsigned flag, choice;

    for (flag = 0; flag < 256; flag++) {
        for (choice = 0; choice < sizeof(tags) / sizeof(tags[0]); choice++) {
            unsigned at = 0;
            int index;
            reset_slots();
            g_texture_font_loaded = (uint8_t)flag;
            assert(func_800482D0(tags[choice], color) == (flag != 0));
            if (flag != 1) {
                assert(events[at].kind == ASSERT_STATE && events[at++].line == 0x19C);
            }
            if (flag != 0) {
                for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; index++) {
                    if (g_texture_font_slots[index].tag == (int8_t)tags[choice]) {
                        assert(events[at].kind == VISIT && events[at++].index == index);
                        if (index % 3 == 0) {
                            assert(events[at].kind == ASSERT_OPERATION && events[at++].line == 0x1A8);
                        }
                    }
                }
            }
            assert(at == event_count);
        }
    }
}

static void test_callback_mutations(void) {
    reset_slots();
    g_texture_font_loaded = 0;
    state_replacement = 1;
    assert(func_800482D0(1, color) == 1);
    assert(events[0].kind == ASSERT_STATE && events[1].kind == VISIT);

    reset_slots();
    g_texture_font_loaded = 2;
    state_replacement = 0;
    assert(func_800482D0(1, color) == 0);
    assert(event_count == 1 && events[0].kind == ASSERT_STATE);

    reset_slots();
    g_texture_font_loaded = 1;
    mutate_on_first_visit = 1;
    assert(func_800482D0(1, color) == 1);
    assert(g_texture_font_loaded == 0);
    assert(events[event_count - 1].kind == VISIT && events[event_count - 1].index == 49);
}

int main(void) {
    test_flags_and_tags();
    test_callback_mutations();
    puts("texture-font color dispatch tests passed");
    return 0;
}
