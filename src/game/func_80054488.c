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

int32_t func_80054488(const ComponentBasisView *input, ComponentBasisView *output) {
    register const uint16_t *input_row COMPONENT_REGISTER("$4") = input->matrix;
    register uint16_t *output_row COMPONENT_REGISTER("$5") = output->matrix;
    register int32_t row COMPONENT_REGISTER("$8") = 0;

    for (; row < 3; row++) {
        register int32_t column COMPONENT_REGISTER("$7") = 0;
        register uint16_t *output_cursor COMPONENT_REGISTER("$6") = output_row;
        register const uint16_t *input_cursor COMPONENT_REGISTER("$3") = input_row;

        for (; column < 3; column++) {
            *output_cursor++ = *input_cursor++;
        }
        input_row += 3;
        output_row += 3;
    }
    return 0;
}

#undef COMPONENT_REGISTER