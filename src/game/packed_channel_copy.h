#ifndef BUBSY3D_PACKED_CHANNEL_COPY_H
#define BUBSY3D_PACKED_CHANNEL_COPY_H

#include <stdint.h>

/* Both require word-aligned output and a nonzero count. The first also requires
 * halfword-aligned input. Expanded eight-byte records are written forward. */
int32_t func_80010244(uint8_t *output, const uint8_t *input, uint32_t count);
int32_t func_80010278(uint8_t *output, const uint8_t *input, uint32_t count);

#ifndef BUBSY3D_MATCH_ORIGINAL_L0
/* Host byte accesses preserve the PS1's little-endian layout and aliasing. */
static inline int32_t packed_channel_signed_byte(uint8_t value) {
    return value < 0x80 ? value : (int32_t)value - 0x100;
}

static inline int32_t packed_channel_signed_halfword(const uint8_t *input) {
    uint32_t value = (uint32_t)input[0] | ((uint32_t)input[1] << 8);
    return value < 0x8000 ? (int32_t)value : (int32_t)value - 0x10000;
}

static inline uint32_t packed_channel_load_u32(const uint8_t *input) {
    return (uint32_t)input[0] | ((uint32_t)input[1] << 8)
         | ((uint32_t)input[2] << 16) | ((uint32_t)input[3] << 24);
}

static inline void packed_channel_store_u16(uint8_t *output, uint32_t value) {
    output[0] = (uint8_t)value;
    output[1] = (uint8_t)(value >> 8);
}

static inline void packed_channel_store_u32(uint8_t *output, uint32_t value) {
    output[0] = (uint8_t)value;
    output[1] = (uint8_t)(value >> 8);
    output[2] = (uint8_t)(value >> 16);
    output[3] = (uint8_t)(value >> 24);
}

static inline void packed_channel_store(uint8_t *output, int32_t first, int32_t third, int32_t second) {
    packed_channel_store_u16(output, (uint32_t)first);
    packed_channel_store_u16(output + 2, (uint32_t)third);
    packed_channel_store_u32(output + 4, (uint32_t)second);
}
#endif

#endif
