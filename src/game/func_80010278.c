#include "packed_channel_copy.h"

int32_t func_80010278(uint8_t *output, const uint8_t *input, uint32_t count) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t byte_count __asm__("$9") = count << 2;
    register const uint8_t *end __asm__("$8");
    register int32_t third __asm__("$9");
    register int32_t first __asm__("$10");
    register int32_t second __asm__("$11");
    register uint32_t shift __asm__("$13") = 5;
    __asm__("" : "=r"(byte_count), "=r"(shift) : "0"(byte_count), "1"(shift));
#else
    uint32_t byte_count = count << 2;
    const uint8_t *end;
    int32_t third;
    int32_t first;
    int32_t second;
#endif

    end = (const uint8_t *)(byte_count + (uintptr_t)input);
    do {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        third = *(const int8_t *)(input + 2);
        __asm__ volatile("" : "=r"(third), "=r"(input) : "0"(third), "1"(input) : "memory");
        first = *(const int8_t *)input;
        __asm__ volatile("" : "=r"(first), "=r"(input) : "0"(first), "1"(input) : "memory");
        second = *(const int8_t *)(input + 1);
        __asm__ volatile("" : "=r"(second), "=r"(input) : "0"(second), "1"(input) : "memory");
        third = (int32_t)((uint32_t)third << shift);
        first = (int32_t)((uint32_t)first << shift);
        second = (int32_t)((uint32_t)second << shift);
        *(volatile int16_t *)output = (int16_t)first;
        *(volatile int16_t *)(output + 2) = (int16_t)third;
        *(volatile int32_t *)(output + 4) = second;
        __asm__ volatile("" : "=r"(input) : "0"(input) : "memory");
#else
        third = packed_channel_signed_byte(input[2]) * 32;
        first = packed_channel_signed_byte(input[0]) * 32;
        second = packed_channel_signed_byte(input[1]) * 32;
        packed_channel_store(output, first, third, second);
#endif
        input += 4;
        output += 8;
    } while (end != input);
    return 0;
}
