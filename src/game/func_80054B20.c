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
    int32_t component_translation[3];
    int32_t padding;
    ComponentBasisView normalized_basis;
} ComponentTransformScratch;

extern const ComponentBasisView g_global_basis;
extern int32_t func_8005476C(const ComponentBasisView *basis, ComponentBasisView *output);
extern int32_t func_80064548(const int32_t input[3], const ComponentBasisView *basis, int32_t output[3]);

int32_t func_80054B20(
    const ComponentTransformView *component,
    const ComponentBasisView *basis,
    int32_t output[3]
) {
    ComponentTransformScratch scratch;
    register const ComponentBasisView *basis_value __asm__("$16") = basis;
    register int32_t *output_value __asm__("$17") = output;

    if (component == 0) {
        return 0x18;
    }
    __asm__ volatile("" : "=r"(output_value) : "0"(output_value));

    if (basis_value == 0) {
        goto copy_translation;
    }
    __asm__ volatile("" : "=r"(basis_value) : "0"(basis_value));
    if (basis_value->matrix_words[0] != g_global_basis.matrix_words[0]) goto transform_basis;
    if (basis_value->matrix_words[1] != g_global_basis.matrix_words[1]) goto transform_basis;
    if (basis_value->matrix_words[2] != g_global_basis.matrix_words[2]) goto transform_basis;
    if (basis_value->matrix_words[3] != g_global_basis.matrix_words[3]) goto transform_basis;
    if (basis_value->matrix_last != g_global_basis.matrix_last) goto transform_basis;
    if (basis_value->translation[0] != g_global_basis.translation[0]) goto transform_basis;
    if (basis_value->translation[1] != g_global_basis.translation[1]) goto transform_basis;
    if (basis_value->translation[2] != g_global_basis.translation[2]) goto transform_basis;

copy_translation:
    output_value[0] = component->translation[0];
    output_value[1] = -component->translation[1];
    output_value[2] = component->translation[2];
    return 0;

transform_basis:
    func_80054B20(component, 0, scratch.component_translation);
    func_8005476C(basis_value, &scratch.normalized_basis);
    basis_value = &scratch.normalized_basis;
    func_80064548(scratch.component_translation, basis_value, output_value);
    return 0;
}
