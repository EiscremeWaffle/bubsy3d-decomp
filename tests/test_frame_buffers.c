#include "frame_buffers.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int16_t g_frame_buffer_index;
uint16_t g_frame_buffer_values_a[2];
uint16_t g_frame_buffer_values_b[2];
uint16_t g_frame_buffer_selected_values[2];
uint32_t g_frame_buffer_sequence;
FrameBufferEntry g_frame_buffer_entries[FRAME_BUFFER_ENTRY_COUNT];

static unsigned calls;
static int16_t selected_index;
static int16_t index_after_sync;
static uint32_t sequence_after_callbacks;

void func_8006D434(uint16_t values[2]) {
    assert(calls++ == 0);
    assert(values == g_frame_buffer_selected_values);
    assert(values[0] == g_frame_buffer_values_a[selected_index]);
    assert(values[1] == g_frame_buffer_values_b[selected_index]);
    values[0] = 0x1357;
    g_frame_buffer_index = 4;
}

void func_8006CCC4(int32_t mode) {
    assert(calls++ == 1);
    assert(mode == 1);
    assert(g_frame_buffer_index == 4);
    assert(g_frame_buffer_selected_values[0] == 0x1357);
    g_frame_buffer_index = index_after_sync;
}

void func_8007201C(void) {
    assert(calls++ == 2);
    assert(g_frame_buffer_index == (index_after_sync == 0 ? 1 : 0));
    g_frame_buffer_sequence = 0xAABBCCDDU;
    g_frame_buffer_entries[5].first_halfword = 0x3456;
}

void func_80071F00(void) {
    assert(calls++ == 3);
    assert(g_frame_buffer_sequence == 0xAABBCCDDU);
    assert(g_frame_buffer_entries[5].first_halfword == 0x3456);
    g_frame_buffer_sequence = sequence_after_callbacks;
}

static void check_case(int16_t index, int16_t after_sync, uint32_t sequence) {
    unsigned entry;
    unsigned byte;

    calls = 0;
    selected_index = index;
    index_after_sync = after_sync;
    sequence_after_callbacks = sequence;
    g_frame_buffer_index = index;
    g_frame_buffer_values_a[0] = 0x0123;
    g_frame_buffer_values_a[1] = 0x4567;
    g_frame_buffer_values_b[0] = 0x89AB;
    g_frame_buffer_values_b[1] = 0xCDEF;
    g_frame_buffer_selected_values[0] = 0;
    g_frame_buffer_selected_values[1] = 0;
    g_frame_buffer_sequence = 0x87654321U;
    memset(g_frame_buffer_entries, 0xA5, sizeof(g_frame_buffer_entries));

    func_800720FC();
    assert(calls == 4);
    assert(g_frame_buffer_selected_values[0] == 0x1357);
    assert(g_frame_buffer_selected_values[1] == g_frame_buffer_values_b[index]);
    assert(g_frame_buffer_index == (after_sync == 0 ? 1 : 0));
    assert(g_frame_buffer_sequence == sequence + 1U);
    for (entry = 0; entry < FRAME_BUFFER_ENTRY_COUNT; ++entry) {
        assert(g_frame_buffer_entries[entry].first_halfword == 0);
        for (byte = 0; byte < sizeof(g_frame_buffer_entries[entry].other_bytes); ++byte) {
            assert(g_frame_buffer_entries[entry].other_bytes[byte] == 0xA5);
        }
    }
}

int main(void) {
    assert(sizeof(FrameBufferEntry) == FRAME_BUFFER_ENTRY_SIZE);
    assert(offsetof(FrameBufferEntry, other_bytes) == 2);
    check_case(0, 0, 0);
    check_case(1, 1, 9);
    check_case(1, -3, UINT32_MAX);
    check_case(0, 2, 0x7FFFFFFFU);
    puts("frame buffer tests passed");
    return 0;
}
