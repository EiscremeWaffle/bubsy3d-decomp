#include "grounded_transform.h"
#include "component_rotation.h"
#include "queue_update.h"

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

typedef struct GroundedTransformScratch {
    GroundedTransformVector translation;
    uint8_t reserved[20];
} GroundedTransformScratch;

extern int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t *output
);
extern int32_t func_80054C7C(
    ComponentTransformView *component, const ComponentBasisView *basis, const int32_t *input
);
extern int32_t func_80053D00(const int32_t *input, int32_t distance, int32_t *output);
extern int32_t func_800516F8(void **output);

void func_8002E858(
    const GroundedTransformVector *direction, int32_t distance,
    uint32_t height, GroundedTransformVector *output
) {
    GroundedTransformScratch scratch;

    func_80054B20(g_unknown_800BE528, 0, scratch.translation.values);
    func_80053D00(direction->values, distance, output->values);
    output->coordinates.x = scratch.translation.coordinates.x + output->coordinates.x;
    output->coordinates.y = height + output->coordinates.y;
    output->coordinates.z = scratch.translation.coordinates.z + output->coordinates.z;
}

void func_8002E954(GroundedTransformVector *translation, uint32_t offset) {
    translation->coordinates.y = translation->coordinates.y + offset;
    func_80054C7C(g_unknown_800BE528, 0, translation->values);
    translation->coordinates.y = translation->coordinates.y - offset;
}

void func_8002EA80(void) {
    void *component;

    if (g_unknown_800BE024 == 0) return;
    func_800516F8(&component);
    func_800550A4(component, g_unknown_800BE528);
}
