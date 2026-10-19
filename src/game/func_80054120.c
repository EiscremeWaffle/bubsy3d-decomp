#include <stdint.h>

typedef struct ComponentBasisView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentBasisView;

extern int32_t func_80010000(const int32_t left[3], const int32_t right[3], int32_t output[3]);
extern int32_t func_8005406C(const int32_t input[4], const ComponentBasisView *basis, int32_t output[4]);

int32_t func_80054120(const int32_t input[4], const ComponentBasisView *basis, int32_t output[4]) {
    int32_t basis_translation[4];
    int32_t difference[4];

    basis_translation[0] = basis->translation[0];
    basis_translation[1] = basis->translation[1];
    basis_translation[2] = basis->translation[2];
    func_80010000(input, basis_translation, difference);
    func_8005406C(difference, basis, output);
    return 0;
}