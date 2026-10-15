#include <stdint.h>

typedef struct MoveInfoView {
    int32_t words[0xA4 / sizeof(int32_t)];
} MoveInfoView;

typedef struct ComponentHolderView {
    uint8_t unused_00[0x14];
    void *component_transform;
} ComponentHolderView;

typedef struct ActorView {
    uint8_t unused_00[0x1C];
    ComponentHolderView *component_holder;
    uint8_t unused_20[0x08];
    MoveInfoView *move_info;
} ActorView;

typedef struct MoveAdjustmentScratch {
    int32_t component_translation[3];
    int32_t alignment_word;
    int32_t transformed_translation[3];
} MoveAdjustmentScratch;

extern const char g_move_adjustment_condition[];
extern const char g_game_source_path[];
extern const char g_move_adjustment_step_condition[];
extern int32_t func_800556D0(int32_t failed, const char *condition, const char *source_path, uint32_t line);
extern int32_t func_80054B20(void *component_transform, int32_t mode, int32_t *output);
extern int32_t func_80062E50(const int32_t *current, const int32_t *target, int32_t *remaining);
extern int32_t func_80062D4C(const int32_t *current, const int32_t *target, int32_t *remaining);
extern void func_80010000(const int32_t *left, const int32_t *right, int32_t *output);
extern int32_t func_800539E4(const int32_t *input, int32_t *output);
extern int32_t func_80053A08(const int32_t *input, int32_t scale, int32_t *output);

int32_t func_800227A4(ActorView *actor) {
    register ActorView *actor_value __asm__("$4") = actor;
    register MoveInfoView *move_info __asm__("$17") = actor_value->move_info;
    register int32_t *current_vector __asm__("$16");
    register int32_t remaining_scalar __asm__("$4");
    register int32_t step_assert __asm__("$2");
    register int32_t step __asm__("$3");
    register int32_t threshold __asm__("$2");
    register int32_t remaining_after_distance __asm__("$3");
    register int32_t step_after_distance __asm__("$2");
    MoveAdjustmentScratch scratch;

    if (move_info == 0) {
        return 0;
    }

    if (func_80054B20(actor_value->component_holder->component_transform, 0,
        scratch.component_translation) != 0) {
        return 0;
    }

    if (func_80062E50(scratch.component_translation,
        move_info->words + 0x08 / sizeof(int32_t),
        move_info->words + 0x18 / sizeof(int32_t)) != 0) {
        func_800556D0(0, g_move_adjustment_condition, g_game_source_path, 0x28A);
    }

    step_assert = *(volatile int32_t *)&move_info->words[0x30 / sizeof(int32_t)];
    if (step_assert < 0) {
        func_800556D0(0, g_move_adjustment_step_condition, g_game_source_path, 0x28D);
    }

    remaining_scalar = *(volatile int32_t *)&move_info->words[0x18 / sizeof(int32_t)];
    if (remaining_scalar != 0) {
        step = *(volatile int32_t *)&move_info->words[0x30 / sizeof(int32_t)];
        if (step >= 0) {
            threshold = step + (step >> 1);
            if (remaining_scalar < threshold) {
                goto compare_vectors;
            }
            goto scale_pending_vector;
        }
        threshold = step - ((-step) >> 1);
        if (remaining_scalar >= threshold) {
            goto scale_pending_vector;
        }
    }

compare_vectors:
    current_vector = move_info->words + 0x08 / sizeof(int32_t);
    if (func_80062D4C(scratch.component_translation, current_vector,
        move_info->words + 0x18 / sizeof(int32_t)) != 0) {
        func_800556D0(0, g_move_adjustment_condition, g_game_source_path, 0x297);
    }
    __asm__ volatile("" : : : "memory");
    remaining_after_distance = *(volatile int32_t *)&move_info->words[0x18 / sizeof(int32_t)];
    step_after_distance = *(volatile int32_t *)&move_info->words[0x30 / sizeof(int32_t)];
    if (step_after_distance < remaining_after_distance) {
        goto scale_pending_vector;
    }
    move_info->words[0x18 / sizeof(int32_t)] = 0;
    func_80010000(current_vector, scratch.component_translation,
        move_info->words + 0x38 / sizeof(int32_t));

scale_pending_vector:
    if (move_info->words[0x18 / sizeof(int32_t)] != 0) {
        func_80010000(move_info->words + 0x08 / sizeof(int32_t),
            scratch.component_translation, scratch.transformed_translation);
        func_800539E4(scratch.transformed_translation, move_info->words + 0x88 / sizeof(int32_t));
        func_80053A08(move_info->words + 0x88 / sizeof(int32_t),
            *(volatile int32_t *)&move_info->words[0x30 / sizeof(int32_t)],
            move_info->words + 0x38 / sizeof(int32_t));
    }
    return 1;
}
