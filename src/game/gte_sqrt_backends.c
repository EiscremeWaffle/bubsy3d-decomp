/* Readable GTE-assisted candidates; not yet byte-matched to the originals. */
#include "sqrt_backend.h"

static __inline__ uint32_t leading_sign_count(int32_t value) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t count __asm__("$2");
    __asm__ volatile(
        "mtc2 %1,$30\n\t"
        "nop\n\t"
        "nop\n\t"
        "mfc2 %0,$31"
        : "=r"(count) : "r"(value));
    return count;
#else
    uint32_t bits = (uint32_t)value;
    uint32_t count = 0;

    if (bits & 0x80000000u) {
        bits = ~bits;
    }
    while ((bits & 0x80000000u) == 0 && count < 32) {
        bits <<= 1;
        count++;
    }
    return count;
#endif
}

static __inline__ int32_t signed_word(uint32_t bits) {
    return bits & 0x80000000u ? -1 - (int32_t)~bits : (int32_t)bits;
}

static __inline__ int16_t lookup_sample(int32_t value, uint32_t even) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t left_shift __asm__("$11") = (int32_t)even - 24;
    register int32_t normalized __asm__("$12");
#else
    int32_t left_shift = (int32_t)even - 24;
    int32_t normalized;
#endif

    if (left_shift >= 0) {
        normalized = signed_word((uint32_t)value << left_shift);
    } else {
        normalized = value >> (24 - even);
    }
    return g_sqrt_lookup_domain[normalized + 256];
}

int32_t func_800764D0(int32_t value) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t leading __asm__("$2") = leading_sign_count(value);
    register uint32_t even __asm__("$10");
    register uint32_t parity __asm__("$8");
    register int32_t shift __asm__("$9");
    int16_t sample;
#else
    uint32_t leading = leading_sign_count(value);
    uint32_t even;
    int16_t sample;
    int32_t shift;
#endif

    if (leading != 32) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        parity = leading & 1u;
        __asm__ volatile("" : : "r"(parity));
        even = ~1u;
        even &= leading;
        shift = (31 - (int32_t)even) >> 1;
        __asm__ volatile("" : : "r"(shift));
#else
        even = leading & ~1u;
        shift = (31 - (int32_t)even) >> 1;
#endif
        sample = lookup_sample(value, even);
        return (int32_t)(((uint32_t)sample << shift) >> 12);
    }
    return 0;
}

int32_t func_80076554(int32_t value) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t leading __asm__("$2") = leading_sign_count(value);
    register uint32_t even __asm__("$10");
    register uint32_t parity __asm__("$8");
    register int32_t shift __asm__("$9");
    int16_t sample;
#else
    uint32_t leading = leading_sign_count(value);
    uint32_t even;
    int16_t sample;
    int32_t shift;
#endif

    if (leading != 32) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        parity = leading & 1u;
        __asm__ volatile("" : : "r"(parity));
        even = ~1u;
        even &= leading;
        shift = (19 - (int32_t)even) >> 1;
        __asm__ volatile("" : : "r"(shift));
#else
        even = leading & ~1u;
        shift = (19 - (int32_t)even) >> 1;
#endif
        sample = lookup_sample(value, even);
        if (shift < 0) {
            return signed_word((uint32_t)sample >> -shift);
        }
        return signed_word((uint32_t)sample << shift);
    }
    return 0;
}
