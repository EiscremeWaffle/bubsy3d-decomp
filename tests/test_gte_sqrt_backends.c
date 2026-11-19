#include "sqrt_backend.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int16_t g_sqrt_lookup_domain[512];

int32_t func_8005359C(int32_t value, int32_t *output);
int32_t func_800535CC(int32_t value, int32_t *output);
int32_t move_vector_length_from_wrapped_squares(
    const int32_t input[3], int32_t *output) __asm__("func_80053904");

static uint32_t random_state = 0x76554u;

static uint32_t random_word(void) {
    random_state = random_state * 1664525u + 1013904223u;
    return random_state;
}

static uint32_t reference_leading_count(uint32_t bits) {
    uint32_t sign = bits >> 31;
    uint32_t count = 0;
    int bit;

    for (bit = 31; bit >= 0; bit--) {
        if (((bits >> bit) & 1u) != sign) {
            break;
        }
        count++;
    }
    return count;
}

static uint32_t reference_backend(uint32_t bits, int q12) {
    uint32_t leading = reference_leading_count(bits);
    uint32_t even, sample;
    int32_t adjustment, shift;
    int64_t value, normalized, divisor;

    if (leading == 32) {
        return 0;
    }
    even = leading & ~1u;
    adjustment = (int32_t)even - 24;
    value = bits < 0x80000000u ? (int64_t)bits : (int64_t)bits - 0x100000000LL;
    if (adjustment >= 0) {
        uint32_t shifted = (uint32_t)((uint64_t)bits << adjustment);
        normalized = shifted < 0x80000000u ? (int64_t)shifted : (int64_t)shifted - 0x100000000LL;
    } else {
        divisor = (int64_t)1 << -adjustment;
        normalized = value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor);
    }
    assert(normalized + 256 >= 0 && normalized + 256 < 512);
    sample = (uint32_t)(int32_t)g_sqrt_lookup_domain[normalized + 256];
    if (q12) {
        shift = 9 - (int32_t)(even / 2);
        return shift >= 0 ? (uint32_t)((uint64_t)sample << shift) : sample >> -shift;
    }
    return (uint32_t)((uint64_t)sample << (15 - even / 2)) >> 12;
}

static void run_case(uint32_t bits) {
    int32_t input, output[3];
    unsigned q12;

    memcpy(&input, &bits, sizeof(input));
    for (q12 = 0; q12 < 2; q12++) {
        uint32_t expected = reference_backend(bits, (int)q12);
        int32_t direct = q12 ? func_80076554(input) : func_800764D0(input);
        output[0] = 0x11223344;
        output[1] = 0;
        output[2] = 0x55667788;
        assert((uint32_t)direct == expected);
        assert((q12 ? func_8005359C(input, &output[1]) :
                      func_800535CC(input, &output[1])) == 0);
        assert(output[0] == 0x11223344 && (uint32_t)output[1] == expected);
        assert(output[2] == 0x55667788);
    }
}

int main(void) {
    static const int32_t values[] = {
        INT32_MIN, -65536, -4097, -1, 0, 1, 4096, 65536, INT32_MAX,
    };
    int16_t saved_domain[512];
    unsigned index, bit, x, y, z;

    for (index = 0; index < 512; index++) {
        uint32_t bits = random_word() & 0xFFFFu;
        g_sqrt_lookup_domain[index] = (int16_t)(
            bits < 0x8000u ? (int32_t)bits : (int32_t)bits - 0x10000);
    }
    g_sqrt_lookup_domain[0] = INT16_MIN;
    g_sqrt_lookup_domain[1] = INT16_MAX;
    g_sqrt_lookup_domain[191] = -1;
    g_sqrt_lookup_domain[320] = 4096;
    g_sqrt_lookup_domain[511] = INT16_MAX;
    memcpy(saved_domain, g_sqrt_lookup_domain, sizeof(saved_domain));

    run_case(0);
    run_case(0xFFFFFFFFu);
    for (bit = 0; bit < 32; bit++) {
        uint32_t value = 1u << bit;
        run_case(value);
        run_case(value - 1u);
        run_case(value + 1u);
        run_case(~value);
        run_case(~(value - 1u));
        run_case(~(value + 1u));
    }
    for (index = 0; index < 100000; index++) {
        run_case(random_word());
    }
    for (x = 0; x < sizeof(values) / sizeof(values[0]); x++) {
        for (y = 0; y < sizeof(values) / sizeof(values[0]); y++) {
            for (z = 0; z < sizeof(values) / sizeof(values[0]); z++) {
                int32_t vector[3] = {values[x], values[y], values[z]};
                int32_t output[3] = {0x11223344, 0, 0x55667788};
                uint32_t squares = (uint32_t)vector[0] * (uint32_t)vector[0] +
                                   (uint32_t)vector[1] * (uint32_t)vector[1] +
                                   (uint32_t)vector[2] * (uint32_t)vector[2];
                assert(move_vector_length_from_wrapped_squares(vector, &output[1]) == 0);
                assert(output[0] == 0x11223344);
                assert((uint32_t)output[1] == reference_backend(squares, 0));
                assert(output[2] == 0x55667788);
            }
        }
    }
    assert(memcmp(saved_domain, g_sqrt_lookup_domain, sizeof(saved_domain)) == 0);
    puts("GTE square-root candidates: signed lookup, scaling and wrapper integration passed");
    return 0;
}
