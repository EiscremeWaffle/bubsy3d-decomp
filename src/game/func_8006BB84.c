#include <stdint.h>

extern volatile uint32_t * volatile g_unknown_800A436C;
extern volatile uint32_t * volatile g_unknown_800A4370;
extern volatile uint32_t g_unknown_800A4374;
extern volatile uint32_t g_unknown_800A4378;
extern volatile uint32_t g_unknown_800A5438;
extern void func_8006BCCC(uint32_t position, int32_t count);

uint32_t func_8006BB84(int32_t count) {
    uint32_t initial_tick;
    uint32_t raw_tick_delta;
    int32_t next_count;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register volatile uint32_t *current_tick_pointer __asm__("$2");
    register volatile uint32_t *tick_pointer __asm__("$3");
    register volatile uint32_t *loop_tick_pointer __asm__("$3");
    register uint32_t tick_delta __asm__("$17");
    register uint32_t current_tick_value __asm__("$2");
    register uint32_t previous_tick_value __asm__("$3");
    register uint32_t position __asm__("$2");
#else
    volatile uint32_t *current_tick_pointer;
    volatile uint32_t *tick_pointer;
    volatile uint32_t *loop_tick_pointer;
    uint32_t tick_delta;
    uint32_t current_tick_value;
    uint32_t previous_tick_value;
    uint32_t position;
#endif

    current_tick_pointer = g_unknown_800A436C;
    tick_pointer = g_unknown_800A4370;
    initial_tick = *current_tick_pointer;
    current_tick_value = *tick_pointer;
    previous_tick_value = g_unknown_800A4374;
    raw_tick_delta = current_tick_value - previous_tick_value;
    tick_delta = (uint16_t)raw_tick_delta;
    if (count < 0) {
        return g_unknown_800A5438;
    }
    if (count == 1) {
        return tick_delta;
    }

    if (count > 0) {
        position = g_unknown_800A4378 - 1;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(position) : "0"(position));
#endif
        position += (uint32_t)count;
    } else {
        position = g_unknown_800A4378;
    }
    next_count = count > 0 ? count - 1 : 0;
    func_8006BCCC(position, next_count);

    initial_tick = *g_unknown_800A436C;
    func_8006BCCC(g_unknown_800A5438 + 1, 1);
    if ((initial_tick & 0x00080000U) != 0) {
        loop_tick_pointer = g_unknown_800A436C;
        while (((*loop_tick_pointer ^ initial_tick) & 0x80000000U) == 0) {
        }
    }

    position = g_unknown_800A5438;
    tick_pointer = g_unknown_800A4370;
    g_unknown_800A4378 = position;
    g_unknown_800A4374 = *tick_pointer;
    return tick_delta;
}
