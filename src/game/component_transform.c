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
        normalized_basis = *basis;
        for (index = 1; index < 8; index += 2) {
            normalized_basis.matrix_halfwords[index] = (uint16_t)(0U - normalized_basis.matrix_halfwords[index]);
        }
        normalized_basis.translation[1] = component_translation_negate(normalized_basis.translation[1]);
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