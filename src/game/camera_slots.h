#ifndef BUBSY3D_CAMERA_SLOTS_H
#define BUBSY3D_CAMERA_SLOTS_H

#include <stdint.h>

typedef struct CameraTransformRecord {
    int32_t position[3];
    uint8_t rotation[3];
    uint8_t padding;
} CameraTransformRecord;

extern CameraTransformRecord g_camera_transforms[3];
extern uint8_t g_camera_slot_status[3];
extern uint8_t g_camera_mode;
extern int32_t func_80072798(uint32_t camera_index, CameraTransformRecord *record);
extern int32_t func_80072D14(int32_t mode);

int32_t func_8004EC20(uint32_t camera_index, const uint8_t rotation[3], const int32_t position[3]);
int32_t func_8004ECFC(uint32_t camera_index);
int32_t func_8004ED78(uint32_t camera_index, const uint8_t rotation[3]);

#endif
