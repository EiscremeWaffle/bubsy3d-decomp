#include "component_bounds.h"

typedef union ExtentVector {
    int32_t words[4];
    struct {
        int32_t x, y, z, pad;
    } component;
} ExtentVector;

typedef struct ExtentScratch {
    ComponentBounds bounds;
    ExtentVector difference;
} ExtentScratch;

extern int32_t func_80010000(const int32_t *upper, const int32_t *lower, int32_t *difference);

int32_t func_80059274(void *object, int32_t *output) {
    ExtentScratch scratch;

    func_80058C74(object, &scratch.bounds);
    func_80010000(scratch.bounds.signed_words + 4, scratch.bounds.signed_words, scratch.difference.words);
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    *output = ({
        register int32_t maximum = scratch.difference.component.y < scratch.difference.component.z
            ? scratch.difference.component.z : scratch.difference.component.y;
        scratch.difference.component.x < maximum ? maximum : scratch.difference.component.x;
    });
#else
    {
        int32_t maximum = scratch.difference.component.y < scratch.difference.component.z
            ? scratch.difference.component.z : scratch.difference.component.y;
        *output = scratch.difference.component.x < maximum ? maximum : scratch.difference.component.x;
    }
#endif
    return 0;
}
