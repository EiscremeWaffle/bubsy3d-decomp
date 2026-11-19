#include "mode_camera_helpers.h"

extern void func_80077874(int32_t first, int32_t second, int32_t third);

void func_80072DF0(int32_t first, int32_t second, int32_t third) {
    func_80077874(first >> 4, second >> 4, third >> 4);
}
