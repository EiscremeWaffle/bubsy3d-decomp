#include <stdint.h>

typedef struct ComponentBasisView {
    uint16_t matrix[9];
    uint16_t padding;
    int32_t translation[3];
} ComponentBasisView;

typedef struct ComponentTransformView {
    uint16_t matrix[9];
    uint16_t padding;
    int32_t translation[3];
} ComponentTransformView;

typedef struct ComponentTransformVectorScratch {
    int32_t values[3];
    int32_t padding;
} ComponentTransformVectorScratch;

typedef struct ComponentTransformScratch {
    ComponentTransformVectorScratch direction;
    ComponentTransformVectorScratch normalized_direction;
    ComponentTransformVectorScratch cross;
    ComponentTransformVectorScratch axis;
    ComponentTransformVectorScratch vertical;
    ComponentTransformVectorScratch component_translation;
    ComponentTransformVectorScratch basis_translation;
    uint16_t matrix[9];
    uint16_t matrix_padding;
    volatile int32_t reserved[2];
} ComponentTransformScratch;

extern int32_t func_80010000(
    const int32_t left[3],
    const int32_t right[3],
    int32_t output[3]
);
extern int32_t func_80010128(
    const int32_t left[3],
    const int32_t right[3],
    int32_t output[3]
);
extern int32_t func_8005395C(const int32_t input[3], int32_t output[3]);
extern int32_t func_80053D68(const int32_t input[3], int32_t output[3]);
extern int32_t func_80054488(
    const ComponentBasisView *input,
    ComponentBasisView *output
);

int32_t func_800550A4(
    ComponentTransformView *component,
    const ComponentBasisView *basis
) {
    ComponentTransformScratch scratch;
    int32_t status;

    if (component == 0 || basis == 0) {
        return 0x18;
    }

    scratch.reserved[0] = 0;
    scratch.reserved[1] = 0;
    scratch.reserved[0] = 0;
    scratch.component_translation.values[0] = component->translation[0];
    scratch.component_translation.values[1] = component->translation[1];
    scratch.component_translation.values[2] = component->translation[2];
    scratch.basis_translation.values[0] = basis->translation[0];
    scratch.basis_translation.values[1] = basis->translation[1];
    scratch.basis_translation.values[2] = basis->translation[2];
    scratch.vertical.values[0] = 0;
    scratch.vertical.values[1] = -0x1000;
    scratch.vertical.values[2] = 0;

    status = func_80010000(scratch.basis_translation.values,
        scratch.component_translation.values, scratch.direction.values);
    if (status != 0) {
        return status;
    }
    if (scratch.direction.values[0] == 0 && scratch.direction.values[1] == 0 &&
        scratch.direction.values[2] == 0) {
        return 0;
    }

    func_8005395C(scratch.direction.values, scratch.normalized_direction.values);
    func_80053D68(scratch.direction.values, scratch.direction.values);
    func_80053D68(scratch.vertical.values, scratch.vertical.values);
    if (scratch.direction.values[0] == 0 && scratch.direction.values[2] == 0) {
        scratch.direction.values[2] = 1;
        scratch.direction.values[0] = 1;
    }

    status = func_80010128(scratch.direction.values, scratch.vertical.values,
        scratch.cross.values);
    if (status != 0) {
        return status;
    }
    func_8005395C(scratch.cross.values, scratch.axis.values);
    scratch.matrix[0] = (uint16_t)scratch.axis.values[0];
    scratch.matrix[3] = (uint16_t)scratch.axis.values[1];
    scratch.matrix[6] = (uint16_t)scratch.axis.values[2];
    func_80053D68(scratch.axis.values, scratch.axis.values);
    status = func_80010128(scratch.direction.values, scratch.axis.values,
        scratch.cross.values);
    if (status != 0) {
        return status;
    }
    func_8005395C(scratch.cross.values, scratch.axis.values);

    scratch.matrix[1] = (uint16_t)scratch.axis.values[0];
    scratch.matrix[4] = (uint16_t)scratch.axis.values[1];
    scratch.matrix[7] = (uint16_t)scratch.axis.values[2];
    scratch.matrix[2] = (uint16_t)scratch.normalized_direction.values[0];
    scratch.matrix[5] = (uint16_t)scratch.normalized_direction.values[1];
    scratch.matrix[8] = (uint16_t)scratch.normalized_direction.values[2];
    func_80054488((const ComponentBasisView *)scratch.matrix,
        (ComponentBasisView *)component);
    return 0;
}