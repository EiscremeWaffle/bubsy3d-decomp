#include "controller_input.h"
#include "frame_buffers.h"
#include "tick_history.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int32_t g_input_stream_sample;
volatile uint32_t g_tick_window_reference;
volatile uint32_t g_tick_window_snapshot;
volatile uint32_t g_tick_window_count;
volatile uint32_t g_tick_total_count;
volatile uint32_t g_tick_history_index;
uint32_t g_tick_history[100];
int16_t g_frame_buffer_index;
uint16_t g_frame_buffer_values_a[2];
uint16_t g_frame_buffer_values_b[2];
uint16_t g_frame_buffer_selected_values[2];
uint32_t g_frame_buffer_sequence;
FrameBufferEntry g_frame_buffer_entries[FRAME_BUFFER_ENTRY_COUNT];

static unsigned calls;

void func_8006C8C0(int32_t mode) {
    assert(calls++ == 0);
    assert(mode == 1);
    assert(g_tick_window_reference == 160);
    assert(g_tick_window_snapshot == 4);
    assert(g_tick_window_count == 0);
    assert(g_tick_total_count == 10);
    assert(g_tick_history_index == 0);
    assert(g_tick_history[0] == 160);
    g_frame_buffer_index = 1;
}

void func_8006D434(uint16_t values[2]) {
    assert(calls++ == 1);
    assert(values == g_frame_buffer_selected_values);
    assert(values[0] == 0x2222 && values[1] == 0x4444);
    g_frame_buffer_index = 4;
}

void func_8006CCC4(int32_t mode) {
    assert(calls++ == 2);
    assert(mode == 1);
    assert(g_frame_buffer_index == 4);
    g_frame_buffer_index = 0;
}

void func_8007201C(void) {
    assert(calls++ == 3);
    assert(g_frame_buffer_index == 1);
}

void func_80071F00(void) {
    assert(calls++ == 4);
    g_frame_buffer_sequence = UINT32_MAX;
}

uint32_t func_80051360(void) {
    unsigned index;
    assert(calls++ == 5);
    assert(g_frame_buffer_sequence == 0);
    for (index = 0; index < FRAME_BUFFER_ENTRY_COUNT; ++index) {
        assert(g_frame_buffer_entries[index].first_halfword == 0);
        assert(g_frame_buffer_entries[index].other_bytes[0] == 0xA5);
    }
    return 0x1234ABCDU;
}

int main(void) {
    g_input_stream_sample = 160;
    g_tick_window_reference = 100;
    g_tick_window_snapshot = 0;
    g_tick_window_count = 3;
    g_tick_total_count = 9;
    g_tick_history_index = 99;
    g_frame_buffer_index = 0;
    g_frame_buffer_values_a[0] = 0x1111;
    g_frame_buffer_values_a[1] = 0x2222;
    g_frame_buffer_values_b[0] = 0x3333;
    g_frame_buffer_values_b[1] = 0x4444;
    memset(g_frame_buffer_entries, 0xA5, sizeof(g_frame_buffer_entries));

    assert(func_8004F688() == 0xCD);
    assert(calls == 6);
    puts("tick/frame integration tests passed");
    return 0;
}
