#include "component_rotation.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

_Static_assert(sizeof(ComponentRotationMatrix) == 32, "unexpected matrix layout");
_Static_assert(offsetof(ComponentRotationMatrix, translation) == 20, "unexpected translation offset");

static int32_t signed_element(uint16_t value) {
    return value < 0x8000U ? (int32_t)value : (int32_t)value - 65536;
}

static void reference(
    const ComponentRotationMatrix *left,
    const ComponentRotationMatrix *right,
    ComponentRotationMatrix *output
) {
    uint16_t values[9];
    unsigned row;
    unsigned column;
    unsigned inner;

    for (row = 0; row < 3; row++) {
        for (column = 0; column < 3; column++) {
            int64_t total = 0;
            int64_t result;
            for (inner = 0; inner < 3; inner++) {
                total += (int64_t)signed_element(left->elements[row * 3 + inner])
                    * signed_element(right->elements[inner * 3 + column]);
            }
            result = total / 4096;
            if (total < 0 && total % 4096 != 0) {
                result--;
            }
            if (result > 32767) result = 32767;
            if (result < -32768) result = -32768;
            values[row * 3 + column] = (uint16_t)result;
        }
    }
    memcpy(output->elements, values, sizeof(values));
    output->padding = values[8] & 0x8000U ? 0xFFFFU : 0;
}

static void check_case(const ComponentRotationMatrix *left, const ComponentRotationMatrix *right) {
    ComponentRotationMatrix left_copy = *left;
    ComponentRotationMatrix right_copy = *right;
    ComponentRotationMatrix first = *left;
    ComponentRotationMatrix second = *right;
    ComponentRotationMatrix expected_first = first;
    ComponentRotationMatrix expected_second = second;
    ComponentRotationMatrix alias = *left;
    ComponentRotationMatrix expected_alias = alias;

    reference(left, right, &expected_first);
    reference(left, right, &expected_second);
    reference(left, left, &expected_alias);
    assert(func_80076890(&first, right) == &first);
    assert(func_8007699C(left, &second) == &second);
    assert(func_80076890(&alias, &alias) == &alias);
    assert(memcmp(&first, &expected_first, sizeof(first)) == 0);
    assert(memcmp(&second, &expected_second, sizeof(second)) == 0);
    assert(memcmp(&alias, &expected_alias, sizeof(alias)) == 0);
    assert(memcmp(left, &left_copy, sizeof(*left)) == 0);
    assert(memcmp(right, &right_copy, sizeof(*right)) == 0);

    alias = *right;
    expected_alias = alias;
    reference(right, right, &expected_alias);
    assert(func_8007699C(&alias, &alias) == &alias);
    assert(memcmp(&alias, &expected_alias, sizeof(alias)) == 0);
}

static uint32_t random_word(uint32_t *state) {
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return *state;
}

int main(void) {
    static const uint16_t edges[] = {0, 1, 0x7FFF, 0x8000, 0xFFFF, 0x1000, 0xF000};
    ComponentRotationMatrix left = {{0}, 0xAAAA, {11, -22, 33}};
    ComponentRotationMatrix right = {{0}, 0xBBBB, {-44, 55, -66}};
    uint32_t state = 0x7699CU;
    unsigned index;
    unsigned iteration;

    for (iteration = 0; iteration < 343; iteration++) {
        for (index = 0; index < 9; index++) {
            left.elements[index] = edges[(iteration / 49 + index) % 7];
            right.elements[index] = edges[(iteration / 7 + index * 3 + iteration % 7) % 7];
        }
        check_case(&left, &right);
    }
    for (iteration = 0; iteration < 10000; iteration++) {
        for (index = 0; index < 9; index++) {
            left.elements[index] = (uint16_t)random_word(&state);
            right.elements[index] = (uint16_t)random_word(&state);
        }
        check_case(&left, &right);
    }
    puts("GTE composition host model: boundary, random, destination and alias cases passed");
    return 0;
}
