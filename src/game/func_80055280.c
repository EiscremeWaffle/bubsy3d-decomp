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

extern const ComponentBasisView g_component_identity_basis;
extern int32_t func_80054B20(
    const ComponentTransformView *component,
    const ComponentBasisView *basis,
    int32_t output[3]
);
extern int32_t func_80010038(
    const int32_t left[3],
    const int32_t right[3],
    int32_t output[3]
);
extern int32_t func_80054C7C(
    ComponentTransformView *component,
    const ComponentBasisView *basis,
    const int32_t translation[3]
);
extern int32_t func_800550A4(
    ComponentTransformView *component,
    const ComponentBasisView *basis
);

int32_t func_80055280(
    ComponentTransformView *component,
    const ComponentBasisView *basis,
    const int32_t translation_delta[3]
) {
    int32_t component_translation[3];
    int32_t translated_component[3];
    ComponentBasisView translated_basis;

    translated_basis = g_component_identity_basis;
    if (component == 0) {
        return 0x18;
    }
    if (translation_delta == 0) {
        return 1;
    }

    func_80054B20(component, basis, component_translation);
    func_80010038(component_translation, translation_delta, translated_component);
    func_80054C7C(&translated_basis, basis, translated_component);
    func_800550A4(component, &translated_basis);
    return 0;
}