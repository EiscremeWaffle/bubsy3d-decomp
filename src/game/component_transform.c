#include "component_transform.h"

#include <string.h>

static int game_component_basis_matches_global(
    const GameComponentBasisView *basis,
    const GameComponentBasisView *global_basis
) {
    return global_basis != NULL
        && memcmp(basis->words_00_0c, global_basis->words_00_0c, sizeof(basis->words_00_0c)) == 0
        && basis->halfword_10 == global_basis->halfword_10
        && memcmp(basis->words_14_1c, global_basis->words_14_1c, sizeof(basis->words_14_1c)) == 0;
}

static int32_t component_translation_negate(int32_t value) {
    const uint32_t bits = 0U - (uint32_t)value;
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
    int32_t converted_translation[3];

    if (component == NULL) {
        return 0x18;
    }

    if (basis == NULL || game_component_basis_matches_global(basis, global_basis)) {
        converted_translation[0] = translation[0];
        converted_translation[1] = component_translation_negate(translation[1]);
        converted_translation[2] = translation[2];
    } else {
        ops->transform_vector(ops->context, basis, translation, converted_translation);
        converted_translation[1] = component_translation_negate(converted_translation[1]);
    }

    component->words[0x14 / sizeof(int32_t)] = converted_translation[0];
    component->words[0x18 / sizeof(int32_t)] = converted_translation[1];
    component->words[0x1C / sizeof(int32_t)] = converted_translation[2];
    return 0;
}