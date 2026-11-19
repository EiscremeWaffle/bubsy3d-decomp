#include "frame_buffers.h"

extern void func_8006D434(uint16_t values[2]);
extern void func_8006CCC4(int32_t mode);
extern void func_8007201C(void);
extern void func_80071F00(void);

void func_800720FC(void) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t index __asm__("$2") = g_frame_buffer_index;
    register uint16_t first __asm__("$3") = g_frame_buffer_values_a[index];
    register uint16_t *selected __asm__("$4");
    register uint32_t previous __asm__("$2");
    register uint32_t incremented __asm__("$3");
#else
    int32_t index = g_frame_buffer_index;
    uint16_t first = g_frame_buffer_values_a[index];
    uint16_t *selected;
    uint32_t previous;
    uint32_t incremented;
#endif
    int32_t offset;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    /* Preserve the original indexed loads and branch-delay scheduling. */
    __asm__ volatile("" : "=r"(first) : "0"(first));
#endif
    selected = g_frame_buffer_selected_values;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(selected) : "0"(selected));
#endif
    selected[0] = first;
    g_frame_buffer_selected_values[1] = g_frame_buffer_values_b[index];
    func_8006D434(selected);
    func_8006CCC4(1);
    if (g_frame_buffer_index == 0) {
        *(volatile int16_t *)&g_frame_buffer_index = 1;
    } else {
        *(volatile int16_t *)&g_frame_buffer_index = 0;
    }
    func_8007201C();
    func_80071F00();

    previous = g_frame_buffer_sequence;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(previous) : "0"(previous));
#endif
    incremented = previous + 1U;
    *(volatile uint32_t *)&g_frame_buffer_sequence = incremented;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(previous), "=r"(incremented) : "0"(previous), "1"(incremented) : "$4");
#endif
    *(volatile uint32_t *)&g_frame_buffer_sequence = previous == 0 ? 1U : incremented;

    offset = (FRAME_BUFFER_ENTRY_COUNT - 1) * FRAME_BUFFER_ENTRY_SIZE;
    do {
        *(uint16_t *)((uint8_t *)g_frame_buffer_entries + offset) = 0;
        offset -= FRAME_BUFFER_ENTRY_SIZE;
    } while (offset >= 0);
}
