#include <stdint.h>

typedef struct ComponentBasisView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentBasisView;

typedef struct ComponentTransformView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentTransformView;

typedef struct ComponentTransformScratch {
    int32_t transformed[3];
    int32_t padding;
    ComponentBasisView normalized_basis;
} ComponentTransformScratch;

extern const ComponentBasisView g_global_basis;
extern int32_t func_8005476C(const ComponentBasisView *basis, ComponentBasisView *output);
extern int32_t func_80064528(const int32_t input[3], const ComponentBasisView *basis, int32_t output[3]);

int32_t func_80054C7C(
    ComponentTransformView *component,
    const ComponentBasisView *basis,
    const int32_t input[3]
) {
    ComponentTransformScratch scratch;
    register ComponentTransformView *component_value __asm__("$17") = component;
    register const ComponentBasisView *basis_value __asm__("$4") = basis;
    register const int32_t *input_value __asm__("$18") = input;
    register ComponentBasisView *normalized_basis __asm__("$16");

    if (component_value == 0) {
        return 0x18;
    }
    __asm__ volatile("" : "=r"(basis_value) : "0"(basis_value));
    __asm__ volatile("" : "=r"(input_value) : "0"(input_value));

    if (basis_value == 0) {
        goto copy_translation;
    }
    if (basis_value->matrix_words[0] != g_global_basis.matrix_words[0]) goto transform_basis;
    if (basis_value->matrix_words[1] != g_global_basis.matrix_words[1]) goto transform_basis;
    if (basis_value->matrix_words[2] != g_global_basis.matrix_words[2]) goto transform_basis;
    if (basis_value->matrix_words[3] != g_global_basis.matrix_words[3]) goto transform_basis;
    if (basis_value->matrix_last != g_global_basis.matrix_last) goto transform_basis;
    if (basis_value->translation[0] != g_global_basis.translation[0]) goto transform_basis;
    if (basis_value->translation[1] != g_global_basis.translation[1]) goto transform_basis;
    if (basis_value->translation[2] != g_global_basis.translation[2]) goto transform_basis;

copy_translation:
    component_value->translation[0] = input_value[0];
    component_value->translation[1] = -input_value[1];
    component_value->translation[2] = input_value[2];
    return 0;

transform_basis:
        normalized_basis = &scratch.normalized_basis;
        func_8005476C(basis_value, normalized_basis);
        func_80064528(input_value, normalized_basis, scratch.transformed);
        func_80054C7C(component_value, 0, scratch.transformed);
        return 0;
}
