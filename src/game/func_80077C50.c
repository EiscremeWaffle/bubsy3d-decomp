#include "rotation_generator.h"

#define ROTATION_TABLE_ENTRY(index) (((const volatile uint32_t *)g_rotation_trig_table)[index])

#define Q12_PRODUCT(left, right) ((int32_t)((uint32_t)(left) * (uint32_t)(right)) >> 12)
#define Q12_NEGATIVE_PRODUCT(left, right) ((int32_t)(0u - (uint32_t)(left) * (uint32_t)(right)) >> 12)

ComponentRotationMatrix *func_80077C50(
    const uint16_t angles[3], ComponentRotationMatrix *output) {
    /* Preserve the original packed 32-bit lookup for both halfwords. */
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t angle __asm__("$15");
    register int32_t angle_z __asm__("$14");
    register int32_t sine_x __asm__("$11");
    register int32_t cosine_x __asm__("$8");
    register int32_t sine_y __asm__("$12");
    register int32_t cosine_y __asm__("$9");
    register int32_t sine_z __asm__("$13");
    register int32_t cosine_z __asm__("$10");
    register uint32_t packed __asm__("$25");
    register uint32_t index __asm__("$24");
    register int32_t x_part;
    register int32_t z_part;
    register ComponentRotationMatrix *result __asm__("$2") = output;
#else
    int32_t angle, angle_z;
    int32_t sine_x, cosine_x, sine_y, cosine_y, sine_z, cosine_z;
    int32_t x_part, z_part;
    uint32_t packed, index;
    ComponentRotationMatrix *result = output;
#endif

    angle = (int16_t)angles[0];
    if (angle < 0) {
        index = (uint32_t)-angle & 0xFFFu;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(index) : "0"(index));
#endif
        packed = ROTATION_TABLE_ENTRY(index);
        sine_x = -(int16_t)packed;
        cosine_x = (int16_t)(packed >> 16);
    } else {
        index = (uint32_t)angle & 0xFFFu;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(index) : "0"(index));
#endif
        packed = ROTATION_TABLE_ENTRY(index);
        sine_x = (int16_t)packed;
        cosine_x = (int16_t)(packed >> 16);
    }

    angle = (int16_t)angles[1];
    if (angle < 0) {
        index = (uint32_t)-angle & 0xFFFu;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(index) : "0"(index));
#endif
        packed = ROTATION_TABLE_ENTRY(index);
        sine_y = -(int16_t)packed;
        cosine_y = (int16_t)(packed >> 16);
    } else {
        index = (uint32_t)angle & 0xFFFu;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(index) : "0"(index));
#endif
        packed = ROTATION_TABLE_ENTRY(index);
        sine_y = (int16_t)packed;
        cosine_y = (int16_t)(packed >> 16);
    }
    angle_z = (int16_t)angles[2];

    result->elements[2] = (uint16_t)sine_y;
    result->elements[5] = (uint16_t)Q12_NEGATIVE_PRODUCT(cosine_y, sine_x);
    result->elements[8] = (uint16_t)Q12_PRODUCT(cosine_y, cosine_x);

    angle = angle_z;
    if (angle < 0) {
        index = (uint32_t)-angle & 0xFFFu;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(index) : "0"(index));
#endif
        packed = ROTATION_TABLE_ENTRY(index);
        sine_z = -(int16_t)packed;
        cosine_z = (int16_t)(packed >> 16);
    } else {
        index = (uint32_t)angle & 0xFFFu;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(index) : "0"(index));
#endif
        packed = ROTATION_TABLE_ENTRY(index);
        sine_z = (int16_t)packed;
        cosine_z = (int16_t)(packed >> 16);
    }

    result->elements[0] = (uint16_t)Q12_PRODUCT(cosine_z, cosine_y);
    result->elements[1] = (uint16_t)Q12_NEGATIVE_PRODUCT(sine_z, cosine_y);
    x_part = Q12_PRODUCT(cosine_z, -sine_y);
    result->elements[3] = (uint16_t)(
        Q12_PRODUCT(sine_z, cosine_x) - Q12_PRODUCT(x_part, sine_x));
    result->elements[6] = (uint16_t)(
        Q12_PRODUCT(sine_z, sine_x) + Q12_PRODUCT(x_part, cosine_x));
    z_part = Q12_PRODUCT(sine_z, -sine_y);
    result->elements[4] = (uint16_t)(
        Q12_PRODUCT(cosine_z, cosine_x) + Q12_PRODUCT(z_part, sine_x));
    result->elements[7] = (uint16_t)(
        Q12_PRODUCT(cosine_z, sine_x) - Q12_PRODUCT(z_part, cosine_x));
    return result;
}

#undef Q12_PRODUCT
#undef Q12_NEGATIVE_PRODUCT
#undef ROTATION_TABLE_ENTRY
