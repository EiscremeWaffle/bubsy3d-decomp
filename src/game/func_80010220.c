#include "word_copy.h"

int32_t func_80010220(uint32_t *output, const uint32_t *input, uint32_t count) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t byte_count __asm__("$9") = count << 2;
    register const uint32_t *end __asm__("$8");
    register uint32_t word __asm__("$9");

    __asm__("" : "=r"(byte_count) : "0"(byte_count));
#else
    uint32_t byte_count = count << 2;
    const uint32_t *end;
    uint32_t word;
#endif

    end = (const uint32_t *)(byte_count + (uintptr_t)input);
    do {
        word = *input++;
        *output++ = word;
    } while (end != input);
    return 0;
}
