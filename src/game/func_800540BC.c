#include <stdint.h>

typedef struct ComponentBasisView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentBasisView;

extern int32_t func_80053FA4(const int32_t input[4], const ComponentBasisView *basis, int32_t output[4]);
extern int32_t func_80010038(const int32_t left[3], const int32_t right[3], int32_t output[3]);

int32_t func_800540BC(const int32_t input[4], const ComponentBasisView *basis, int32_t output[3]) {
    int32_t transformed[4];
    int32_t translated[4];
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register const ComponentBasisView *basis_value __asm__("$16") = basis;
    register int32_t *output_value __asm__("$17") = output;
#else
    const ComponentBasisView *basis_value = basis;
    int32_t *output_value = output;
#endif

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : : "r"(basis_value));
    __asm__ volatile("" : : "r"(output_value));
#endif

    func_80053FA4(input, basis_value, transformed);
    translated[0] = basis_value->translation[0];
    translated[1] = basis_value->translation[1];
    translated[2] = basis_value->translation[2];
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    {
        register const int32_t *translation_argument __asm__("$5");
        register int32_t *output_argument __asm__("$6");

        translation_argument = translated;
        __asm__ volatile("" : "=r"(translation_argument) : "0"(translation_argument));
        output_argument = output_value;
        func_80010038(transformed, translation_argument, output_argument);
    }
#else
    func_80010038(transformed, translated, output_value);
#endif
    return 0;
}