#include "controller_input.h"
#include "tick_history.h"

extern void func_8006C8C0(int32_t mode);
extern void func_800720FC(void);
extern uint32_t func_80051360(void);

uint8_t func_8004F688(void) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t previous_window __asm__("$2");
    register uint32_t window_count __asm__("$4");
    register uint32_t history_index __asm__("$3");
    register uint32_t status __asm__("$3");
#else
    uint32_t window_count;
    uint32_t history_index;
    uint32_t status;
#endif
    uint32_t total_count;
    uint32_t sample;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    previous_window = g_tick_window_count;
    history_index = g_tick_history_index;
    __asm__ volatile("" : "=r"(previous_window), "=r"(history_index)
                     : "0"(previous_window), "1"(history_index));
    window_count = previous_window + 1U;
    history_index += 1U;
#else
    window_count = g_tick_window_count + 1U;
    history_index = g_tick_history_index + 1U;
#endif
    total_count = g_tick_total_count + 1U;
    g_tick_window_count = window_count;
    g_tick_history_index = history_index;
    g_tick_total_count = total_count;
    if (history_index == 100U) {
        g_tick_history_index = 0;
    }

    /* The history write and the elapsed-time check each read the live sample. */
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    g_tick_history[g_tick_history_index] = (uint32_t)*(volatile int32_t *)&g_input_stream_sample;
    __asm__ volatile("" : : : "memory");
#else
    ((volatile uint32_t *)g_tick_history)[g_tick_history_index] =
        (uint32_t)*(volatile int32_t *)&g_input_stream_sample;
#endif
    sample = (uint32_t)*(volatile int32_t *)&g_input_stream_sample;
    if (sample - g_tick_window_reference >= 60U) {
        g_tick_window_reference = sample;
        g_tick_window_snapshot = window_count;
        g_tick_window_count = 0;
    }
    func_8006C8C0(1);
    func_800720FC();
    status = func_80051360() & 0xFFU;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    if (status == 0) {
        __asm__ volatile("" : "=r"(status) : "0"(status));
        return 0;
    }
#endif
    return (uint8_t)status;
}
