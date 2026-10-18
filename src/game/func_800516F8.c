#include <stdint.h>

int32_t func_800516F8(void **output) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register void * volatile *gp __asm__("$28");
    *output = gp[0x888 / sizeof(*gp)];
#else
    extern void *g_l0_gp_888;
    *output = g_l0_gp_888;
#endif
    return 0;
}