#include <stdint.h>

extern int32_t func_80053350(int32_t value, int32_t scale, int32_t *output);

int32_t func_80053B70(const int32_t *input, int32_t scale, int32_t *output) {
    func_80053350(input[0], scale, &output[0]);
    func_80053350(input[1], scale, &output[1]);
    func_80053350(input[2], scale, &output[2]);
    return 0;
}
