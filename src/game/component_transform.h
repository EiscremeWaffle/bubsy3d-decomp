#ifndef BUBSY3D_COMPONENT_TRANSFORM_H
#define BUBSY3D_COMPONENT_TRANSFORM_H

#include <stddef.h>
#include <stdint.h>

typedef struct GameComponentTransformView {
    int32_t words[8];
} GameComponentTransformView;

typedef struct GameComponentBasisView {
    uint32_t words_00_0c[4];
    int16_t halfword_10;
    uint16_t ignored_halfword_12;
    uint32_t words_14_1c[3];
} GameComponentBasisView;

_Static_assert(sizeof(GameComponentTransformView) == 0x20, "component transform view size must match observed accesses");
_Static_assert(offsetof(GameComponentBasisView, halfword_10) == 0x10, "basis halfword offset must match observed loads");
_Static_assert(offsetof(GameComponentBasisView, words_14_1c) == 0x14, "basis word offsets must match observed loads");
_Static_assert(sizeof(GameComponentBasisView) == 0x20, "basis view size must match observed accesses");

typedef void (*GameComponentVectorTransform)(
    void *context,
    const GameComponentBasisView *basis,
    const int32_t input[3],
    int32_t output[3]
);

typedef struct GameComponentTransformOps {
    void *context;
    GameComponentVectorTransform transform_vector;
} GameComponentTransformOps;

int32_t game_component_transform_set_translation(
    GameComponentTransformView *component,
    const GameComponentBasisView *basis,
    const GameComponentBasisView *global_basis,
    const int32_t translation[3],
    const GameComponentTransformOps *ops
);

#endif