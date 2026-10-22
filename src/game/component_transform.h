#ifndef BUBSY3D_COMPONENT_TRANSFORM_H
#define BUBSY3D_COMPONENT_TRANSFORM_H

#include <stddef.h>
#include <stdint.h>

typedef struct GameComponentTransformView {
    uint16_t matrix_halfwords[9];
    uint16_t unused_halfword_12;
    int32_t translation[3];
} GameComponentTransformView;

typedef struct GameComponentBasisView {
    uint16_t matrix_halfwords[9];
    uint16_t ignored_halfword_12;
    int32_t translation[3];
} GameComponentBasisView;

_Static_assert(sizeof(GameComponentTransformView) == 0x20, "component transform view size must match observed accesses");
_Static_assert(offsetof(GameComponentTransformView, translation) == 0x14, "component translation offset must match observed stores");
_Static_assert(offsetof(GameComponentBasisView, matrix_halfwords) + 8 * sizeof(uint16_t) == 0x10, "basis matrix offset must match observed loads");
_Static_assert(offsetof(GameComponentBasisView, translation) == 0x14, "basis translation offset must match observed loads");
_Static_assert(sizeof(GameComponentBasisView) == 0x20, "basis view size must match observed accesses");

typedef void (*GameComponentPointTransform)(
    void *context,
    const GameComponentBasisView *basis,
    const int32_t input[3],
    int32_t output[3]
);

typedef void (*GameComponentPointToBasisTransform)(
    void *context,
    const GameComponentBasisView *basis,
    const int32_t input[3],
    int32_t output[3]
);

typedef struct GameComponentTransformOps {
    void *context;
    GameComponentPointTransform transform_point;
    GameComponentPointToBasisTransform transform_point_to_basis;
} GameComponentTransformOps;

typedef void (*GameComponentMagnitudeFromSquaredSum)(
    void *context,
    uint32_t squared_sum,
    int32_t *magnitude_out
);

typedef struct GameComponentVectorMagnitudeOps {
    void *context;
    GameComponentMagnitudeFromSquaredSum convert_squared_sum;
} GameComponentVectorMagnitudeOps;

int32_t game_component_transform_set_translation(
    GameComponentTransformView *component,
    const GameComponentBasisView *basis,
    const GameComponentBasisView *global_basis,
    const int32_t translation[3],
    const GameComponentTransformOps *ops
);

int32_t game_component_transform_read_translation(
    const GameComponentTransformView *component,
    const GameComponentBasisView *basis,
    const GameComponentBasisView *global_basis,
    int32_t output[3],
    const GameComponentTransformOps *ops
);

int32_t game_component_transform_read_matrix_vector(
    const GameComponentTransformView *component,
    int32_t output[3]
);

int32_t game_component_basis_transform_point_q12(
    const GameComponentBasisView *basis,
    const int32_t input[4],
    int32_t output[4]
);

int32_t game_component_vector_separation(
    const int32_t left[3],
    const int32_t right[3],
    int32_t *magnitude_out,
    const GameComponentVectorMagnitudeOps *ops
);

#endif