#include "secondary_position.h"

typedef struct SecondaryPositionScratch {
    ComponentBounds bounds;
    int32_t size[4];
    int32_t half_size[4];
    int32_t center[4];
    ComponentRotationMatrix *component;
    uint32_t padding;
    ComponentRotationMatrix normalized_basis;
} SecondaryPositionScratch;

extern int32_t func_80010000(const int32_t *left, const int32_t *right, int32_t *output);
extern int32_t func_80053EA0(const int32_t *input, int32_t *output);
extern int32_t func_80010038(const int32_t *left, const int32_t *right, int32_t *output);
extern int32_t func_8005476C(const ComponentRotationMatrix *basis, ComponentRotationMatrix *output);
extern int32_t func_80064528(const int32_t *input, const ComponentRotationMatrix *basis, int32_t *output);

int32_t func_80058E7C(void *object, void *basis, int32_t output[3]) {
    SecondaryPositionScratch scratch;

    (void)basis;
    func_80058C74(&((SecondaryPositionOwner *)object)->records, &scratch.bounds);
    func_80010000(scratch.bounds.signed_words + 4, scratch.bounds.signed_words, scratch.size);
    func_80053EA0(scratch.size, scratch.half_size);
    func_80010038(scratch.bounds.signed_words, scratch.half_size, scratch.center);
#ifndef BUBSY3D_MATCH_ORIGINAL_L0
    /* The identity transform copies the unused fourth word. */
    scratch.center[3] = 0;
#endif
    scratch.component = ((SecondaryPositionOwner *)object)->position.component_14;
    func_8005476C(scratch.component, &scratch.normalized_basis);
    func_80064528(scratch.center, &scratch.normalized_basis, output);
    return 0;
}
