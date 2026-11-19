#include "mode_camera_helpers.h"

extern void func_80072DF0(int32_t first, int32_t second, int32_t third);

int32_t func_8004EF10(int32_t first, int32_t second, int32_t third) {
    if ((uint32_t)first < 0x1000U) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : : : "$4", "$5", "$6");
#endif
        if ((uint32_t)second < 0x1000U && (uint32_t)third < 0x1000U) {
            func_80072DF0(first, second, third);
            g_unknown_800BE5A4[0] = (uint8_t)(first >> 4);
            g_unknown_800BE5A4[1] = (uint8_t)(second >> 4);
            g_unknown_800BE5A4[2] = (uint8_t)(third >> 4);
            return 0;
        }
    }
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : : : "memory");
#endif
    return 1;
}
