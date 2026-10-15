#include <stdint.h>

extern void func_80010000(const int32_t *left, const int32_t *right, int32_t *output);
extern int32_t func_80053904(const int32_t *input, int32_t *output);

int32_t func_80062D4C(const int32_t *left, const int32_t *right, int32_t *output) {
    int32_t difference[4];

    func_80010000(left, right, difference);
    func_80053904(difference, output);
    return 0;
}
