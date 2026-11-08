#include "level_records.h"
#include "queue_update.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

typedef struct ComponentBasisView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentBasisView;

typedef struct ComponentTransformView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentTransformView;

extern int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t output[3]
);
extern int32_t func_80054C7C(
    ComponentTransformView *component, const ComponentBasisView *basis, const int32_t input[3]
);

uint8_t g_unknown_800BE0C8;
int32_t g_unknown_800BE0D4;
const ComponentBasisView g_global_basis = {
    {1, 2, 3, 4}, 5, 6, {7, 8, 9}
};

static MovePreparationActor actor;
static MovePreparationComponent holder;
static ComponentTransformView transform;
static int actor_available;
static int getter_calls;
static int normalizer_calls;
static int transform_calls;
static const ComponentBasisView *expected_basis;
static int32_t expected_input[3];

void *func_80028764(int32_t index) {
    assert(index == 0);
    ++getter_calls;
    return actor_available ? &actor : NULL;
}

int32_t func_8005476C(const ComponentBasisView *basis, ComponentBasisView *output) {
    assert(basis == expected_basis);
    ++normalizer_calls;
    *output = *basis;
    output->translation[0] = 1234;
    return -1;
}

int32_t func_80064548(
    const int32_t input[3], const ComponentBasisView *basis, int32_t output[3]
) {
    assert(normalizer_calls == 1 && basis->translation[0] == 1234);
    assert(memcmp(input, expected_input, sizeof(expected_input)) == 0);
    ++transform_calls;
    output[0] = -41;
    output[1] = INT32_MIN;
    output[2] = 73;
    return -1;
}

int32_t func_80064528(
    const int32_t input[3], const ComponentBasisView *basis, int32_t output[3]
) {
    assert(normalizer_calls == 1 && basis->translation[0] == 1234);
    assert(memcmp(input, expected_input, sizeof(expected_input)) == 0);
    ++transform_calls;
    output[0] = 41;
    output[1] = INT32_MIN;
    output[2] = -73;
    return -1;
}

static void reset_fixture(void) {
    memset(&actor, 0, sizeof(actor));
    memset(&holder, 0, sizeof(holder));
    memset(&transform, 0, sizeof(transform));
    actor.component_1C = &holder;
    holder.transform_14 = &transform;
    transform.matrix_words[0] = 0x12345678;
    transform.matrix_last = -321;
    transform.padding = 0x5A5A;
    transform.translation[0] = INT32_MIN;
    transform.translation[2] = INT32_MAX;
    actor_available = 1;
    getter_calls = 0;
    normalizer_calls = 0;
    transform_calls = 0;
}

static void test_queue_adjustment(void) {
    static const int32_t values[] = {INT32_MIN, INT32_MAX, -1, 0, 1};
    unsigned int flag, y_index, offset_index;

    for (flag = 0; flag < 256; ++flag) {
        for (y_index = 0; y_index < sizeof(values) / sizeof(values[0]); ++y_index) {
            for (offset_index = 0; offset_index < sizeof(values) / sizeof(values[0]);
                 ++offset_index) {
                uint32_t expected_y = (uint32_t)values[y_index];
                reset_fixture();
                transform.translation[1] = values[y_index];
                g_unknown_800BE0C8 = (uint8_t)flag;
                g_unknown_800BE0D4 = values[offset_index];
                assert(func_8003ABE8() == 1);
                if (flag != 0) {
                    expected_y += (uint32_t)values[offset_index];
                }
                assert((uint32_t)transform.translation[1] == expected_y);
                assert(transform.translation[0] == INT32_MIN);
                assert(transform.translation[2] == INT32_MAX);
                assert(transform.matrix_words[0] == 0x12345678);
                assert(transform.matrix_last == -321 && transform.padding == 0x5A5A);
                assert(getter_calls == (flag != 0));
                assert(g_unknown_800BE0D4 == (flag != 0 ? 0 : values[offset_index]));
            }
        }
    }

    reset_fixture();
    actor_available = 0;
    transform.translation[1] = 31;
    g_unknown_800BE0C8 = 1;
    g_unknown_800BE0D4 = -7;
    assert(func_8003ABE8() == 1);
    assert(getter_calls == 1 && transform.translation[1] == 31);
    assert(g_unknown_800BE0D4 == -7);
}

static void test_real_translation_helpers(void) {
    ComponentBasisView basis = g_global_basis;
    int32_t output[3] = {11, 12, 13};
    const int32_t input[3] = {17, INT32_MIN, -19};

    reset_fixture();
    transform.translation[1] = INT32_MIN;
    assert(func_80054B20(NULL, NULL, output) == 0x18);
    assert(output[0] == 11 && output[1] == 12 && output[2] == 13);
    assert(func_80054C7C(NULL, NULL, input) == 0x18);

    basis.padding ^= 1;
    assert(func_80054B20(&transform, &basis, output) == 0);
    assert(output[0] == INT32_MIN && output[1] == INT32_MIN && output[2] == INT32_MAX);
    assert(func_80054C7C(&transform, &basis, input) == 0);
    assert(transform.translation[0] == 17 && transform.translation[1] == INT32_MIN);
    assert(transform.translation[2] == -19);
    assert(normalizer_calls == 0 && transform_calls == 0);

    basis.translation[2] ^= 1;
    expected_basis = &basis;
    expected_input[0] = 17;
    expected_input[1] = INT32_MIN;
    expected_input[2] = -19;
    assert(func_80054B20(&transform, &basis, output) == 0);
    assert(output[0] == -41 && output[1] == INT32_MIN && output[2] == 73);
    assert(normalizer_calls == 1 && transform_calls == 1);

    normalizer_calls = 0;
    transform_calls = 0;
    memcpy(expected_input, input, sizeof(expected_input));
    assert(func_80054C7C(&transform, &basis, input) == 0);
    assert(transform.translation[0] == 41 && transform.translation[1] == INT32_MIN);
    assert(transform.translation[2] == -73);
    assert(normalizer_calls == 1 && transform_calls == 1);
}

int main(void) {
    test_queue_adjustment();
    test_real_translation_helpers();
    puts("Queue adjustment with real translation helpers: gates, wrapping, basis paths passed.");
    return 0;
}
