#include "mode_camera_helpers.h"

int32_t func_8004EDEC(uint32_t camera_index, const int32_t position[3]) {
    if (camera_index >= 3) {
        return 1;
    }

    g_camera_transforms[camera_index].position[0] = position[0];
    g_camera_transforms[camera_index].position[1] = position[1];
    g_camera_transforms[camera_index].position[2] = position[2];
    func_80072798(camera_index, &g_camera_transforms[camera_index]);
    return 0;
}
