#include <stdint.h>

extern int32_t func_800590B4(void *first, void *second, void *basis, int32_t *distance);
extern int32_t func_80059274(void *object, int32_t *extent);

int32_t func_8005C3A4(void *first, void *second, void *basis, uint8_t *output) {
    int32_t distance, first_extent, second_extent;

    func_800590B4(first, second, basis, &distance);
    func_80059274(first, &first_extent);
    func_80059274(second, &second_extent);
    *output = (int32_t)((uint32_t)distance << 1)
        <= (int32_t)((uint32_t)first_extent + (uint32_t)second_extent);
    return 0;
}
