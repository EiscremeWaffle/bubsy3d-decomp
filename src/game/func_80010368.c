/* Behavioral candidate; not yet byte-matched to the original routine. */
#include "sequence_stream.h"

int32_t func_80010368(SequenceStreamArguments *arguments) {
    uint32_t *output = arguments->descriptor_10->surface_00;
    const uint32_t *end = output + 256;
    uint32_t x = arguments->remainder_scaled_08;
    uint32_t y = arguments->quotient_scaled_0C;
    uint32_t offset = arguments->offset_1C;
    const int8_t *input = (const int8_t *)((uintptr_t)arguments->table_14 + offset);
    uint32_t column = 0;

    for (;;) {
        int32_t height = *(const volatile int8_t *)input;
        if (output == end) {
            break;
        }
        ++input;
        *output++ = ((0u - ((uint32_t)height << 5)) << 16) + x;
        *output++ = y;
        x += 0x100u;
        if (column++ == 8) {
            column = 0;
            x = arguments->remainder_scaled_08;
            y += 0x100u;
            offset += arguments->stride_18;
            input = (const int8_t *)((uintptr_t)arguments->table_14 + offset);
        }
    }
    return 0;
}
