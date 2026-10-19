#include <stdint.h>

typedef struct ComponentBasisView {
    uint16_t matrix[9];
    uint16_t padding;
    uint32_t translation[3];
} ComponentBasisView;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
#define COMPONENT_REGISTER(name) __asm__(name)
#else
#define COMPONENT_REGISTER(name)
#endif

int32_t func_80054414(const ComponentBasisView *input, ComponentBasisView *output) {
    register int32_t row COMPONENT_REGISTER("$8") = 0;
    register uint16_t *output_base COMPONENT_REGISTER("$10") = output->matrix;
    register int32_t row_offset COMPONENT_REGISTER("$9");

    for (; row < 3; row++) {
        register int32_t column COMPONENT_REGISTER("$7") = 0;
        register const uint16_t *input_cursor COMPONENT_REGISTER("$3");
        register uint16_t *output_cursor COMPONENT_REGISTER("$6");

        row_offset = row * sizeof(uint16_t);
        output_cursor = output_base;
        input_cursor = input->matrix;
        for (; column < 3; column++) {
            *output_cursor++ = *(const uint16_t *)(row_offset + (uintptr_t)input_cursor);
            input_cursor += 3;
        }
        output_base += 3;
    }
    output->translation[0] = input->translation[0];
    output->translation[1] = input->translation[1];
    output->translation[2] = input->translation[2];
    return 0;
}

#undef COMPONENT_REGISTER