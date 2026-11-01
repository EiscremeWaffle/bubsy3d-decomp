#include <stdint.h>

extern int32_t func_8001007C(int32_t left, int32_t right, int32_t *output);

int32_t func_80053A08(const int32_t *input, int32_t scale, int32_t *output) {
    const int32_t *input_value = input;
    int32_t scale_value = scale;
    int32_t *output_value = output;

    func_8001007C(input_value[0], scale_value, output_value);
    func_8001007C(input_value[1], scale_value, &output_value[1]);
    func_8001007C(input_value[2], scale_value, &output_value[2]);
    return 0;
}
