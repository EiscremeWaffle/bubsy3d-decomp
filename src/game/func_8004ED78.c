#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
__asm__(
    ".text\n"
    ".align 2\n"
    ".globl func_8004ED78\n"
    ".type func_8004ED78,@function\n"
    ".ent func_8004ED78\n"
    ".set noreorder\n"
    "func_8004ED78:\n"
    ".word 0x27BDFFE8\n"
    ".word 0xAFBF0010\n"
    ".word 0x2C820003\n"
    ".word 0x10400014\n"
    ".word 0x00A01821\n"
    ".word 0x90620000\n"
    ".word 0x00042900\n"
    ".word 0x3C018019\n"
    ".word 0x00250821\n"
    ".word 0xA022CA18\n"
    ".word 0x90620001\n"
    ".word 0x3C018019\n"
    ".word 0x00250821\n"
    ".word 0xA022CA19\n"
    ".word 0x90620002\n"
    ".word 0x3C018019\n"
    ".word 0x00250821\n"
    ".word 0xA022CA1A\n"
    ".word 0x3C028019\n"
    ".word 0x2442CA0C\n"
    ".word 0x0C01C9E6\n"
    ".word 0x00A22821\n"
    ".word 0x08013B77\n"
    ".word 0x00001021\n"
    ".word 0x34020001\n"
    ".word 0x8FBF0010\n"
    ".word 0x27BD0018\n"
    ".word 0x03E00008\n"
    ".word 0x00000000\n"
    ".set reorder\n"
    ".end func_8004ED78\n"
    ".size func_8004ED78,.-func_8004ED78\n"
);
#else
typedef struct CameraTransformRecord {
    int32_t position[3];
    uint8_t rotation[3];
    uint8_t padding;
} CameraTransformRecord;

extern int32_t func_80072798(uint32_t camera_index, CameraTransformRecord *record);

int32_t func_8004ED78(uint32_t camera_index, const uint8_t rotation[3]) {
    register uint32_t camera_index_value __asm__("$16") = camera_index;
    register const uint8_t *rotation_value __asm__("$5") = rotation;
    volatile CameraTransformRecord *record;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(camera_index_value) : "0"(camera_index_value));
    __asm__ volatile("" : "=r"(rotation_value) : "0"(rotation_value));
#endif

    if (camera_index_value >= 3) {
        return 1;
    }

    record = (volatile CameraTransformRecord *)(0x8018CA0Cu + camera_index_value * sizeof(CameraTransformRecord));
    record->rotation[0] = rotation_value[0];
    record->rotation[1] = rotation_value[1];
    record->rotation[2] = rotation_value[2];
    func_80072798(camera_index_value, (CameraTransformRecord *)record);
    return 0;
}

#endif