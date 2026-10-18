#include <stdint.h>

uint32_t func_8001DCE0(void) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register volatile uint32_t *gp __asm__("$28");
    return gp[0x170 / sizeof(*gp)];
#else
    extern uint32_t g_l0_gp_170;
    return g_l0_gp_170;
#endif
}