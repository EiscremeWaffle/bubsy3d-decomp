/* Behavioral candidate; not yet byte-matched to the original routine. */
#include "packed_channel_copy.h"
#include "sequence_stream.h"

static __inline__ int32_t stream_signed_byte(uint8_t value) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    return (int8_t)value;
#else
    return packed_channel_signed_byte(value);
#endif
}

static __inline__ void stream_store_u16(uint8_t *output, uint32_t value) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    *(uint16_t *)output = (uint16_t)value;
#else
    packed_channel_store_u16(output, value);
#endif
}

static __inline__ void stream_store_u32(uint8_t *output, uint32_t value) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    *(uint32_t *)output = value;
#else
    packed_channel_store_u32(output, value);
#endif
}

static __inline__ uint32_t stream_load_u32(const uint8_t *input) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    return *(const uint32_t *)input;
#else
    return packed_channel_load_u32(input);
#endif
}

int32_t func_800102BC(
    uint8_t *output, const uint8_t *input, void *metadata_pointer, const uint32_t *lookup
) {
    uint8_t *metadata = metadata_pointer;
    const uint8_t *end = input + 0x200;
    uint32_t count = (uint32_t)(int32_t)g_sequence_stream_lookup_count - 1;

    do {
        int32_t z = stream_signed_byte(input[2]);
        int32_t x = stream_signed_byte(input[0]);
        int32_t y = stream_signed_byte(input[1]);
        uint32_t index, key, remaining;
        const SequenceStreamLookupRecord *record;

        stream_store_u32(output + 4, (uint32_t)y << 5);
        index = input[3] & 0x7Fu;
        stream_store_u16(output, (uint32_t)x << 5);
        key = stream_load_u32((const uint8_t *)lookup + index * 4) & 0xFDFFFFFFu;
        stream_store_u16(output + 2, (uint32_t)z << 5);
        remaining = count;
        record = g_sequence_stream_lookup_records;
        for (;;) {
            uint32_t candidate = *(const volatile uint32_t *)&record->value_10;
            if (remaining == 0) {
                stream_store_u32(metadata + 4, key);
                break;
            }
            --remaining;
            if (candidate == key) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                stream_store_u32(metadata + 4, (uint32_t)(uintptr_t)record);
#else
                /* Metadata contains PS1 addresses, not host pointers. */
                stream_store_u32(metadata + 4, 0x801D7A34u
                    + (uint32_t)((uintptr_t)record - (uintptr_t)g_sequence_stream_lookup_records));
#endif
                break;
            }
            ++record;
        }
        metadata += 16;
        input += 4;
        output += 8;
    } while (input != end);
    return 0;
}
