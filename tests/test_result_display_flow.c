#include "controller_callback_helpers.h"
#include "controller_result.h"
#include "result_display.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

int16_t g_result_previous_selection;
const char g_result_player_one_text[] = "one";
const char g_result_player_two_text[] = "two";
uint32_t g_unknown_8009F58C;
uint8_t g_result_display_phase;
int32_t g_result_display_scale;
int8_t g_result_display_tag;
GroundedEventRecord *g_result_display_event;

static ControllerResultContext context;
static GroundedEventRecord event_record;
static unsigned font_creations, scale_updates, font_enables, font_removals;
static unsigned font_releases, event_creations, event_removals, followup_calls;

void *func_80026648(void *argument0, void *argument1) {
    assert(argument0 == &context && argument1 == 0);
    return &context;
}

void func_800486D0(void *text, int32_t first, int32_t second, int8_t *tag) {
    assert(text == g_result_player_two_text && first == -1 && second == 0xF0);
    assert(tag == &g_result_display_tag && g_result_display_phase == 1 && g_result_display_scale == 0);
    *tag = -2;
    context.selection_14 = 0;
    ++font_creations;
}

int32_t func_800481D4(int32_t tag, int32_t first, int32_t second) {
    assert(tag == -2);
    assert(first <= 0x1000 && second <= 0x2C00);
    ++scale_updates;
    return 0;
}

int32_t func_800483C0(int32_t tag) {
    assert(tag == -2);
    ++font_enables;
    return 0;
}

int32_t func_80048504(int32_t tag) {
    assert(tag == -2);
    ++font_removals;
    return 0;
}

int32_t func_80048C48(int32_t tag) {
    assert(tag == -2);
    ++font_releases;
    return 0;
}

int32_t func_8004FF50(void *actor, void *value, GroundedEventRecord **event_out) {
    assert(actor == (void *)func_8002DE34 && (uintptr_t)value == 4 && event_out == &g_result_display_event);
    *event_out = &event_record;
    ++event_creations;
    return 0;
}

int32_t func_8004FFEC(GroundedEventRecord *event) {
    assert(event == &event_record && g_result_display_phase == 0 && g_result_display_scale == 0x1000);
    ++event_removals;
    return 0;
}

void func_8002C604(int32_t value) {
    assert(value == 6 || value == 7);
    ++followup_calls;
}

int32_t func_8002BD0C(int32_t value) {
    assert(value == 17);
    ++followup_calls;
    return 123;
}

void func_8002CA88(int32_t value, int32_t duration) {
    assert(value == 17 && duration == 123);
    ++followup_calls;
}

void func_8002E1E0(int32_t value) {
    assert(value == 0);
    ++followup_calls;
}

void func_8002E348(int32_t value) {
    assert(value == 0);
    ++followup_calls;
}

void func_8002BC7C(void) { assert(0); }
void func_8002C010(void) { assert(0); }
void func_8002C1F0(void) { assert(0); }
uint8_t func_800283E0(void) { assert(0); return 0; }
void func_8002BC9C(void) { assert(0); }

int main(void) {
    unsigned ticks = 0;

    context.mode_0C = 1;
    context.selection_14 = 1;
    func_8002C370(&context, 0);
    assert(g_result_previous_selection == 0 && g_result_display_event == &event_record);
    assert(font_creations == 1 && font_enables == 1 && event_creations == 1 && followup_calls == 6);
    while (g_result_display_event != 0 && ticks < 100) {
        func_8002DE34();
        ++ticks;
    }
    assert(ticks == 55 && g_result_display_event == 0);
    assert(g_result_display_phase == 0 && g_result_display_scale == 0x1000);
    assert(scale_updates == 57 && font_removals == 1 && font_releases == 1 && event_removals == 1);
    puts("result display flow tests passed");
    return 0;
}
