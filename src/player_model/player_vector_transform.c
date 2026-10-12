#include <stdint.h>

extern int32_t func_8001007C(int32_t left, int32_t right, int32_t *output);
extern int32_t func_8005359C(int32_t value, int32_t *output);
extern int32_t func_80053D68(const int32_t *input, int32_t *output);
extern int32_t func_80053350(int32_t value, int32_t scale, int32_t *output);
extern int32_t func_8005385C(const int32_t *input, int32_t *output);

#if !defined(BUBSY3D_MATCH_ORIGINAL_L0) || defined(BUBSY3D_MATCH_FUNC_8005385C)
int32_t func_8005385C(const int32_t *input, int32_t *output) {
    int32_t components[3];
    int32_t sum;

    components[0] = input[0];
    components[1] = input[1];
    components[2] = input[2];
    if (components[0] != 0) {
        func_8001007C(components[0], components[0], (int32_t *)&components[0]);
    }
    if (components[1] != 0) {
        func_8001007C(components[1], components[1], (int32_t *)&components[1]);
    }
    if (components[2] != 0) {
        func_8001007C(components[2], components[2], (int32_t *)&components[2]);
    }
    sum = components[0] + components[1] + components[2];
    func_8005359C(sum, output);
    return 0;
}
#endif

#ifndef BUBSY3D_MATCH_FUNC_8005385C
int32_t func_8005395C(const int32_t *input, int32_t *output) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t first_scale __asm__("$5");
    register int32_t result __asm__("$2");
#else
    int32_t first_scale;
#endif
    struct VectorTransformScratch {
        volatile int32_t ranged[3];
        volatile int32_t alignment_word;
        volatile int32_t transformed;
    } scratch;

    func_80053D68(input, (int32_t *)scratch.ranged);
    func_8005385C((const int32_t *)scratch.ranged, (int32_t *)&scratch.transformed);
    first_scale = scratch.transformed;
    if (first_scale == 0) {
    #ifdef BUBSY3D_MATCH_ORIGINAL_L0
        result = 7;
        __asm__ volatile(
            "sw $zero,0(%0)\n\t"
            "sw $zero,4(%0)\n\t"
            "sw $zero,8(%0)\n\t"
            :
            : "r"(output)
        );
        return result;
    #else
        output[0] = 0;
        output[1] = 0;
        output[2] = 0;
        return 7;
#endif
    }

    func_80053350(scratch.ranged[0], first_scale, &output[0]);
    func_80053350(scratch.ranged[1], scratch.transformed, &output[1]);
    func_80053350(scratch.ranged[2], scratch.transformed, &output[2]);
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    result = 0;
    return result;
#else
    return 0;
#endif
}
#endif