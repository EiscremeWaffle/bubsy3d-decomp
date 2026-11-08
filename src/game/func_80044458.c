#include "reset_entry_helpers.h"

extern void func_80052BDC(void *object, int32_t mode);

uint8_t func_80044458(uint32_t input_index, uint32_t input_mode) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t index __asm__("$17") = input_index;
    register void **table __asm__("$3") = g_reset_entry_table;
    register uint32_t offset __asm__("$2");
    register uint32_t mode_source __asm__("$5") = input_mode;
    register uint32_t mode __asm__("$16");
    /* Preserve target argument setup without emitting instructions. */
    __asm__("" : "=r"(table), "=r"(index), "=r"(mode_source)
            : "0"(table), "1"(index), "2"(mode_source));
    offset = index << 2;
    mode = mode_source;
    __asm__ volatile("" : "=r"(mode), "=r"(offset), "=r"(table)
                     : "0"(mode), "1"(offset), "2"(table));
    func_80052BDC(*(void **)(offset + (uintptr_t)table), 1);
#else
    uint32_t index = input_index;
    uint32_t mode = input_mode;
    func_80052BDC(g_reset_entry_table[index], 1);
#endif
    return (uint8_t)func_80044B24(index, (uint8_t)mode);
}
