#include <stdint.h>

extern int32_t func_8001007C(int32_t left, int32_t right, int32_t *output);

int32_t func_80053A08(const int32_t *input, int32_t scale, int32_t * volatile output) {
    register int32_t *output_value __asm__("$18");
    register const int32_t *input_value __asm__("$16") = input;
    register int32_t scale_value __asm__("$17") = scale;

    func_8001007C(input_value[0], scale_value, (output_value = output, output));
    func_8001007C(input_value[1], scale_value, &output_value[1]);
    func_8001007C(input_value[2], scale_value, &output_value[2]);
    return 0;
}
