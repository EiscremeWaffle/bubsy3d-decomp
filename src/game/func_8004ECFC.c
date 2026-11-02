#include "camera_slots.h"

int32_t func_8004ECFC(uint32_t camera_index) {
    if (camera_index >= 3) {
        return 1;
    }

    g_camera_transforms[camera_index].rotation[0] = 0;
    g_camera_transforms[camera_index].rotation[1] = 0;
    g_camera_transforms[camera_index].rotation[2] = 0;
    func_80072798(camera_index, &g_camera_transforms[camera_index]);
    g_camera_slot_status[camera_index] = 0;
    return 0;
}
