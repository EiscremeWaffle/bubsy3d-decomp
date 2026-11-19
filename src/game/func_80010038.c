#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
/* Keep the original register and store schedule without emitting instructions. */
#define KEEP_VECTOR_INPUTS(left, right) __asm__ volatile("" : "=r"(left), "=r"(right) : "0"(left), "1"(right))
#define COMMIT_VECTOR_OUTPUT() __asm__ volatile("" : : : "memory")
#else
#define KEEP_VECTOR_INPUTS(left, right) ((void)0)
#define COMMIT_VECTOR_OUTPUT() ((void)0)
#endif

int32_t func_80010038(const int32_t left[3], const int32_t right[3], int32_t output[3]) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t left_value __asm__("$8");
    register uint32_t right_value __asm__("$9");
#else
    uint32_t left_value;
    uint32_t right_value;
#endif

    left_value = (uint32_t)left[0];
    right_value = (uint32_t)right[0];
    KEEP_VECTOR_INPUTS(left_value, right_value);
    output[0] = (int32_t)(left_value + right_value);
    left_value = (uint32_t)left[1];
    right_value = (uint32_t)right[1];
    KEEP_VECTOR_INPUTS(left_value, right_value);
    output[1] = (int32_t)(left_value + right_value);
    left_value = (uint32_t)left[2];
    right_value = (uint32_t)right[2];
    KEEP_VECTOR_INPUTS(left_value, right_value);
    output[2] = (int32_t)(left_value + right_value);
    COMMIT_VECTOR_OUTPUT();
    return 0;
}