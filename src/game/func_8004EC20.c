#include "camera_slots.h"

int32_t func_8004EC20(
    uint32_t camera_index,
    const uint8_t rotation[3],
    const int32_t position[3]
) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t mode __asm__("$4");
    register int32_t result __asm__("$2");
#else
    int32_t mode;
    int32_t result;
#endif

    if (camera_index >= 3) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : : : "memory");
#endif
        result = 1;
        goto done;
    }

    g_camera_transforms[camera_index].position[0] = position[0];
    g_camera_transforms[camera_index].position[1] = position[1];
    g_camera_transforms[camera_index].position[2] = position[2];
    g_camera_transforms[camera_index].rotation[0] = rotation[0];
    g_camera_transforms[camera_index].rotation[1] = rotation[1];
    g_camera_transforms[camera_index].rotation[2] = rotation[2];
    func_80072798(camera_index, &g_camera_transforms[camera_index]);
    mode = 1;
    if (g_camera_mode == 0) {
        mode = 0;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(mode) : "0"(mode));
#endif
    }
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(mode) : "0"(mode));
#endif
    func_80072D14(mode);
    g_camera_slot_status[camera_index] = 1;
    result = 0;
done:
    return result;
}
