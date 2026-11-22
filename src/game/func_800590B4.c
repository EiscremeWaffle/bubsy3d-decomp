#include <stdint.h>

extern int32_t func_80058E7C(void *object, void *basis, int32_t *output);
extern int32_t func_80062D4C(const int32_t *left, const int32_t *right, int32_t *output);

int32_t func_800590B4(void *first, void *second, void *basis, int32_t *output) {
    int32_t first_position[4], second_position[4];

    func_80058E7C(first, basis, first_position);
    func_80058E7C(second, basis, second_position);
    func_80062D4C(first_position, second_position, output);
    return 0;
}
