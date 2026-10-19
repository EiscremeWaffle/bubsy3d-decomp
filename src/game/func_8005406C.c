#include <stdint.h>

typedef struct ComponentBasisView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentBasisView;

extern int32_t func_80054414(const ComponentBasisView *input, ComponentBasisView *output);
extern int32_t func_80053FA4(const int32_t input[4], const volatile ComponentBasisView *basis, int32_t output[4]);

int32_t func_8005406C(const int32_t input[4], const ComponentBasisView *basis, int32_t output[4]) {
    ComponentBasisView transposed_basis;

    func_80054414(basis, &transposed_basis);
    func_80053FA4(input, &transposed_basis, output);
    return 0;
}