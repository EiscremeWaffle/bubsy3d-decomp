#include "level_records.h"
#include "queue_update.h"

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

extern int32_t func_80054B20(
    const ComponentTransformView *component,
    const ComponentBasisView *basis,
    int32_t output[3]
);
extern int32_t func_80054C7C(
    ComponentTransformView *component,
    const ComponentBasisView *basis,
    const int32_t input[3]
);

int32_t func_8003ABE8(void) {
    MovePreparationActor *actor;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register ComponentTransformView *transform __asm__("$16");
    register ComponentTransformView *target __asm__("$4");
    register const ComponentBasisView *basis __asm__("$5");
    register int32_t y __asm__("$2");
#else
    ComponentTransformView *transform;
    ComponentTransformView *target;
    const ComponentBasisView *basis;
    int32_t y;
#endif
    int32_t translation[3];
    int32_t offset;

    if (g_unknown_800BE0C8 != 0) {
        actor = func_80028764(0);
        if (actor != 0) {
            transform = actor->component_1C->transform_14;
            func_80054B20(transform, 0, translation);
            target = transform;
            basis = 0;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            /* Preserve the original argument and load scheduling. */
            __asm__ volatile("" : "=r"(target), "=r"(basis) : "0"(target), "1"(basis));
#endif
            y = translation[1];
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            __asm__ volatile("" : "=r"(y) : "0"(y));
#endif
            offset = g_unknown_800BE0D4;
            g_unknown_800BE0D4 = 0;
            translation[1] = (int32_t)((uint32_t)y - (uint32_t)offset);
            func_80054C7C(target, basis, translation);
        }
    }
    return 1;
}
