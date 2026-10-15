#include <stdint.h>

typedef struct MoveInfoView {
    uint32_t unused_00;
    int32_t mode_04;
    uint32_t unused_08[4];
    int32_t clamp_18;
    uint32_t unused_1C[5];
    int32_t step_30;
    int32_t step_delta_34;
    int32_t pending_38[3];
    uint32_t unused_44[17];
    int32_t direction_88[3];
} MoveInfoView;

typedef struct ActorView {
    uint8_t state_00;
    uint8_t unused_01[11];
    void *descriptor_0C;
    uint8_t unused_10[12];
    void *component_holder_1C;
    uint8_t unused_20[8];
    MoveInfoView *move_info_28;
} ActorView;

typedef struct ComponentHolderView {
    uint8_t unused_00[0x14];
    void *component_transform_14;
} ComponentHolderView;

typedef struct MoveMathScratch {
    int32_t matrix_vector[4];
    int32_t conditioned_vector[4];
    int32_t x_product;
    int32_t padding;
    int32_t z_product;
    int32_t unused;
} MoveMathScratch;

extern const char g_move_info_condition[];
extern const char g_game_source_path[];
extern int32_t func_800556D0(int32_t failed, const char *condition, const char *source_path, uint32_t line);
extern int32_t func_800227A4(ActorView *actor);
extern int32_t func_8001007C(int32_t left, int32_t right, int32_t *output);
extern int32_t func_80054AF0(void *component_transform, int32_t *output);
extern int32_t func_8005395C(const int32_t *input, int32_t *output);

void func_80022944(ActorView *actor) {
    register ActorView *actor_value __asm__("$18") = actor;
    register MoveInfoView *move_info __asm__("$16") = actor_value->move_info_28;
    register int32_t delta __asm__("$2");
    register int32_t base_step __asm__("$3");
    register int32_t computed_step __asm__("$5");
    register int32_t step __asm__("$17");
    register int32_t remaining __asm__("$2");
    register int32_t clamp __asm__("$3");
    register int32_t clear_value __asm__("$2");
    MoveMathScratch scratch;

    if (move_info == 0) {
            register int32_t failed __asm__("$4");
            __asm__ volatile("" : "=r"(failed) : "0"(0));
        func_800556D0(failed, g_move_info_condition, g_game_source_path, 0x2BE);
        return;
    }

    delta = move_info->step_delta_34;
    base_step = move_info->step_30;
    if (delta >= 0) {
        computed_step = base_step + (delta >> 12);
    } else {
        delta = -delta;
        computed_step = base_step - (delta >> 12);
    }
    step = computed_step;
    move_info->step_30 = step;

    if (actor_value->state_00 == 0xDC || actor_value->state_00 == 0xDF) {
        step >>= 1;
    }
    if (actor_value->state_00 == 0xF7 || actor_value->state_00 == 0xE6) {
        func_800227A4(actor_value);
        goto after_clamp;
    }

    if (actor_value->descriptor_0C != 0) {
        if (move_info->mode_04 != -2) {
            clamp = move_info->clamp_18;
            remaining = clamp - step;
            __asm__ volatile("" : "=r"(remaining) : "0"(remaining));
            if (remaining < 0) {
                step = clamp;
                __asm__ volatile("subu $2,$3,$3" : "=r"(remaining) : "r"(clamp));
            }
            move_info->clamp_18 = remaining;
        }
    }

after_clamp:
    if (actor_value->state_00 == 0xF7 || actor_value->state_00 == 0xE6) {
        return;
    }

    if (move_info->direction_88[0] != 0 || move_info->direction_88[1] != 0 ||
        move_info->direction_88[2] != 0) {
        func_8001007C(move_info->direction_88[0], step, &move_info->pending_38[0]);
        func_8001007C(move_info->direction_88[1], step, &move_info->pending_38[1]);
        func_8001007C(move_info->direction_88[2], step, &move_info->pending_38[2]);
        return;
    }

    func_80054AF0(
        ((ComponentHolderView *)actor_value->component_holder_1C)->component_transform_14,
        scratch.matrix_vector
    );
    func_8005395C(scratch.matrix_vector, scratch.conditioned_vector);
    func_8001007C(scratch.conditioned_vector[0], step, &scratch.x_product);
    if (move_info->pending_38[1] == 0) {
        func_8001007C(scratch.conditioned_vector[1], step, &move_info->pending_38[1]);
    }
    func_8001007C(scratch.conditioned_vector[2], step, &scratch.z_product);
    move_info->pending_38[0] += scratch.x_product;
    move_info->pending_38[2] += scratch.z_product;
}
