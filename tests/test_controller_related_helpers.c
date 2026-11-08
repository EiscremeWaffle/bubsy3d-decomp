#include "controller_callback_helpers.h"
#include "controller_input.h"
#include "input_events.h"
#include "texture_font_slots.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

_Static_assert(sizeof(TextureFontSlot) == 44, "texture-font slot size");
_Static_assert(offsetof(TextureFontSlot, payload) == 4, "texture-font payload offset");

uint8_t g_event_runtime_byte;
int32_t g_unknown_800BE400;
uint8_t g_unknown_800BE4F8[1];
void *g_player_model_asset;
uint32_t g_unknown_8009F58C;
uint8_t g_unknown_800BDFD2;
uint8_t g_texture_font_loaded;
TextureFontSlot g_texture_font_slots[TEXTURE_FONT_SLOT_COUNT];
uint32_t g_texture_font_count;
const char g_texture_font_error_condition[] = "font operation";
const char g_texture_font_state_condition[] = "font loaded";
const char g_texture_font_source_path[] = "texture font";

enum EventKind { CALL_50534, CALL_50650, ASSERT_STATE, ASSERT_OPERATION };
typedef struct Event {
    enum EventKind kind;
    int32_t index;
    uint32_t line;
} Event;

static Event events[2 * TEXTURE_FONT_SLOT_COUNT + 1];
static unsigned int event_count;
static int mutate_on_first_call;
static int mutate_on_state_assert;
static uint8_t player_model_mode;
static int mutate_model_mode;
static int32_t mode_calls[4];
static unsigned int mode_call_count;
static void *replacement_asset;
static void *cleaned_state;
static void *freed_asset;

static void record(enum EventKind kind, int32_t index, uint32_t line) {
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].index = index;
    events[event_count].line = line;
    ++event_count;
}

static int32_t visit(void *payload, enum EventKind kind) {
    int32_t index;
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
        if (payload == g_texture_font_slots[index].payload) {
            break;
        }
    }
    assert(index < TEXTURE_FONT_SLOT_COUNT);
    record(kind, index, 0);
    if (mutate_on_first_call) {
        mutate_on_first_call = 0;
        g_texture_font_loaded = 0;
        g_texture_font_slots[49].tag = g_texture_font_slots[index].tag;
    }
    return index % 3 == 0 ? -1 : 0;
}

int32_t func_80050534(void *payload) {
    return visit(payload, CALL_50534);
}

int32_t func_80050650(void *payload) {
    return visit(payload, CALL_50650);
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0);
    assert(source == g_texture_font_source_path);
    if (condition == g_texture_font_state_condition) {
        record(ASSERT_STATE, -1, line);
        if (mutate_on_state_assert) {
            g_texture_font_slots[49].tag = 1;
        }
    } else {
        assert(condition == g_texture_font_error_condition);
        record(ASSERT_OPERATION, -1, line);
    }
}

void func_80062C54(void *state) {
    cleaned_state = state;
    g_player_model_asset = replacement_asset;
}

void func_80082EFC(void *allocation) {
    freed_asset = allocation;
}

uint8_t func_800283E0(void) {
    return player_model_mode;
}

void func_8002BC9C(void) {
    assert(mode_call_count < sizeof(mode_calls) / sizeof(mode_calls[0]));
    mode_calls[mode_call_count++] = -2;
    g_unknown_800BDFD2 = 0xA5;
}

void func_8002CA88(int32_t value, int32_t duration) {
    assert(duration == -1);
    assert(mode_call_count < sizeof(mode_calls) / sizeof(mode_calls[0]));
    mode_calls[mode_call_count++] = value;
    if (mutate_model_mode && value == 5) {
        player_model_mode = 1;
    }
}

static void reset_slots(void) {
    static const uint8_t tags[] = {0, 1, 127, 128, 255};
    int32_t index;
    memset(g_texture_font_slots, 0xA5, sizeof(g_texture_font_slots));
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
        g_texture_font_slots[index].tag = (int8_t)tags[index % 5];
    }
    g_texture_font_count = 0xABCDEF01u;
    event_count = 0;
    mutate_on_first_call = 0;
    mutate_on_state_assert = 0;
}

static void expect_event(unsigned int *at, enum EventKind kind, int32_t index, uint32_t line) {
    assert(*at < event_count);
    assert(events[*at].kind == kind);
    assert(events[*at].index == index);
    assert(events[*at].line == line);
    ++*at;
}

static void test_setters(void) {
    static const int32_t words[] = {0, 1, INT32_MAX, INT32_MIN, -1};
    unsigned int index;
    for (index = 0; index < 256; ++index) {
        g_event_runtime_byte = (uint8_t)index;
        func_8001BC08();
        assert(g_event_runtime_byte == 1);
    }
    for (index = 0; index < sizeof(words) / sizeof(words[0]); ++index) {
        g_unknown_800BE400 = 42;
        assert(func_80061A64(words[index]) == 0);
        assert(g_unknown_800BE400 == words[index]);
    }
}

static void test_callback_leaves(void) {
    static const int32_t values[] = {0, 1, 0x100, 0x101, INT32_MIN, -1};
    static const uint32_t gates[] = {0, 1, UINT32_MAX};
    static const uint8_t modes[] = {0, 1, 2, 255};
    uint8_t assets[2];
    unsigned int value_index, gate_index, mode_index;

    replacement_asset = &assets[1];
    g_player_model_asset = &assets[0];
    cleaned_state = 0;
    freed_asset = 0;
    func_80027F40();
    assert(cleaned_state == g_unknown_800BE4F8);
    assert(freed_asset == &assets[1] && g_player_model_asset == &assets[1]);
    replacement_asset = 0;
    func_80027F40();
    assert(cleaned_state == g_unknown_800BE4F8);
    assert(freed_asset == 0 && g_player_model_asset == 0);

    for (value_index = 0; value_index < sizeof(values) / sizeof(values[0]); ++value_index) {
        for (gate_index = 0; gate_index < sizeof(gates) / sizeof(gates[0]); ++gate_index) {
            for (mode_index = 0; mode_index < sizeof(modes) / sizeof(modes[0]); ++mode_index) {
                unsigned int expected = 0;
                int32_t value = values[value_index];
                g_unknown_8009F58C = gates[gate_index];
                g_unknown_800BDFD2 = 0x5A;
                player_model_mode = modes[mode_index];
                mutate_model_mode = 0;
                mode_call_count = 0;
                func_8002C9D4(value);
                if ((uint8_t)value == 0) {
                    if (gates[gate_index] != 0) {
                        assert(mode_calls[expected++] == 5);
                    }
                    if (modes[mode_index] == 1) {
                        assert(mode_calls[expected++] == -2);
                        assert(mode_calls[expected++] == 15);
                    }
                }
                assert(mode_call_count == expected);
                assert(g_unknown_800BDFD2 == (uint8_t)value);
            }
        }
    }

    g_unknown_8009F58C = 1;
    g_unknown_800BDFD2 = 0x5A;
    player_model_mode = 0;
    mutate_model_mode = 1;
    mode_call_count = 0;
    func_8002C9D4(0x100);
    assert(mode_call_count == 3 && mode_calls[0] == 5 && mode_calls[1] == -2 && mode_calls[2] == 15);
    assert(g_unknown_800BDFD2 == 0);
}

static void test_font_operations(void) {
    static const int32_t tags[] = {0, 1, 127, 128, 255, 256, INT32_MIN + 128, -1};
    TextureFontSlot before[TEXTURE_FONT_SLOT_COUNT];
    unsigned int flag, tag_index, operation;
    for (flag = 0; flag < 256; ++flag) {
        for (tag_index = 0; tag_index < sizeof(tags) / sizeof(tags[0]); ++tag_index) {
            for (operation = 0; operation < 2; ++operation) {
                unsigned int expected = 0;
                int32_t index;
                reset_slots();
                g_texture_font_loaded = (uint8_t)flag;
                memcpy(before, g_texture_font_slots, sizeof(before));
                if (operation == 0) {
                    assert(func_800483C0(tags[tag_index]) == 1);
                } else {
                    assert(func_80048504(tags[tag_index]) == 0);
                }
                if (operation == 0 && flag != 1) {
                    expect_event(&expected, ASSERT_STATE, -1, 0x1BF);
                }
                if (operation == 0 || flag != 0) {
                    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
                        if ((uint8_t)before[index].tag != (uint8_t)tags[tag_index]) {
                            continue;
                        }
                        expect_event(&expected, operation == 0 ? CALL_50534 : CALL_50650, index, 0);
                        if (index % 3 == 0) {
                            expect_event(&expected, ASSERT_OPERATION, -1,
                                         operation == 0 ? 0x1C8 : 0x1FA);
                        }
                    }
                }
                assert(expected == event_count);
                assert(memcmp(before, g_texture_font_slots, sizeof(before)) == 0);
                assert(g_texture_font_count == 0xABCDEF01u);
            }
        }
    }
}

static void test_cleanup(void) {
    static const int32_t tags[] = {0, 1, 127, 128, 255, 256, INT32_MIN + 128, -1};
    static const uint32_t counts[] = {0, 1, 50, UINT32_MAX};
    TextureFontSlot expected[TEXTURE_FONT_SLOT_COUNT];
    unsigned int flag, tag_index, count_index;
    for (flag = 0; flag < 256; ++flag) {
        for (tag_index = 0; tag_index < sizeof(tags) / sizeof(tags[0]); ++tag_index) {
            for (count_index = 0; count_index < sizeof(counts) / sizeof(counts[0]); ++count_index) {
                uint32_t removed = 0;
                int32_t index;
                reset_slots();
                g_texture_font_loaded = (uint8_t)flag;
                g_texture_font_count = counts[count_index];
                memcpy(expected, g_texture_font_slots, sizeof(expected));
                if (flag != 0) {
                    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
                        if ((uint8_t)expected[index].tag == (uint8_t)tags[tag_index]) {
                            expected[index].tag = -1;
                            ++removed;
                        }
                    }
                }
                assert(func_80048C48(tags[tag_index]) == 1);
                assert(g_texture_font_count == counts[count_index] - removed);
                assert(memcmp(expected, g_texture_font_slots, sizeof(expected)) == 0);
                assert(event_count == 0);
            }
        }
    }
}

static void test_callback_mutations(void) {
    unsigned int operation;
    for (operation = 0; operation < 2; ++operation) {
        unsigned int expected = 0;
        int32_t index;
        reset_slots();
        g_texture_font_loaded = 1;
        mutate_on_first_call = 1;
        if (operation == 0) {
            assert(func_800483C0(1) == 1);
        } else {
            assert(func_80048504(1) == 0);
        }
        for (index = 1; index < 50; index += 5) {
            expect_event(&expected, operation == 0 ? CALL_50534 : CALL_50650, index, 0);
            if (index % 3 == 0) {
                expect_event(&expected, ASSERT_OPERATION, -1, operation == 0 ? 0x1C8 : 0x1FA);
            }
        }
        expect_event(&expected, operation == 0 ? CALL_50534 : CALL_50650, 49, 0);
        assert(expected == event_count);
        assert(g_texture_font_loaded == 0);
    }

    reset_slots();
    g_texture_font_loaded = 0;
    mutate_on_state_assert = 1;
    assert(func_800483C0(1) == 1);
    assert(events[0].kind == ASSERT_STATE && events[0].line == 0x1BF);
    assert(events[event_count - 1].kind == CALL_50534 && events[event_count - 1].index == 49);
}

static void test_sweep_with_real_helpers(void) {
    TextureFontSlot before[TEXTURE_FONT_SLOT_COUNT];
    unsigned int flag;
    for (flag = 0; flag < 256; ++flag) {
        unsigned int expected = 0;
        int32_t group;
        int32_t index;
        reset_slots();
        g_texture_font_loaded = (uint8_t)flag;
        g_texture_font_count = 40;
        memcpy(before, g_texture_font_slots, sizeof(before));
        func_80048488();
        if (flag == 0) {
            assert(memcmp(before, g_texture_font_slots, sizeof(before)) == 0);
            assert(g_texture_font_count == 40 && event_count == 0);
            continue;
        }
        for (group = 0; group < 4; ++group) {
            for (index = group; index < TEXTURE_FONT_SLOT_COUNT; index += 5) {
                expect_event(&expected, CALL_50650, index, 0);
                if (index % 3 == 0) {
                    expect_event(&expected, ASSERT_OPERATION, -1, 0x1FA);
                }
            }
        }
        assert(expected == event_count && g_texture_font_count == 0);
        for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
            assert(g_texture_font_slots[index].tag == -1);
        }
    }

    reset_slots();
    g_texture_font_loaded = 1;
    g_texture_font_count = 40;
    mutate_on_first_call = 1;
    func_80048488();
    {
        unsigned int expected = 0;
        int32_t index;
        for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; index += 5) {
            expect_event(&expected, CALL_50650, index, 0);
            if (index % 3 == 0) {
                expect_event(&expected, ASSERT_OPERATION, -1, 0x1FA);
            }
        }
        expect_event(&expected, CALL_50650, 49, 0);
        assert(expected == event_count);
    }
    assert(g_texture_font_loaded == 0 && g_texture_font_count == 40);
    assert(g_texture_font_slots[49].tag == 0);
}

int main(void) {
    test_setters();
    test_callback_leaves();
    test_font_operations();
    test_cleanup();
    test_callback_mutations();
    test_sweep_with_real_helpers();
    puts("Controller-related setters and texture-font slot helpers passed.");
    return 0;
}
