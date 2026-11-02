#include "camera_slots.h"

int32_t func_8004ED78(uint32_t camera_index, const uint8_t rotation[3]) {
    if (camera_index >= 3) {
        return 1;
    }

    g_camera_transforms[camera_index].rotation[0] = rotation[0];
    g_camera_transforms[camera_index].rotation[1] = rotation[1];
    g_camera_transforms[camera_index].rotation[2] = rotation[2];
    func_80072798(camera_index, &g_camera_transforms[camera_index]);
    return 0;
}
