#include "move_step.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const char g_move_info_condition[] = "move info";
const char g_game_source_path[] = "game";

static ActorView actor;
static MoveInfoView move_info;
static MoveInfoView replacement_info;
static ComponentHolderView holder;
static int transform;
static int32_t conditioned[3];
static unsigned calls[8];
static size_t call_count;
static int32_t multiply_left[3];
static int32_t multiply_right[3];
static size_t multiply_count;
static int mutate_direction;
static int mutate_pending_y;
static int resume_after_special;

static void record(unsigned call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

int32_t func_800556D0(
    int32_t failed, const char *condition, const char *source, uint32_t line
) {
    record(0x800556D0);
    assert(failed == 0);
    assert(condition == g_move_info_condition);
    assert(source == g_game_source_path);
    assert(line == 0x2BE);
    actor.move_info_28 = &replacement_info;
    return 0;
}

int32_t func_800227A4(ActorView *value) {
    record(0x800227A4);
    assert(value == &actor);
    if (resume_after_special) {
        actor.state_00 = 0;
        actor.move_info_28 = &replacement_info;
        move_info.step_30 = 999;
    }
    return 0;
}

static int32_t product_q12(int32_t left, int32_t right) {
    return (int32_t)(uint32_t)((uint64_t)((int64_t)left * right) >> 12);
}

int32_t func_8001007C(int32_t left, int32_t right, int32_t *output) {
    record(0x8001007C);
    assert(multiply_count < 3);
    multiply_left[multiply_count] = left;
    multiply_right[multiply_count++] = right;
    *output = product_q12(left, right);
    if (multiply_count == 1) {
        if (mutate_direction) {
            move_info.direction_88[1] = -8192;
            move_info.direction_88[2] = 12288;
        }
        if (mutate_pending_y) {
            move_info.pending_38[1] = 77;
        }
    }
    return 0;
}

int32_t func_80054AF0(void *value, int32_t *output) {
    record(0x80054AF0);
    assert(value == &transform);
    output[0] = 11;
    output[1] = 22;
    output[2] = 33;
    output[3] = 44;
    return 0;
}

int32_t func_8005395C(const int32_t *input, int32_t *output) {
    record(0x8005395C);
    assert(input[0] == 11 && input[1] == 22 && input[2] == 33 && input[3] == 44);
    memcpy(output, conditioned, sizeof(conditioned));
    return 0;
}

static void reset(void) {
    memset(&actor, 0, sizeof(actor));
    memset(&move_info, 0, sizeof(move_info));
    memset(&replacement_info, 0, sizeof(replacement_info));
    memset(calls, 0, sizeof(calls));
    actor.move_info_28 = &move_info;
    actor.component_holder_1C = &holder;
    holder.component_transform_14 = &transform;
    conditioned[0] = 4096;
    conditioned[1] = -4096;
    conditioned[2] = 8192;
    call_count = 0;
    multiply_count = 0;
    mutate_direction = 0;
    mutate_pending_y = 0;
    resume_after_special = 0;
}

static int32_t expected_step(int32_t base, int32_t delta) {
    int64_t magnitude;
    int64_t units;
    int64_t result;
    if (delta >= 0) {
        result = (int64_t)base + delta / 4096;
    } else {
        magnitude = -(int64_t)delta;
        if (magnitude > INT32_MAX) {
            magnitude -= INT64_C(0x100000000);
        }
        units = magnitude >= 0 ? magnitude / 4096 : -((-magnitude + 4095) / 4096);
        result = (int64_t)base - units;
    }
    return (int32_t)(uint32_t)result;
}

static void test_step_boundaries(void) {
    static const int32_t values[] = {
        INT32_MIN, INT32_MIN + 1, -8193, -8192, -4097, -4096, -4095, -1,
        0, 1, 4095, 4096, 4097, 8192, 8193, INT32_MAX - 1, INT32_MAX
    };
    static const uint8_t states[] = {0, 0xDC, 0xDF, 0xF7, 0xE6};
    size_t i, j, k, state;
    unsigned clamp_mode;
    for (i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        for (j = 0; j < sizeof(values) / sizeof(values[0]); ++j) {
            for (k = 0; k < sizeof(values) / sizeof(values[0]); ++k) {
                for (state = 0; state < sizeof(states); ++state) {
                    for (clamp_mode = 0; clamp_mode < 3; ++clamp_mode) {
                        int32_t stored_step = expected_step(values[i], values[j]);
                        int32_t used_step = stored_step;
                        int32_t remaining = values[k];
                        reset();
                        actor.state_00 = states[state];
                        actor.descriptor_0C = clamp_mode ? &transform : NULL;
                        move_info.mode_04 = clamp_mode == 2 ? -2 : 0;
                        move_info.step_30 = values[i];
                        move_info.step_delta_34 = values[j];
                        move_info.clamp_18 = values[k];
                        move_info.direction_88[0] = 4096;
                        move_info.direction_88[1] = -4096;
                        move_info.direction_88[2] = 8192;
                        if (states[state] == 0xDC || states[state] == 0xDF) {
                            used_step = (int32_t)(used_step >= 0
                                ? used_step / 2 : -((-(int64_t)used_step + 1) / 2));
                        }
                        if (states[state] == 0xF7 || states[state] == 0xE6) {
                            func_80022944(&actor);
                            assert(move_info.step_30 == stored_step);
                            assert(move_info.clamp_18 == remaining);
                            assert(call_count == 1 && calls[0] == 0x800227A4);
                            assert(multiply_count == 0);
                            continue;
                        }
                        if (clamp_mode == 1) {
                            remaining = (int32_t)(uint32_t)((int64_t)values[k] - used_step);
                            if (remaining < 0) {
                                used_step = values[k];
                                remaining = 0;
                            }
                        }
                        func_80022944(&actor);
                        assert(move_info.step_30 == stored_step);
                        assert(move_info.clamp_18 == remaining);
                        assert(call_count == 3 && multiply_count == 3);
                        assert(multiply_right[0] == used_step);
                        assert(multiply_right[1] == used_step);
                        assert(multiply_right[2] == used_step);
                        assert(move_info.pending_38[0] == product_q12(4096, used_step));
                        assert(move_info.pending_38[1] == product_q12(-4096, used_step));
                        assert(move_info.pending_38[2] == product_q12(8192, used_step));
                    }
                }
            }
        }
    }
}

static void test_transform_and_mutations(void) {
    unsigned axis;
    reset();
    actor.move_info_28 = NULL;
    func_80022944(&actor);
    assert(call_count == 1 && calls[0] == 0x800556D0);
    assert(actor.move_info_28 == &replacement_info);
    assert(replacement_info.step_30 == 0);

    for (axis = 0; axis < 3; ++axis) {
        reset();
        move_info.direction_88[axis] = 4096;
        move_info.step_30 = 10;
        func_80022944(&actor);
        assert(call_count == 3 && multiply_count == 3);
        assert(move_info.pending_38[axis] == 10);
    }

    reset();
    move_info.step_30 = 10;
    move_info.pending_38[0] = INT32_MAX - 2;
    move_info.pending_38[2] = INT32_MAX - 3;
    func_80022944(&actor);
    assert(call_count == 5 && multiply_count == 3);
    assert(calls[0] == 0x80054AF0 && calls[1] == 0x8005395C);
    assert(move_info.pending_38[0] == INT32_MIN + 7);
    assert(move_info.pending_38[1] == -10);
    assert(move_info.pending_38[2] == INT32_MIN + 16);

    reset();
    move_info.step_30 = 10;
    mutate_pending_y = 1;
    func_80022944(&actor);
    assert(call_count == 4 && multiply_count == 2);
    assert(move_info.pending_38[1] == 77);
    assert(multiply_left[1] == 8192);

    reset();
    move_info.step_30 = 10;
    move_info.pending_38[1] = -9;
    func_80022944(&actor);
    assert(multiply_count == 2 && move_info.pending_38[1] == -9);

    reset();
    move_info.step_30 = 10;
    move_info.direction_88[0] = 4096;
    mutate_direction = 1;
    func_80022944(&actor);
    assert(move_info.pending_38[0] == 10);
    assert(move_info.pending_38[1] == -20);
    assert(move_info.pending_38[2] == 30);

    reset();
    actor.state_00 = 0xF7;
    actor.descriptor_0C = &transform;
    move_info.step_30 = 10;
    move_info.clamp_18 = 1;
    move_info.direction_88[0] = 4096;
    resume_after_special = 1;
    func_80022944(&actor);
    assert(call_count == 4 && multiply_count == 3);
    assert(move_info.step_30 == 999 && move_info.clamp_18 == 1);
    assert(move_info.pending_38[0] == 10);
    assert(replacement_info.pending_38[0] == 0);
}

int main(void) {
    test_step_boundaries();
    test_transform_and_mutations();
    puts("Movement step tests passed (73,695 boundary combinations).");
    return 0;
}
