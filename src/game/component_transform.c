#include "component_transform.h"

#include <string.h>

static int game_component_basis_matches_global(
    const GameComponentBasisView *basis,
    const GameComponentBasisView *global_basis
) {
    return global_basis != NULL
        && memcmp(basis->matrix_halfwords, global_basis->matrix_halfwords, sizeof(basis->matrix_halfwords)) == 0
        && memcmp(basis->translation, global_basis->translation, sizeof(basis->translation)) == 0;
}

static int32_t component_translation_negate(int32_t value) {
    const uint32_t bits = 0U - (uint32_t)value;
    int32_t result;

    memcpy(&result, &bits, sizeof(result));
    return result;
}

static int32_t component_translation_add(int32_t left, int32_t right) {
    const uint32_t bits = (uint32_t)left + (uint32_t)right;
    int32_t result;

    memcpy(&result, &bits, sizeof(result));
    return result;
}

static void component_basis_normalize(
    GameComponentBasisView *output,
    const GameComponentBasisView *input
) {
    size_t index;

    *output = *input;
    for (index = 1; index < 8; index += 2) {
        output->matrix_halfwords[index] = (uint16_t)(0U - output->matrix_halfwords[index]);
    }
    output->translation[1] = component_translation_negate(output->translation[1]);
}

int32_t game_component_transform_set_translation(
    GameComponentTransformView *component,
    const GameComponentBasisView *basis,
    const GameComponentBasisView *global_basis,
    const int32_t translation[3],
    const GameComponentTransformOps *ops
) {
    GameComponentBasisView normalized_basis;
    int32_t converted_translation[3];
    size_t index;

    if (component == NULL) {
        return 0x18;
    }

    if (basis == NULL || game_component_basis_matches_global(basis, global_basis)) {
        converted_translation[0] = translation[0];
        converted_translation[1] = translation[1];
        converted_translation[2] = translation[2];
    } else {
        component_basis_normalize(&normalized_basis, basis);
        ops->transform_vector(ops->context, &normalized_basis, translation, converted_translation);
        for (index = 0; index < 3; index++) {
            converted_translation[index] = component_translation_add(
                converted_translation[index],
                normalized_basis.translation[index]
            );
        }
    }

    component->translation[0] = converted_translation[0];
    component->translation[1] = component_translation_negate(converted_translation[1]);
    component->translation[2] = converted_translation[2];
    return 0;
}

int32_t game_component_transform_read_translation(
    const GameComponentTransformView *component,
    const GameComponentBasisView *basis,
    const GameComponentBasisView *global_basis,
    int32_t output[3],
    const GameComponentTransformOps *ops
) {
    GameComponentBasisView normalized_basis;
    int32_t component_translation[3];

    if (component == NULL) {
        return 0x18;
    }

    if (basis == NULL || game_component_basis_matches_global(basis, global_basis)) {
        output[0] = component->translation[0];
        output[1] = component_translation_negate(component->translation[1]);
        output[2] = component->translation[2];
        return 0;
    }

    game_component_transform_read_translation(component, NULL, NULL, component_translation, NULL);
    component_basis_normalize(&normalized_basis, basis);
    ops->transform_point_to_basis(ops->context, &normalized_basis, component_translation, output);
    return 0;
}