#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct ComponentBasisView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentBasisView;

typedef struct GteTransformMatrix {
    uint32_t words[8];
} GteTransformMatrix;

const volatile ComponentBasisView g_gte_global_basis = {{10, 20, 30, 40}, -7, 0, {0, 0, 0}};
const volatile GteTransformMatrix g_camera_view_transform = {{0, 1, 0x80000000u, 3, 4, 5, 6, UINT32_MAX}};
static int transform_calls, matrix_calls;
static const void *object_argument;

int32_t func_80053FA4(const int32_t input[4], const volatile ComponentBasisView *basis, int32_t output[4]);
int32_t func_800540BC(const int32_t input[4], const ComponentBasisView *basis, int32_t output[3]);
void func_800722C8(const void *object);

int32_t func_80077298(const volatile ComponentBasisView *basis, const int32_t input[4], int32_t output[4]) {
    assert(basis != NULL && input != NULL);
    transform_calls++;
    output[0] = -5;
    output[1] = 6;
    output[2] = 7;
    return -1;
}

int32_t func_80010038(const int32_t left[3], const int32_t right[3], int32_t output[3]) {
    for (int item = 0; item < 3; item++) {
        output[item] = (int32_t)((uint32_t)left[item] + (uint32_t)right[item]);
    }
    return 0;
}

void func_8007754C(void) {
    assert(matrix_calls++ == 0);
}

void *func_80076890(GteTransformMatrix *matrix, const void *object) {
    assert(matrix_calls++ == 1 && object == object_argument);
    for (int item = 0; item < 8; item++) assert(matrix->words[item] == g_camera_view_transform.words[item]);
    matrix->words[0] = 0xDEADBEEFu;
    return NULL;
}

void func_800775EC(void) {
    assert(matrix_calls++ == 2);
}

void func_80076CCC(const GteTransformMatrix *matrix) {
    assert(matrix_calls++ == 3 && matrix->words[0] == 0xDEADBEEFu);
    for (int item = 1; item < 8; item++) assert(matrix->words[item] == g_camera_view_transform.words[item]);
}

int main(void) {
    ComponentBasisView basis = {{10, 20, 30, 40}, -7, 0, {INT32_MIN, 0, INT32_MAX}};
    const int32_t input[4] = {INT32_MAX, -9, INT32_MIN, 1234};
    int32_t output[4];

    for (int mismatch = -1; mismatch < 5; mismatch++) {
        ComponentBasisView test_basis = basis;
        if (mismatch >= 0 && mismatch < 4) test_basis.matrix_words[mismatch] ^= 0x80000000u;
        if (mismatch == 4) test_basis.matrix_last = INT16_MIN;
        for (int item = 0; item < 4; item++) output[item] = 99;
        transform_calls = 0;
        assert(func_80053FA4(input, &test_basis, output) == 0);
        assert(transform_calls == (mismatch >= 0));
        if (mismatch < 0) {
            assert(memcmp(input, output, sizeof(output)) == 0);
        } else {
            assert(output[0] == -5 && output[1] == 6 && output[2] == 7 && output[3] == 99);
        }
        transform_calls = 0;
        assert(func_800540BC(input, &test_basis, output) == 0);
        assert(transform_calls == (mismatch >= 0));
        const int32_t transformed[3] = {-5, 6, 7};
        for (int item = 0; item < 3; item++) {
            uint32_t value = (uint32_t)(mismatch < 0 ? input[item] : transformed[item]);
            assert((uint32_t)output[item] == value + (uint32_t)basis.translation[item]);
        }
    }

    int32_t overlap[5] = {1, -2, 3, -4, 99};
    assert(func_80053FA4(overlap, &basis, overlap + 1) == 0);
    assert(overlap[1] == 1 && overlap[2] == -2 && overlap[3] == 3 && overlap[4] == -4);
    assert(func_80053FA4(overlap, &basis, overlap) == 0);
    assert(overlap[0] == 1 && overlap[1] == 1 && overlap[2] == -2 && overlap[3] == 3);

    object_argument = &basis;
    matrix_calls = 0;
    func_800722C8(object_argument);
    assert(matrix_calls == 4);
    puts("Readable transform C: identity/mismatch, overlap, wrapping translation and matrix-copy call order passed");
    return 0;
}
