#include <stdint.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int32_t func_8001007C(int32_t left, int32_t right, int32_t *output);
int32_t func_800100A0(const int32_t *left, const int32_t *right, int32_t *output);
int32_t func_80010128(const int32_t *left, const int32_t *right, int32_t *output);

static int32_t signed_bits(uint32_t bits) {
    int32_t value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static uint32_t fixed_product(int32_t left, int32_t right) {
    int64_t product = (int64_t)left * right;
    return (uint32_t)((uint64_t)product >> 12);
}

static void reference_dot(int32_t *memory, unsigned int left, unsigned int right,
                          unsigned int output) {
    uint32_t total = 0;
    unsigned int index;
    for (index = 0; index < 3; ++index) {
        total += fixed_product(memory[left + index], memory[right + index]);
    }
    memory[output] = signed_bits(total);
}

static void reference_cross(int32_t *memory, unsigned int left, unsigned int right,
                            unsigned int output) {
    static const unsigned int components[3][4] = {
        {1, 2, 2, 1}, {2, 0, 0, 2}, {0, 1, 1, 0}
    };
    unsigned int index;
    for (index = 0; index < 3; ++index) {
        const unsigned int *c = components[index];
        uint32_t value = fixed_product(memory[left + c[0]], memory[right + c[1]])
            - fixed_product(memory[left + c[2]], memory[right + c[3]]);
        memory[output + index] = signed_bits(value);
    }
}

int main(void) {
    static const uint32_t boundaries[] = {
        0, 1, 0xFFF, 0x1000, 0x7FFFFFFF, 0x80000000u, UINT32_MAX
    };
    static const unsigned int layouts[][3] = {
        {0, 3, 6}, {0, 3, 0}, {0, 3, 3}, {0, 2, 1}
    };
    uint32_t random_state = 0xB3Du;
    unsigned int iteration, layout, function, index, left_index, right_index;

    for (left_index = 0; left_index < sizeof(boundaries) / sizeof(boundaries[0]); ++left_index) {
        for (right_index = 0; right_index < sizeof(boundaries) / sizeof(boundaries[0]); ++right_index) {
            int32_t left = signed_bits(boundaries[left_index]);
            int32_t right = signed_bits(boundaries[right_index]);
            int32_t result[] = {123, 456, 789};

            assert(func_8001007C(left, right, &result[1]) == 0);
            assert(result[0] == 123 && result[2] == 789);
            assert(result[1] == signed_bits(fixed_product(left, right)));
        }
    }

    for (iteration = 0; iteration < 10000; ++iteration) {
        int32_t initial[9];
        int32_t scalar_result;
        for (index = 0; index < 9; ++index) {
            uint32_t bits;
            if (iteration < 128) {
                bits = boundaries[(iteration + index * 3) %
                                  (sizeof(boundaries) / sizeof(boundaries[0]))];
            } else {
                random_state = random_state * 1664525u + 1013904223u;
                bits = random_state;
            }
            initial[index] = signed_bits(bits);
        }
        assert(func_8001007C(initial[0], initial[8], &scalar_result) == 0);
        assert(scalar_result == signed_bits(fixed_product(initial[0], initial[8])));
        for (layout = 0; layout < sizeof(layouts) / sizeof(layouts[0]); ++layout) {
            const unsigned int left = layouts[layout][0];
            const unsigned int right = layouts[layout][1];
            const unsigned int output = layouts[layout][2];
            for (function = 0; function < 2; ++function) {
                int32_t actual[9], expected[9];
                memcpy(actual, initial, sizeof(actual));
                memcpy(expected, initial, sizeof(expected));
                if (function == 0) {
                    reference_dot(expected, left, right, output);
                    assert(func_800100A0(actual + left, actual + right, actual + output) == 0);
                } else {
                    reference_cross(expected, left, right, output);
                    assert(func_80010128(actual + left, actual + right, actual + output) == 0);
                }
                assert(memcmp(actual, expected, sizeof(actual)) == 0);
            }
        }
    }
    puts("Fixed-point products: 10049 scalar and 80000 vector cases passed.");
    return 0;
}
