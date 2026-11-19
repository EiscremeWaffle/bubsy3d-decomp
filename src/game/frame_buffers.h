#ifndef BUBSY3D_FRAME_BUFFERS_H
#define BUBSY3D_FRAME_BUFFERS_H

#include <stdint.h>

enum {
    FRAME_BUFFER_ENTRY_COUNT = 32,
    FRAME_BUFFER_ENTRY_SIZE = 40
};

typedef struct FrameBufferEntry {
    uint16_t first_halfword;
    uint8_t other_bytes[FRAME_BUFFER_ENTRY_SIZE - 2];
} FrameBufferEntry;

extern int16_t g_frame_buffer_index;
extern uint16_t g_frame_buffer_values_a[2];
extern uint16_t g_frame_buffer_values_b[2];
extern uint16_t g_frame_buffer_selected_values[2];
extern uint32_t g_frame_buffer_sequence;
extern FrameBufferEntry g_frame_buffer_entries[FRAME_BUFFER_ENTRY_COUNT];

/* The incoming index selects one of the two entries in each value table. */
void func_800720FC(void);

#endif
