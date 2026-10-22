#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
__asm__(
    ".text\n"
    ".align 2\n"
    ".globl func_8004ECFC\n"
    ".type func_8004ECFC,@function\n"
    ".ent func_8004ECFC\n"
    ".set noreorder\n"
    "func_8004ECFC:\n"
    ".word 0x27BDFFE8\n"
    ".word 0xAFB00010\n"
    ".word 0x00808021\n"
    ".word 0x2E020003\n"
    ".word 0x10400014\n"
    ".word 0xAFBF0014\n"
    ".word 0x00102900\n"
    ".word 0x3C028019\n"
    ".word 0x2442CA0C\n"
    ".word 0x3C018019\n"
    ".word 0x00250821\n"
    ".word 0xA020CA18\n"
    ".word 0x3C018019\n"
    ".word 0x00250821\n"
    ".word 0xA020CA19\n"
    ".word 0x3C018019\n"
    ".word 0x00250821\n"
    ".word 0xA020CA1A\n"
    ".word 0x0C01C9E6\n"
    ".word 0x00A22821\n"
    ".word 0x3C01800C\n"
    ".word 0x00300821\n"
    ".word 0xA020E2F4\n"
    ".word 0x08013B59\n"
    ".word 0x00001021\n"
    ".word 0x34020001\n"
    ".word 0x8FBF0014\n"
    ".word 0x8FB00010\n"
    ".word 0x27BD0018\n"
    ".word 0x03E00008\n"
    ".word 0x00000000\n"
    ".set reorder\n"
    ".end func_8004ECFC\n"
    ".size func_8004ECFC,.-func_8004ECFC\n"
);
#else

typedef struct CameraTransformRecord {
    int32_t position[3];
    uint8_t rotation[3];
    uint8_t padding;
} CameraTransformRecord;

extern int32_t func_80072798(uint32_t camera_index, CameraTransformRecord *record);

int32_t func_8004ECFC(uint32_t camera_index) {
    volatile CameraTransformRecord *record;
    volatile uint8_t *status = (volatile uint8_t *)0x800AE2F4u;

    if (camera_index >= 3) {
        return 1;
    }

    record = (volatile CameraTransformRecord *)(0x8018CA0Cu + camera_index * sizeof(CameraTransformRecord));
    record->rotation[0] = 0;
    record->rotation[1] = 0;
    record->rotation[2] = 0;
    func_80072798(camera_index, (CameraTransformRecord *)record);
    status[camera_index] = 0;
    return 0;
}

#endif