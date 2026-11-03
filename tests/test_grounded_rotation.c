#include "component_rotation.h"
#include "shared_actor_state_dispatch.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

uint8_t g_stride_mode;

extern int32_t func_80054AF0(const int16_t *component, int32_t *output);

static SharedActorDispatchTarget actor;
static PlayerActorSharedStateView shared;
static PlayerActorSharedStateView replacement;
static ComponentRotationMatrix context;
static ComponentRotationMatrix saved_context;
static ComponentRotationMatrix rotation_matrix;
static uint16_t expected_angles[3];
static unsigned calls[8];
static unsigned call_count;
static int32_t sequence;
static int32_t expected_stride;
static int32_t expected_delta;
static int mutate_query;
static int mutate_rotation;
static int mutate_step;

static void record(unsigned call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

int32_t func_80052ABC(void *component, int32_t *output) {
    record(0x80052ABC);
    assert(component == &context);
    *output = sequence;
    if (mutate_query) {
        actor.flags_04 = 0x100;
        actor.shared_28 = &replacement;
    }
    return 0;
}

void func_80077C50(const uint16_t angles[3], ComponentRotationMatrix *output) {
    record(0x80077C50);
    assert(memcmp(angles, expected_angles, sizeof(expected_angles)) == 0);
    *output = rotation_matrix;
}

void func_80076890(ComponentRotationMatrix *rotation, const ComponentRotationMatrix *basis) {
    record(0x80076890);
    assert(memcmp(rotation, &rotation_matrix, sizeof(*rotation)) == 0);
    assert(memcmp(basis, &saved_context, sizeof(*basis)) == 0);
    rotation->elements[0] = 0x5678;
}

int32_t func_800542A4(const ComponentRotationMatrix *input, ComponentRotationMatrix *output) {
    record(0x800542A4);
    assert(input->elements[0] == 0x5678);
    *output = *input;
    if (mutate_rotation) {
        actor.state_10 = 0x8000;
        actor.shared_28 = &replacement;
        g_stride_mode = 7;
    }
    return -77;
}

void func_80022944(void *value) {
    record(0x80022944);
    assert(value == &actor);
    assert(shared.progress_30 == expected_stride);
    assert(shared.progress_34 == expected_delta);
    if (mutate_step) {
        shared.progress_30 = 777;
        shared.progress_34 = 888;
        actor.shared_28 = &replacement;
        context.elements[0] = 0x9999;
        context.translation[1] = INT32_MIN;
    }
}

static void reset(void) {
    unsigned i;
    memset(&actor, 0, sizeof(actor));
    memset(&shared, 0, sizeof(shared));
    memset(&replacement, 0, sizeof(replacement));
    for (i = 0; i < 9; ++i) {
        context.elements[i] = (uint16_t)(0xE000u + i * 0x101u);
        rotation_matrix.elements[i] = (uint16_t)(0xA000u + i);
    }
    context.padding = 0xCAFE;
    context.translation[0] = 10;
    context.translation[1] = 20;
    context.translation[2] = 30;
    rotation_matrix.padding = 0xBEEF;
    rotation_matrix.translation[0] = -1;
    rotation_matrix.translation[1] = -2;
    rotation_matrix.translation[2] = -3;
    saved_context = context;
    actor.component_1C = &context;
    actor.shared_28 = &shared;
    sequence = 0;
    g_stride_mode = 0;
    call_count = 0;
    mutate_query = 0;
    mutate_rotation = 0;
    mutate_step = 0;
}

static void test_restore_overlap(void) {
    static const uint16_t values[] = {0, 1, 0x7FFF, 0x8000, 0xFFFF};
    uint16_t storage[40];
    uint16_t expected[40];
    unsigned value, source, destination, i;
    for (value = 0; value < sizeof(values) / sizeof(values[0]); ++value) {
        for (source = 0; source <= 16; source += 2) {
            for (destination = 0; destination <= 16; destination += 2) {
                for (i = 0; i < 40; ++i) {
                    storage[i] = (uint16_t)(values[value] + i * 131u);
                }
                memcpy(expected, storage, sizeof(storage));
                for (i = 0; i < 9; ++i) {
                    uint16_t sample = expected[source + i];
                    expected[destination + i] = (i & 1u) ? (uint16_t)(0u - sample) : sample;
                }
                assert(func_800549E8(
                    (ComponentRotationMatrix *)(void *)(storage + destination),
                    (const ComponentRotationMatrix *)(const void *)(storage + source)) == 0);
                assert(memcmp(storage, expected, sizeof(storage)) == 0);
            }
        }
    }
}

static void test_rotation_wrapper(void) {
    static const int32_t angles[] = {INT32_MIN, -65537, -1, 0, 1, 65537, INT32_MAX};
    unsigned x, y, z;
    for (x = 0; x < sizeof(angles) / sizeof(angles[0]); ++x) {
        for (y = 0; y < sizeof(angles) / sizeof(angles[0]); ++y) {
            for (z = 0; z < sizeof(angles) / sizeof(angles[0]); ++z) {
                int32_t input[3] = {angles[x], angles[y], angles[z]};
                reset();
                expected_angles[0] = (uint16_t)(0u - (uint32_t)input[0]);
                expected_angles[1] = (uint16_t)input[1];
                expected_angles[2] = (uint16_t)(0u - (uint32_t)input[2]);
                assert(func_800547C8(&context, input) == 0);
                assert(call_count == 3);
                assert(calls[0] == 0x80077C50 && calls[1] == 0x80076890 && calls[2] == 0x800542A4);
                rotation_matrix.elements[0] = 0x5678;
                assert(memcmp(context.elements, rotation_matrix.elements, sizeof(context.elements)) == 0);
                assert(context.padding == saved_context.padding);
                assert(memcmp(context.translation, saved_context.translation, sizeof(context.translation)) == 0);
            }
        }
    }
    reset();
    assert(func_800547C8(NULL, NULL) == 0x18);
    assert(call_count == 0);
}

static void test_basis_direction(void) {
    unsigned value;
    for (value = 0; value <= UINT16_MAX; ++value) {
        int16_t basis[16] = {0};
        int32_t output[5] = {123, 456, 789, INT32_MIN, INT32_MAX};
        int32_t expected_x = value < 0x8000u ? (int32_t)value : (int32_t)value - 0x10000;
        unsigned y = value ^ 0x8000u;
        unsigned z = value ^ 0xFFFFu;
        int32_t expected_y = y < 0x8000u ? (int32_t)y : (int32_t)y - 0x10000;
        int32_t expected_z = z < 0x8000u ? (int32_t)z : (int32_t)z - 0x10000;
        basis[2] = (int16_t)expected_x;
        basis[5] = (int16_t)expected_y;
        basis[8] = (int16_t)expected_z;
        assert(func_80054AF0(basis, output) == 0);
        assert(output[0] == expected_x);
        assert(output[1] == -expected_y);
        assert(output[2] == expected_z);
        assert(output[3] == INT32_MIN && output[4] == INT32_MAX);
    }
}

static void test_dispatch_boundaries(void) {
    static const int32_t strides[] = {INT32_MIN, -53, -52, -18, -17, -1, 0, 4, 5, 16, 17, 18, INT32_MAX};
    static const int32_t modes[] = {INT32_MIN, -1, 0, 1, 2, 3, 4, INT32_MAX};
    static const uint32_t states[] = {0, 4, 0x40, 0x8000, 0x10000, 0x18000};
    unsigned stride, mode, state, model;
    for (stride = 0; stride < sizeof(strides) / sizeof(strides[0]); ++stride) {
        for (mode = 0; mode < sizeof(modes) / sizeof(modes[0]); ++mode) {
            for (state = 0; state < sizeof(states) / sizeof(states[0]); ++state) {
                for (model = 0; model < 3; ++model) {
                    int32_t selected_mode = modes[mode];
                    int callbacks = 0;
                    int restore_stride = 0;
                    reset();
                    actor.state_10 = states[state];
                    g_stride_mode = (uint8_t)model;
                    shared.progress_30 = strides[stride];
                    shared.progress_34 = -123456;
                    expected_stride = shared.progress_30;
                    expected_delta = shared.progress_34;
                    if (selected_mode == 0 && !(states[state] & 0x18000)) {
                        if (expected_stride < 5) expected_stride = 5;
                        if (expected_stride >= 17) {
                            expected_stride = 17;
                            expected_delta = 0;
                        } else {
                            expected_delta = 0x198C;
                        }
                        callbacks = 1;
                    } else if (selected_mode == 1 && !(states[state] & 0x8000)) {
                        if (states[state] & 0x10000) {
                            expected_stride = -52;
                        } else {
                            expected_delta = expected_stride < -17 ? 0 : -0x198C;
                        }
                        callbacks = 1;
                    } else if ((selected_mode == 2 || selected_mode == 3) && !(states[state] & 0x10000)) {
                        expected_stride = (states[state] & 0x8000) ? (model ? 6 : 8) : 14;
                        expected_angles[0] = 0;
                        expected_angles[1] = selected_mode == 3 ? 0x400 : (uint16_t)-0x400;
                        expected_angles[2] = 0;
                        callbacks = 1;
                        restore_stride = 1;
                    }
                    shared_actor_state_dispatch(&actor, &context, selected_mode);
                    assert(calls[0] == 0x80052ABC);
                    assert(call_count == (unsigned)(restore_stride ? 5 : callbacks ? 2 : 1));
                    assert(shared.progress_30 == (restore_stride ? strides[stride] : expected_stride));
                    assert(shared.progress_34 == -123456);
                    if (restore_stride) {
                        assert(calls[1] == 0x80077C50 && calls[2] == 0x80076890 && calls[3] == 0x800542A4 && calls[4] == 0x80022944);
                        for (unsigned i = 0; i < 9; ++i) {
                            assert(context.elements[i] == (uint16_t)(0xE000u + i * 0x101u));
                        }
                        assert(context.padding == saved_context.padding);
                    }
                }
            }
        }
    }
}

static void test_dispatch_mutations(void) {
    reset();
    shared.progress_30 = 12;
    shared.progress_34 = 34;
    sequence = 0x2A7;
    shared_actor_state_dispatch(&actor, &context, 2);
    assert(call_count == 1 && shared.progress_30 == 0 && shared.progress_34 == 0);

    reset();
    shared.progress_30 = 12;
    shared.progress_34 = 34;
    mutate_query = 1;
    shared_actor_state_dispatch(&actor, &context, 0);
    assert(call_count == 1 && shared.progress_30 == 0 && shared.progress_34 == 0);
    assert(actor.shared_28 == &replacement && replacement.progress_30 == 0);

    reset();
    shared.progress_30 = 12;
    shared.progress_34 = 34;
    expected_stride = 6;
    expected_delta = 34;
    expected_angles[0] = 0;
    expected_angles[1] = 0x400;
    expected_angles[2] = 0;
    mutate_rotation = 1;
    mutate_step = 1;
    shared_actor_state_dispatch(&actor, &context, 3);
    assert(call_count == 5 && shared.progress_30 == 12 && shared.progress_34 == 34);
    assert(actor.shared_28 == &replacement && replacement.progress_30 == 0);
    assert(context.elements[0] == 0xE000 && context.translation[1] == INT32_MIN);

    reset();
    shared.progress_30 = 16;
    shared.progress_34 = 34;
    expected_stride = 16;
    expected_delta = 0x198C;
    mutate_step = 1;
    shared_actor_state_dispatch(&actor, &context, 0);
    assert(shared.progress_30 == 777 && shared.progress_34 == 34);
}

int main(void) {
    test_restore_overlap();
    test_rotation_wrapper();
    test_basis_direction();
    test_dispatch_boundaries();
    test_dispatch_mutations();
    puts("Grounded rotation: 405 overlap cases, 343 angle cases, 65536 basis directions, 1872 dispatch boundaries and helper mutations passed.");
    return 0;
}
