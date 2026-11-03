#include "component_rotation.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

enum Helper { DIFFERENCE, CONDITION, CROSS, NORMALIZE };

typedef struct ExpectedCall {
    enum Helper helper;
    int32_t left[3];
    int32_t right[3];
    int32_t output[3];
    int32_t status;
} ExpectedCall;

static ExpectedCall calls[10];
static unsigned count;
static unsigned next;
static ComponentRotationMatrix *mutated;
static const int32_t cross_a[3] = {INT32_MIN, 65537, INT32_MAX};
static const int32_t cross_b[3] = {-65537, 12345, -32768};
static const int32_t axis_a[3] = {-32768, 32768, 65535};
static const int32_t axis_b[3] = {65537, INT32_MIN, INT32_MAX};
static const int32_t forward[3] = {0x12345678, -1, -65537};
static const int32_t vertical[3] = {0, -4096, 0};
static ComponentRotationMatrix *scale_component;
static ComponentRotationMatrix scale_reference;
static int32_t *live_scale;
static int32_t reference_scale[3];
static unsigned scale_call;
static int scale_mutation;
static int forced_product;
static int32_t product_value;
static const unsigned scale_order[9] = {0, 3, 6, 1, 4, 7, 2, 5, 8};

static int32_t wrapped_product(int32_t left, int32_t right) {
    int64_t product = (int64_t)left * right;
    int64_t shifted = product / 4096;
    uint32_t bits;
    if (product < 0 && product % 4096 != 0) {
        --shifted;
    }
    bits = (uint32_t)shifted;
    return bits <= INT32_MAX ? (int32_t)bits : (int32_t)((int64_t)bits - 4294967296LL);
}

int32_t func_8001007C(int32_t left, int32_t right, int32_t *output) {
    unsigned index, axis;
    int32_t sample;
    assert(scale_call < 9);
    index = scale_order[scale_call];
    axis = scale_call / 3;
    sample = scale_reference.elements[index];
    if (sample >= 0x8000) {
        sample -= 0x10000;
    }
    assert(left == sample && right == reference_scale[axis]);
    *output = forced_product ? product_value : wrapped_product(left, right);
    if (scale_mutation) {
        unsigned next_index = scale_order[(scale_call + 1) % 9];
        uint16_t changed = (uint16_t)(0x8000u + scale_call * 1234u);
        scale_component->elements[next_index] = changed;
        scale_reference.elements[next_index] = changed;
        scale_component->translation[1] = INT32_MIN;
        scale_reference.translation[1] = INT32_MIN;
        live_scale[axis] = INT32_MAX - (int32_t)scale_call;
        reference_scale[axis] = live_scale[axis];
    }
    sample = *output;
    if (sample > 32767) {
        sample = 32767;
    }
    if (sample < -32767) {
        sample = -32767;
    }
    scale_reference.elements[index] = (uint16_t)sample;
    ++scale_call;
    return -123;
}

static void reset(void) {
    count = 0;
    next = 0;
    mutated = NULL;
}

static void expect(enum Helper helper, const int32_t left[3], const int32_t right[3],
    const int32_t output[3], int32_t status) {
    ExpectedCall *call;
    assert(count < sizeof(calls) / sizeof(calls[0]));
    call = &calls[count++];
    call->helper = helper;
    memcpy(call->left, left, sizeof(call->left));
    if (right != NULL) {
        memcpy(call->right, right, sizeof(call->right));
    }
    memcpy(call->output, output, sizeof(call->output));
    call->status = status;
}

static int32_t execute(enum Helper helper, const int32_t left[3],
    const int32_t right[3], int32_t output[3]) {
    ExpectedCall *call;
    assert(next < count);
    call = &calls[next++];
    assert(call->helper == helper);
    assert(memcmp(left, call->left, sizeof(call->left)) == 0);
    if (right != NULL) {
        assert(memcmp(right, call->right, sizeof(call->right)) == 0);
    }
    if (helper == CONDITION) {
        assert(left == output);
    }
    memcpy(output, call->output, sizeof(call->output));
    if (mutated != NULL && next == 1) {
        mutated->elements[0] = 0x7777;
        mutated->translation[1] = INT32_MIN;
    }
    return call->status;
}

int32_t func_80010000(const int32_t left[3], const int32_t right[3], int32_t output[3]) {
    return execute(DIFFERENCE, left, right, output);
}

int32_t func_80053D68(const int32_t input[3], int32_t output[3]) {
    return execute(CONDITION, input, NULL, output);
}

int32_t func_80010128(const int32_t left[3], const int32_t right[3], int32_t output[3]) {
    return execute(CROSS, left, right, output);
}

int32_t func_8005395C(const int32_t input[3], int32_t output[3]) {
    return execute(NORMALIZE, input, NULL, output);
}

static int32_t signed_halfword(unsigned value) {
    return value < 0x8000u ? (int32_t)value : (int32_t)value - 0x10000;
}

static void set_columns(ComponentRotationMatrix *matrix,
    const int32_t first[3], const int32_t second[3], const int32_t third[3]) {
    unsigned i;
    for (i = 0; i < 3; ++i) {
        matrix->elements[i * 3] = (uint16_t)first[i];
        matrix->elements[i * 3 + 1] = (uint16_t)second[i];
        matrix->elements[i * 3 + 2] = (uint16_t)third[i];
    }
}

static void test_reconstruction(void) {
    unsigned value, inplace, i;
    for (value = 0; value <= UINT16_MAX; ++value) {
        for (inplace = 0; inplace < 2; ++inplace) {
            ComponentRotationMatrix input;
            ComponentRotationMatrix output;
            ComponentRotationMatrix expected;
            int32_t second[3], third[3];
            for (i = 0; i < 9; ++i) {
                input.elements[i] = (uint16_t)(value ^ (i * 0x1111u));
            }
            input.padding = 0xCAFE;
            input.translation[0] = INT32_MAX;
            input.translation[1] = 123;
            input.translation[2] = INT32_MIN;
            output = input;
            for (i = 0; i < 3; ++i) {
                second[i] = signed_halfword(input.elements[i * 3 + 1]);
                third[i] = signed_halfword(input.elements[i * 3 + 2]);
            }
            reset();
            expect(CONDITION, second, NULL, axis_a, -1);
            expect(CONDITION, third, NULL, axis_b, -2);
            expect(CROSS, axis_a, axis_b, cross_a, -3);
            expect(CONDITION, cross_a, NULL, cross_b, -4);
            expect(CROSS, cross_b, axis_a, cross_a, -5);
            expect(NORMALIZE, cross_b, NULL, forward, -6);
            expect(NORMALIZE, axis_a, NULL, axis_b, -7);
            expect(NORMALIZE, cross_a, NULL, axis_a, -8);
            mutated = inplace ? &input : &output;
            expected = output;
            expected.translation[1] = INT32_MIN;
            set_columns(&expected, forward, axis_b, axis_a);
            assert(func_800542A4(&input, mutated) == 0);
            assert(next == count);
            assert(memcmp(mutated, &expected, sizeof(expected)) == 0);
        }
    }
}

static void test_orientation(void) {
    static const int32_t directions[][3] = {
        {0, 0, 0}, {0, 4096, 0}, {0, -4096, 0},
        {INT32_MIN, INT32_MAX, -1}, {1, 0, 0}, {0, 0, 1}
    };
    static const int32_t statuses[] = {0, 1, -1, INT32_MIN, INT32_MAX};
    unsigned d, failure, status;
    for (d = 0; d < sizeof(directions) / sizeof(directions[0]); ++d) {
        for (failure = 0; failure < 4; ++failure) {
            for (status = 0; status < sizeof(statuses) / sizeof(statuses[0]); ++status) {
                ComponentRotationMatrix component = {{1, 2, 3, 4, 5, 6, 7, 8, 9},
                    0xCAFE, {10, 20, 30}};
                ComponentRotationMatrix target = {{0}, 0, {40, 50, 60}};
                ComponentRotationMatrix expected = component;
                int32_t direction[3];
                int32_t result = 0;
                int zero = d == 0;
                int stop;
                reset();
                expect(DIFFERENCE, target.translation, component.translation,
                    directions[d], failure == 1 ? statuses[status] : 0);
                mutated = &component;
                expected.elements[0] = 0x7777;
                expected.translation[1] = INT32_MIN;
                stop = failure == 1 && statuses[status] != 0;
                if (stop) {
                    result = statuses[status];
                }
                if (!stop && !zero) {
                    expect(NORMALIZE, directions[d], NULL, forward, -7);
                    expect(CONDITION, directions[d], NULL, directions[d], -8);
                    expect(CONDITION, vertical, NULL, vertical, -9);
                    memcpy(direction, directions[d], sizeof(direction));
                    if (direction[0] == 0 && direction[2] == 0) {
                        direction[0] = 1;
                        direction[2] = 1;
                    }
                    expect(CROSS, direction, vertical, cross_a,
                        failure == 2 ? statuses[status] : 0);
                    stop = failure == 2 && statuses[status] != 0;
                    if (stop) {
                        result = statuses[status];
                    } else {
                        expect(NORMALIZE, cross_a, NULL, axis_a, -10);
                        expect(CONDITION, axis_a, NULL, axis_b, -11);
                        expect(CROSS, direction, axis_b, cross_b,
                            failure == 3 ? statuses[status] : 0);
                        stop = failure == 3 && statuses[status] != 0;
                        if (stop) {
                            result = statuses[status];
                        } else {
                            expect(NORMALIZE, cross_b, NULL, axis_b, -12);
                            set_columns(&expected, axis_a, axis_b, forward);
                        }
                    }
                }
                assert(func_800550A4(&component, &target) == result);
                assert(next == count);
                assert(memcmp(&component, &expected, sizeof(component)) == 0);
            }
        }
    }
    reset();
    assert(func_800550A4(NULL, NULL) == 0x18);
    {
        ComponentRotationMatrix matrix = {{0}, 0, {0}};
        assert(func_800550A4(&matrix, NULL) == 0x18);
        assert(func_800550A4(NULL, &matrix) == 0x18);
    }
    assert(next == 0);
}

static void run_scale_case(unsigned value, int32_t factor, int mutation, int force) {
    ComponentRotationMatrix component;
    int32_t scale[3] = {factor, factor, factor};
    unsigned i;
    for (i = 0; i < 9; ++i) {
        component.elements[i] = (uint16_t)(value ^ (i * 0x1111u));
    }
    component.padding = 0xCAFE;
    component.translation[0] = 123;
    component.translation[1] = 456;
    component.translation[2] = 789;
    scale_reference = component;
    scale_component = &component;
    live_scale = scale;
    memcpy(reference_scale, scale, sizeof(scale));
    scale_call = 0;
    scale_mutation = mutation;
    forced_product = force;
    assert(func_8005541C(&component, scale) == 0);
    assert(scale_call == 9);
    assert(memcmp(&component, &scale_reference, sizeof(component)) == 0);
}

static void test_column_scaling(void) {
    static const int32_t factors[] = {
        INT32_MIN, -65537, -4096, -1, 0, 1, 4096, 65537, INT32_MAX
    };
    static const int32_t products[] = {
        INT32_MIN, -32769, -32768, -32767, -32766,
        -1, 0, 1, 32766, 32767, 32768, INT32_MAX
    };
    unsigned value, factor, mutation;
    for (value = 0; value <= UINT16_MAX; ++value) {
        for (factor = 0; factor < sizeof(factors) / sizeof(factors[0]); ++factor) {
            run_scale_case(value, factors[factor], 0, 0);
        }
    }
    for (value = 0; value < sizeof(products) / sizeof(products[0]); ++value) {
        product_value = products[value];
        for (mutation = 0; mutation < 2; ++mutation) {
            run_scale_case(0x8000, -4096, (int)mutation, 1);
        }
    }
}

int main(void) {
    test_reconstruction();
    test_orientation();
    test_column_scaling();
    puts("Grounded basis: 131072 reconstruction, 120 orientation/failure, 589824 scaling and 24 clamp/mutation cases passed.");
    return 0;
}
