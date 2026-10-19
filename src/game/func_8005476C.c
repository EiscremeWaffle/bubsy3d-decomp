#include <stdint.h>

typedef struct ComponentBasisView {
    uint16_t matrix[9];
    uint16_t padding;
    int32_t translation[3];
} ComponentBasisView;

extern int32_t func_800546F4(const ComponentBasisView *input, ComponentBasisView *output);

int32_t func_8005476C(const ComponentBasisView *input, ComponentBasisView *output) {
    func_800546F4(input, output);
    output->translation[0] = input->translation[0];
    output->translation[1] = (int32_t)(0u - (uint32_t)input->translation[1]);
    output->translation[2] = input->translation[2];
    return 0;
}