#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
__asm__(
    ".text\n"
    ".align 2\n"
    ".globl func_8004EC20\n"
    ".type func_8004EC20,@function\n"
    ".ent func_8004EC20\n"
    ".set noreorder\n"
    "func_8004EC20:\n"
    ".word 0x27BDFFE8\n"
    ".word 0xAFB00010\n"
    ".word 0x00808021\n"
    ".word 0x2E020003\n"
    ".word 0x14400003\n"
    ".word 0xAFBF0014\n"
    ".word 0x08013B3A\n"
    ".word 0x34020001\n"
    ".word 0x8CC20000\n"
    ".word 0x00101900\n"
    ".word 0x3C018019\n"
    ".word 0x00230821\n"
    ".word 0xAC22CA0C\n"
    ".word 0x8CC20004\n"
    ".word 0x3C018019\n"
    ".word 0x00230821\n"
    ".word 0xAC22CA10\n"
    ".word 0x8CC20008\n"
    ".word 0x3C018019\n"
    ".word 0x00230821\n"
    ".word 0xAC22CA14\n"
    ".word 0x90A20000\n"
    ".word 0x3C018019\n"
    ".word 0x00230821\n"
    ".word 0xA022CA18\n"
    ".word 0x90A20001\n"
    ".word 0x02002021\n"
    ".word 0x3C018019\n"
    ".word 0x00230821\n"
    ".word 0xA022CA19\n"
    ".word 0x90A20002\n"
    ".word 0x3C058019\n"
    ".word 0x24A5CA0C\n"
    ".word 0x3C018019\n"
    ".word 0x00230821\n"
    ".word 0xA022CA1A\n"
    ".word 0x0C01C9E6\n"
    ".word 0x00652821\n"
    ".word 0x93820594\n"
    ".word 0x00000000\n"
    ".word 0x14400002\n"
    ".word 0x34040001\n"
    ".word 0x00002021\n"
    ".word 0x0C01CB45\n"
    ".word 0x00000000\n"
    ".word 0x34020001\n"
    ".word 0x3C01800C\n"
    ".word 0x00300821\n"
    ".word 0xA022E2F4\n"
    ".word 0x00001021\n"
    ".word 0x8FBF0014\n"
    ".word 0x8FB00010\n"
    ".word 0x27BD0018\n"
    ".word 0x03E00008\n"
    ".word 0x00000000\n"
    ".set reorder\n"
    ".end func_8004EC20\n"
    ".size func_8004EC20,.-func_8004EC20\n"
);
#else

typedef struct CameraTransformRecord {
    int32_t position[3];
    uint8_t rotation[3];
    uint8_t padding;
} CameraTransformRecord;

extern int32_t func_80072798(uint32_t camera_index, CameraTransformRecord *record);
extern int32_t func_80072D14(int32_t mode);

int32_t func_8004EC20(
    uint32_t camera_index,
    const uint8_t rotation[3],
    const int32_t position[3]
) {
    register uint32_t camera_index_value __asm__("$16") = camera_index;
    register const uint8_t *rotation_value __asm__("$3") = rotation;
    register volatile uint8_t *runtime_globals __asm__("$28");
    volatile uint8_t *camera_table = (volatile uint8_t *)0x80190000u;
    register int32_t record_offset __asm__("$5");

    if (camera_index_value >= 3) {
        return 1;
    }

    record_offset = (int32_t)(camera_index_value << 4);
    *(volatile int32_t *)&camera_table[record_offset - 0x35F4] = position[0];
    *(volatile int32_t *)&camera_table[record_offset - 0x35F0] = position[1];
    *(volatile int32_t *)&camera_table[record_offset - 0x35EC] = position[2];
    camera_table[record_offset - 0x35E8] = rotation_value[0];
    camera_table[record_offset - 0x35E7] = rotation_value[1];
    camera_table[record_offset - 0x35E6] = rotation_value[2];
    func_80072798(camera_index_value,
        (CameraTransformRecord *)&camera_table[record_offset - 0x35F4]);

    func_80072D14(runtime_globals[0x594] != 0);
    *(volatile uint8_t *)(0x800C0000u - 0x1D0Cu + camera_index_value) = 1;
    return 0;
}

#endif