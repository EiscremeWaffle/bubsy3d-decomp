#include <stdint.h>

int32_t func_80053350(int32_t value, int32_t scale, int32_t *output) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t value_negative __asm__("$9") = 0;
    register int32_t scale_negative __asm__("$8") = 0;
    register int32_t result __asm__("$2");
    register int32_t quotient __asm__("$3");
#else
    int32_t value_negative = 0, scale_negative = 0, result, quotient;
#endif

    if (scale == 0) return 7;
    if (value == 0) {
        *output = 0;
        return 0;
    }
    if (value < 0) {
        value_negative = 1;
        value = (int32_t)(0u - (uint32_t)value);
    }
    if (scale < 0) {
        scale_negative = 1;
        scale = (int32_t)(0u - (uint32_t)scale);
    }
    if (value < scale) {
        if (value > 0x07ffffff) {
            scale >>= 12;
        } else if (value > 0x007fffff) {
            value = (int32_t)((uint32_t)value << 4);
            scale >>= 8;
        } else if (value > 0x0007ffff) {
            value = (int32_t)((uint32_t)value << 8);
            scale >>= 4;
        } else {
            value = (int32_t)((uint32_t)value << 12);
        }
        result = value / scale;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(result) : "0"(result));
#endif
    } else if (value <= 0x0007ffff) {
        value = (int32_t)((uint32_t)value << 12);
        result = value / scale;
    } else {
        quotient = value / scale;
        if (quotient > 0x0007ffff) {
            if (scale_negative == 1) {
                if (value_negative != scale_negative) goto negative_overflow;
                goto positive_overflow;
            }
            if (value_negative == 0) goto positive_overflow;
negative_overflow:
            result = (int32_t)0x80000001u;
            goto store_overflow;
positive_overflow:
            result = 0x7fffffff;
store_overflow:
            *output = result;
            return 14;
        }
        result = (int32_t)((uint32_t)quotient << 12);
    }
    *output = result;
    if (value_negative != scale_negative) {
        /* Preserve the original reload before negation. */
        *output = (int32_t)(0u - (uint32_t)*(volatile int32_t *)output);
    }
    return 0;
}
