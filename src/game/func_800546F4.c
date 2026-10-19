#include <stdint.h>

typedef struct ComponentBasisView {
    uint16_t matrix[9];
    uint16_t padding;
    int32_t translation[3];
} ComponentBasisView;

extern int32_t func_80054488(const ComponentBasisView *input, ComponentBasisView *output);

int32_t func_800546F4(const ComponentBasisView *input, ComponentBasisView *output) {
    if (input == 0) {
        return 0x18;
    }

    func_80054488(input, output);
    output->translation[2] = 0;
    output->translation[1] = 0;
    output->translation[0] = 0;
    output->matrix[1] = (uint16_t)(0u - output->matrix[1]);
    output->matrix[3] = (uint16_t)(0u - output->matrix[3]);
    output->matrix[5] = (uint16_t)(0u - output->matrix[5]);
    output->matrix[7] = (uint16_t)(0u - output->matrix[7]);
    return 0;
}