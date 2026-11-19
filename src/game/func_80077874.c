#include "gte_background_color.h"

#ifndef BUBSY3D_MATCH_ORIGINAL_L0
uint32_t g_gte_background_color[3];
#endif

void func_80077874(int32_t first, int32_t second, int32_t third) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t red __asm__("$4") = (uint32_t)first << 4;
    register uint32_t green __asm__("$5") = (uint32_t)second << 4;
    register uint32_t blue __asm__("$6") = (uint32_t)third << 4;

    /* CTC2 writes GTE control registers 13, 14 and 15; MIPS-I assembly lacks the mnemonic. */
    __asm__ volatile(
        ".word 0x48C46800\n\t"
        ".word 0x48C57000\n\t"
        ".word 0x48C67800"
        : : "r"(red), "r"(green), "r"(blue) : "memory"
    );
#else
    g_gte_background_color[0] = (uint32_t)first << 4;
    g_gte_background_color[1] = (uint32_t)second << 4;
    g_gte_background_color[2] = (uint32_t)third << 4;
#endif
}
