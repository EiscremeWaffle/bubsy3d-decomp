#include "transform_reset_backend.h"

extern void *g_l0_gp_888;
extern int32_t g_transform_reset_status;
extern uint32_t g_transform_reset_snapshot[8];
extern TransformResetDescriptor g_transform_reset_descriptor;

int32_t func_80051678(void) {
    const volatile uint32_t *source = g_l0_gp_888;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    /* Preserve the original destination and grouped-copy registers. */
    register volatile uint32_t *destination __asm__("$6");
    register uint32_t first __asm__("$3");
    register uint32_t second __asm__("$4");
    register uint32_t third __asm__("$5");
#else
    volatile uint32_t *destination;
    uint32_t first;
    uint32_t second;
    uint32_t third;
#endif

    g_transform_reset_status = 0;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : : : "memory");
#endif
    destination = g_transform_reset_snapshot;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(destination) : "0"(destination));
#endif
    /* Each group is loaded before its stores, including when buffers overlap. */
    first = source[0];
    second = source[1];
    third = source[2];
    destination[0] = first;
    destination[1] = second;
    destination[2] = third;
    first = source[3];
    second = source[4];
    third = source[5];
    destination[3] = first;
    destination[4] = second;
    destination[5] = third;
    first = source[6];
    second = source[7];
    destination[6] = first;
    destination[7] = second;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : : : "memory");
#endif
    func_800731C0(&g_transform_reset_descriptor);
    return 0;
}
