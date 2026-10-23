#include <stdint.h>

extern int32_t func_800535CC(int32_t squared_length, int32_t *length_out);

int32_t move_vector_has_nonzero_squared_length(
    const int32_t input[3],
    int32_t *result_out
) __asm__("func_80053904");

int32_t move_vector_has_nonzero_squared_length(
    const int32_t input[3],
    int32_t *result_out
) {
    uint32_t squared_length =
        (uint32_t)input[0] * (uint32_t)input[0] +
        (uint32_t)input[1] * (uint32_t)input[1] +
        (uint32_t)input[2] * (uint32_t)input[2];

    func_800535CC((int32_t)squared_length, result_out);
    return 0;
}