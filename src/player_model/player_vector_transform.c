#include <stdint.h>

extern int32_t func_8001007C(int32_t left, int32_t right, int32_t *output);
extern int32_t func_8005359C(int32_t value, int32_t *output);
extern int32_t func_80053D68(const int32_t *input, int32_t *output);
extern int32_t func_80053350(int32_t value, int32_t scale, int32_t *output);

int32_t func_8005385C(const int32_t *input, int32_t *output) {
    volatile int32_t components[3];
    int32_t sum;
    int index;

    components[0] = input[0];
    components[1] = input[1];
    components[2] = input[2];
    for (index = 0; index < 3; index++) {
        if (components[index] != 0) {
            func_8001007C(components[index], components[index], (int32_t *)&components[index]);
        }
    }
    sum = components[0] + components[1] + components[2];
    func_8005359C(sum, output);
    return 0;
}

int32_t func_8005395C(const int32_t *input, int32_t *output) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t scale __asm__("$5");
#else
    int32_t scale;
#endif
    struct VectorTransformScratch {
        volatile int32_t ranged[3];
        volatile int32_t alignment_word;
        volatile int32_t transformed[3];
    } scratch;

    func_80053D68(input, (int32_t *)scratch.ranged);
    func_8005385C((const int32_t *)scratch.ranged, (int32_t *)scratch.transformed);
    scale = scratch.transformed[0];
    if (scale == 0) {
        output[0] = 0;
        output[1] = 0;
        output[2] = 0;
        return 7;
    }

    func_80053350(scratch.ranged[0], scale, &output[0]);
    func_80053350(scratch.ranged[1], scale, &output[1]);
    func_80053350(scratch.ranged[2], scale, &output[2]);
    return 0;
}