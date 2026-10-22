#include <stdint.h>

extern int32_t func_80076C9C(const void *rotation_matrix);
extern int32_t func_80076D2C(const void *translation_vector);

void func_80072298(const void *transform) {
    register const void *transform_value __asm__("$16") = transform;

    func_80076C9C(transform_value);
    func_80076D2C(transform_value);
}