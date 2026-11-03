#include "component_rotation.h"

extern int32_t func_80053D68(const int32_t input[3], int32_t output[3]);
extern int32_t func_80010128(const int32_t left[3], const int32_t right[3], int32_t output[3]);
extern int32_t func_8005395C(const int32_t input[3], int32_t output[3]);

int32_t func_800542A4(const ComponentRotationMatrix *input, ComponentRotationMatrix *output) {
    int32_t first[3];
    int32_t second[3];
    int32_t third[3];
    int32_t normalized_first[3];
    int32_t normalized_second[3];
    int32_t normalized_third[3];

    first[0] = (int16_t)input->elements[0];
    first[1] = (int16_t)input->elements[3];
    first[2] = (int16_t)input->elements[6];
    second[0] = (int16_t)input->elements[1];
    second[1] = (int16_t)input->elements[4];
    second[2] = (int16_t)input->elements[7];
    third[0] = (int16_t)input->elements[2];
    third[1] = (int16_t)input->elements[5];
    third[2] = (int16_t)input->elements[8];
    func_80053D68(second, second);
    func_80053D68(third, third);
    func_80010128(second, third, first);
    func_80053D68(first, first);
    func_80010128(first, second, third);
    func_8005395C(first, normalized_first);
    func_8005395C(second, normalized_second);
    func_8005395C(third, normalized_third);
    output->elements[0] = (uint16_t)normalized_first[0];
    output->elements[3] = (uint16_t)normalized_first[1];
    output->elements[6] = (uint16_t)normalized_first[2];
    output->elements[1] = (uint16_t)normalized_second[0];
    output->elements[4] = (uint16_t)normalized_second[1];
    output->elements[7] = (uint16_t)normalized_second[2];
    output->elements[2] = (uint16_t)normalized_third[0];
    output->elements[5] = (uint16_t)normalized_third[1];
    output->elements[8] = (uint16_t)normalized_third[2];
    return 0;
}
