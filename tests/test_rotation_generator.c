#include "rotation_generator.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

uint32_t g_rotation_trig_table[4096];

typedef struct TestMemory {
    uint16_t before[16];
    ComponentRotationMatrix output;
    uint16_t after[16];
} TestMemory;

static uint32_t random_state = 0x77C50u;

static uint32_t random_word(void) {
    random_state = random_state * 1664525u + 1013904223u;
    return random_state;
}

static int32_t signed_half(uint32_t value) {
    value &= 0xFFFFu;
    return value < 0x8000u ? (int32_t)value : (int32_t)value - 0x10000;
}

static int32_t reference_product(int32_t left, int32_t right, int negate) {
    int64_t product = (int64_t)left * right;
    uint32_t bits = (uint32_t)(negate ? -product : product);
    int64_t signed_value = bits < 0x80000000u ? (int64_t)bits : (int64_t)bits - 0x100000000LL;

    if (signed_value < 0) {
        return (int32_t)-((-signed_value + 4095) / 4096);
    }
    return (int32_t)(signed_value / 4096);
}

static void reference_trig(uint16_t bits, int32_t *sine, int32_t *cosine) {
    int32_t angle = signed_half(bits);
    uint32_t index = (uint32_t)(angle < 0 ? -angle : angle) % 4096u;
    uint32_t packed = g_rotation_trig_table[index];
    int32_t low = signed_half(packed);

    *sine = angle < 0 ? -low : low;
    *cosine = signed_half(packed / 0x10000u);
}

static void reference_matrix(const uint16_t angles[3], uint16_t result[9]) {
    int32_t sx, cx, sy, cy, sz, cz;
    int32_t x_part, z_part;

    reference_trig(angles[0], &sx, &cx);
    reference_trig(angles[1], &sy, &cy);
    reference_trig(angles[2], &sz, &cz);
    x_part = reference_product(cz, -sy, 0);
    z_part = reference_product(sz, -sy, 0);
    result[0] = (uint16_t)reference_product(cz, cy, 0);
    result[1] = (uint16_t)reference_product(sz, cy, 1);
    result[2] = (uint16_t)sy;
    result[3] = (uint16_t)(reference_product(sz, cx, 0) - reference_product(x_part, sx, 0));
    result[4] = (uint16_t)(reference_product(cz, cx, 0) + reference_product(z_part, sx, 0));
    result[5] = (uint16_t)reference_product(cy, sx, 1);
    result[6] = (uint16_t)(reference_product(sz, sx, 0) + reference_product(x_part, cx, 0));
    result[7] = (uint16_t)(reference_product(cz, sx, 0) - reference_product(z_part, cx, 0));
    result[8] = (uint16_t)reference_product(cy, cx, 0);
}

static void run_case(const uint16_t values[3], unsigned overlap) {
    TestMemory actual, expected;
    uint16_t *angles;
    uint16_t result[9];
    unsigned index;

    for (index = 0; index < sizeof(actual); index++) {
        ((unsigned char *)&actual)[index] = (unsigned char)random_word();
    }
    angles = overlap == 0 ? actual.before : &actual.output.elements[overlap - 1];
    memcpy(angles, values, 3 * sizeof(*angles));
    reference_matrix(angles, result);
    memcpy(&expected, &actual, sizeof(expected));
    memcpy(expected.output.elements, result, sizeof(result));

    assert(func_80077C50(angles, &actual.output) == &actual.output);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

int main(void) {
    static const uint16_t edges[] = {
        0, 1, 2, 1024, 2048, 3072, 4095, 4096, 4097, 0x7FFF,
        0x8000, 0x8001, 0xEFFF, 0xF000, 0xF001, 0xFFFF,
    };
    static const unsigned overlaps[] = {0, 1, 2, 4, 7};
    uint16_t angles[3];
    unsigned index, x, y, z, overlap;

    for (index = 0; index < 4096; index++) {
        g_rotation_trig_table[index] = random_word();
    }
    g_rotation_trig_table[0] = 0x7FFF8000u;
    g_rotation_trig_table[1] = 0x80007FFFu;
    g_rotation_trig_table[1024] = 0u;
    g_rotation_trig_table[2048] = 0x0001FFFFu;
    g_rotation_trig_table[3072] = 0xF0001000u;
    g_rotation_trig_table[4095] = 0x7FFF0000u;

    for (x = 0; x < sizeof(edges) / sizeof(edges[0]); x++) {
        for (y = 0; y < sizeof(edges) / sizeof(edges[0]); y++) {
            for (z = 0; z < sizeof(edges) / sizeof(edges[0]); z++) {
                angles[0] = edges[x];
                angles[1] = edges[y];
                angles[2] = edges[z];
                for (overlap = 0; overlap < sizeof(overlaps) / sizeof(overlaps[0]); overlap++) {
                    run_case(angles, overlaps[overlap]);
                }
            }
        }
    }
    for (index = 0; index < 5000; index++) {
        angles[0] = (uint16_t)random_word();
        angles[1] = (uint16_t)random_word();
        angles[2] = (uint16_t)random_word();
        for (overlap = 0; overlap < sizeof(overlaps) / sizeof(overlaps[0]); overlap++) {
            run_case(angles, overlaps[overlap]);
        }
    }
    puts("Rotation generator: wrapped Q12 arithmetic, lookup, overlap and preservation passed");
    return 0;
}
