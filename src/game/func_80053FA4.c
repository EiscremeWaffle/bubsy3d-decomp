#include <stdint.h>

typedef struct ComponentBasisView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentBasisView;

extern int32_t func_80077298(const volatile ComponentBasisView *basis, const int32_t input[4], int32_t output[4]);
extern const volatile ComponentBasisView g_gte_global_basis;

int32_t func_80053FA4(const int32_t input[4], const volatile ComponentBasisView *basis, int32_t output[4]) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t value0 __asm__("$2");
    register int32_t value1 __asm__("$3");
    register int32_t value2 __asm__("$4");
    register int32_t value3 __asm__("$5");
#else
    int32_t value0;
    int32_t value1;
    int32_t value2;
    int32_t value3;
#endif

    if (basis->matrix_words[0] != g_gte_global_basis.matrix_words[0]) goto transform;
    if (basis->matrix_words[1] != g_gte_global_basis.matrix_words[1]) goto transform;
    if (basis->matrix_words[2] != g_gte_global_basis.matrix_words[2]) goto transform;
    if (basis->matrix_words[3] != g_gte_global_basis.matrix_words[3]) goto transform;
    if (basis->matrix_last != g_gte_global_basis.matrix_last) goto transform;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    value0 = input[0];
    __asm__ volatile("" : : "r"(value0));
    value1 = input[1];
    __asm__ volatile("" : : "r"(value1));
    value2 = input[2];
    __asm__ volatile("" : : "r"(value2));
    value3 = input[3];
    __asm__ volatile("" : : "r"(value3));
#else
    value0 = input[0];
    value1 = input[1];
    value2 = input[2];
    value3 = input[3];
#endif
    output[0] = value0;
    output[1] = value1;
    output[2] = value2;
    output[3] = value3;
    __asm__ volatile("" : : "r"(value3) : "memory");
    return 0;

transform:
    func_80077298(basis, input, output);
    return 0;
}