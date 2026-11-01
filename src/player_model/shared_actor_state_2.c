#include <stdint.h>

/*
 * L0 shared actor state 2 handler (0x8003697C..0x80037214), from ../f/bubsy.c.
 * Loads the actor's model resources, attaches its component transform, and
 * resets the shared movement state.  Gameplay meanings remain provisional.
 */

typedef struct Vector {
    int32_t vx;
    int32_t vy;
    int32_t vz;
    int32_t pad;
} Vector;

typedef struct Matrix {
    int16_t m[3][3];
    int32_t t[3];
} Matrix;

typedef union ActorHeading {
    uint32_t packed;
    struct {
        uint16_t yaw;
        uint16_t pitch;
    } angle;
} ActorHeading;

typedef struct ActorComponent {
    uint8_t unknown_00[0x14];
    void *transform_14;
    uint8_t unknown_18[4];
    uint16_t yaw_1C;
} ActorComponent;

typedef struct ActorSharedState {
    int32_t unknown_00;
    int32_t state_04;
    uint8_t unknown_08[0x28];
    int32_t progress_30;
    int32_t progress_34;
    Vector motion_38;
    Vector motion_48;
    uint8_t unknown_58[0x20];
    Vector motion_78;
} ActorSharedState;

typedef struct SharedActorState2Actor {
    uint8_t unknown_00[8];
    int32_t state_08;
    ActorHeading heading_0C;
    int32_t state_10;
    uint8_t unknown_14[8];
    ActorComponent *component_1C;
    ActorComponent *component_20;
    uint8_t unknown_24[4];
    ActorSharedState *shared_28;
    uint8_t unknown_2C;
    uint8_t state_2D;
} SharedActorState2Actor;

typedef struct LevelConfig {
    uint8_t unknown_00[9];
    uint8_t spawn_09;
    uint8_t unknown_0A[0x3B];
    uint8_t has_extra_model_45;
    uint8_t unknown_46[0x29E2];
    uint8_t mode_2A28;
} LevelConfig;

typedef struct RenderSlot {
    uint8_t data[0x5C];
} RenderSlot;

typedef struct PlayerControlState {
    uint16_t unknown_00;
    uint16_t unknown_02;
    uint8_t unknown_04[4];
    int32_t target_08;
    uint8_t unknown_0C[4];
    uint8_t flag_10;
    uint8_t flag_11;
    uint8_t flag_12;
    uint8_t flag_13;
    uint8_t unknown_14[0x15];
    uint8_t flag_29;
    uint8_t flag_2A;
    uint8_t unknown_2B[0x22];
    uint8_t flag_4D;
    uint8_t unknown_4E[2];
    int32_t limit_50;
    int32_t limit_54;
    int32_t limit_58;
    uint8_t limit_count_5C;
    uint8_t unknown_5D[3];
    int32_t limit_60;
    int32_t limit_64;
    uint8_t flag_68;
    uint8_t unknown_69[4];
    uint8_t flag_6D;
} PlayerControlState;

extern LevelConfig *g_level_config;
extern PlayerControlState g_player_control;
extern uint16_t g_game_phase;
extern void *g_actor_handles[2];
extern void *g_extra_model_handle;
extern void *g_effect_handles[1];
extern void *g_mode_one_handle;
extern uint8_t g_runtime_mode;
extern uint8_t g_player_ready;
extern RenderSlot g_render_slots[2];
extern RenderSlot g_mode_one_render_slot;
extern uint8_t g_renderer[];
extern uint8_t g_mode_zero_resource_a[];
extern uint8_t g_mode_zero_resource_b[];
extern uint8_t g_mode_other_resource_a[];
extern uint8_t g_mode_other_resource_b[];
extern uint8_t g_shared_model_resource[];
extern uint8_t g_mode_one_model_resource[];
extern const Vector g_state2_zero_vector;
extern const Vector g_state2_render_offset;
extern const Vector g_state2_mode_one_offset;
extern const char g_bubsy_error_none[];
extern const char g_bubsy_bool_true[];
extern const char g_bubsy_bool_error_none[];
extern const char g_bubsy_source_path[];

extern void func_800556D0(int32_t level, const char *condition, const char *source_path, uint32_t line);
extern uint8_t func_800283E0(void);
extern void func_800281D4(Vector *position, Vector *rotation);
extern void func_8002D5AC(uint8_t *spawn, Vector *position, int32_t *height);
extern void func_80029790(void **model);
extern uint8_t func_800297A0(int32_t slot, void **model);
extern int32_t func_8005170C(void *model, int32_t frames, void *resource_a, void *resource_b,
    int32_t argument_4, int32_t argument_5, uint32_t flags, void *handle_out);
extern void func_80053270(ActorComponent *component);
extern void func_800532CC(ActorComponent *component, int32_t active, int32_t argument_2, int32_t argument_3, int32_t argument_4);
extern void func_8006B8A4(void *renderer, void *handle, RenderSlot *slot, Vector *offset, Vector *scale);
extern void func_80052BDC(void *handle, int32_t mode);
extern void func_80054C7C(void *transform, int32_t mode, Vector *translation);
extern void func_80035FE4(Vector *position);
extern int32_t func_80055280(void *transform, int32_t mode, Vector *rotation);
extern void func_800289E8(uint32_t heading, Matrix *rotation);
extern void func_800549E8(void *transform, Matrix *rotation);
extern void func_80054978(void *transform, Vector *offset);
extern uint8_t func_800222EC(ActorSharedState **shared, Vector *position);
extern void func_8002CA88(int32_t mode, int32_t duration);
extern void func_8003539C(SharedActorState2Actor *actor, int32_t mode);

#define EGSErrorNone 0
#define EGSBoolTrue 1

#define GS_ASSERT(level, condition, text, line) \
    if (!(condition)) func_800556D0(level, text, g_bubsy_source_path, line)

void shared_actor_state_2_handler(SharedActorState2Actor *actor, void **transform_out)
{
    Matrix rotation;
    Vector position;
    Vector facing;
    Vector offset = g_state2_zero_vector;
    void *model;
    int32_t error;
    uint8_t boolError;
    int32_t index;
    ActorComponent *component;
    ActorSharedState *shared;
    int32_t limit_count;

    g_runtime_mode = func_800283E0();
    func_800281D4(&position, &facing);
    if (g_level_config->mode_2A28 == 3) {
        func_8002D5AC(&g_level_config->spawn_09, &position, &offset.vy);
        offset.vy = -offset.vy;
    }
    func_80029790(&model);
    if (g_runtime_mode == 0) {
        error = func_8005170C(model, 20, g_mode_zero_resource_a, g_mode_zero_resource_b,
            0, 1, 0x800001, &actor->component_20);
    } else {
        error = func_8005170C(model, 20, g_mode_other_resource_a, g_mode_other_resource_b,
            0, 1, 0x800001, &actor->component_20);
    }
    GS_ASSERT(1, error == EGSErrorNone, g_bubsy_error_none, 0x781);
    actor->component_20->yaw_1C = actor->heading_0C.angle.yaw + 0x2000;
    func_80053270(actor->component_20);
    func_800532CC(actor->component_20, 1, 0, 0, 0);

    {
        /* The original keeps &g_actor_handles in s4 across the asserts. */
        register void **handles __asm__("$20");
        void *handle;
        Vector render_offset = g_state2_render_offset;
        Vector render_scale = g_state2_zero_vector;

        boolError = func_800297A0(7, &model);
        GS_ASSERT(1, boolError == EGSBoolTrue, g_bubsy_bool_true, 0x78E);
        __asm__("" : "=r"(handles) : "0"(g_actor_handles));
        error = func_8005170C(model, 20, g_shared_model_resource, 0, 0, 1, 0, &handles[0]);
        GS_ASSERT(1, error == EGSErrorNone, g_bubsy_error_none, 0x798);
        boolError = func_800297A0(8, &model);
        GS_ASSERT(1, boolError == EGSBoolTrue, g_bubsy_bool_true, 0x79B);
        error = func_8005170C(model, 20, g_shared_model_resource, 0, 0, 1, 0, &handles[1]);
        GS_ASSERT(1, error == EGSErrorNone, g_bubsy_error_none, 0x7A5);
        handle = handles[0];
        func_8006B8A4(g_renderer, handle, &g_render_slots[0], &render_offset, &render_scale);
        handle = handles[1];
        func_8006B8A4(g_renderer, handle, &g_render_slots[1], &render_offset, &render_scale);
    }

    if (g_level_config->has_extra_model_45) {
        boolError = func_800297A0(11, &model);
        GS_ASSERT(1, boolError == EGSBoolTrue, g_bubsy_bool_true, 0x7B7);
        boolError = func_8005170C(model, 1, g_shared_model_resource, 0, 0, 1, 0, &g_extra_model_handle);
        GS_ASSERT(1, boolError == EGSErrorNone, g_bubsy_bool_error_none, 0x7C1);
    }

    boolError = func_800297A0(0, &model);
    GS_ASSERT(1, boolError == EGSBoolTrue, g_bubsy_bool_true, 0x7C6);
    for (index = 0; index < 1; index++) {
        error = func_8005170C(model, 20, g_shared_model_resource, 0, 0, 0, 0, &g_effect_handles[index]);
        GS_ASSERT(1, error == EGSErrorNone, g_bubsy_error_none, 0x7D2);
        func_80052BDC(g_effect_handles[index], 0);
    }

    if (g_runtime_mode == 1) {
        Vector mode_offset = g_state2_mode_one_offset;
        Vector mode_scale = g_state2_zero_vector;

        boolError = func_800297A0(13, &model);
        GS_ASSERT(1, boolError == EGSBoolTrue, g_bubsy_bool_true, 0x7DD);
        error = func_8005170C(model, 20, g_mode_one_model_resource, 0, 0, 1, 1, &g_mode_one_handle);
        GS_ASSERT(1, error == EGSErrorNone, g_bubsy_error_none, 0x7E7);
        func_8006B8A4(g_renderer, g_mode_one_handle, &g_mode_one_render_slot, &mode_offset, &mode_scale);
        func_80052BDC(g_mode_one_handle, 1);
    }

    component = actor->component_20;
    actor->component_1C = component;
    *transform_out = component->transform_14;
    func_80054C7C(*transform_out, 0, &position);
    func_80035FE4(&position);
    if (g_game_phase == 0x13 || g_game_phase == 0x14) {
        error = func_80055280(*transform_out, 0, &facing);
        GS_ASSERT(0, error == EGSErrorNone, g_bubsy_error_none, 0x801);
    } else {
        func_800289E8(actor->heading_0C.packed, &rotation);
        func_800549E8(*transform_out, &rotation);
    }
    limit_count = 3;
    if (g_level_config->mode_2A28 == limit_count) {
        func_80054978(*transform_out, &offset);
    }
    boolError = func_800222EC(&actor->shared_28, &position);
    GS_ASSERT(1, boolError == EGSBoolTrue, g_bubsy_bool_true, 0x810);

    shared = actor->shared_28;
    actor->state_08 = 0;
    actor->state_10 = 0;
    shared->state_04 = -2;
    shared->motion_78.vx = shared->motion_78.vy = shared->motion_78.vz = 0;
    shared->progress_30 = 0;
    shared->progress_34 = 0;
    shared->motion_38.vx = shared->motion_38.vy = shared->motion_38.vz = 0;
    shared->motion_48.vx = shared->motion_48.vy = shared->motion_48.vz = 0;

    g_player_control.target_08 = -1;
    g_player_control.unknown_00 = 0;
    g_player_control.unknown_02 = 0;
    g_player_control.flag_10 = 0;
    g_player_control.flag_12 = 0;
    g_player_control.flag_13 = 0;
    g_player_control.flag_11 = 0;
    g_player_control.flag_29 = 0;
    g_player_control.flag_2A = 0;
    g_player_control.flag_4D = 0;
    g_player_control.flag_68 = 0;
    g_player_control.flag_6D = 0;

    if (g_runtime_mode == 0) {
        g_player_control.limit_50 = -0xCCC0;
        g_player_control.limit_58 = -0x4CC0;
        g_player_control.limit_count_5C = 3;
        g_player_control.limit_54 = g_player_control.limit_50 / limit_count;
        g_player_control.limit_60 = (int32_t)0xFFD00300;
        g_player_control.limit_64 = (int32_t)0xFFF00100;
    } else if (g_runtime_mode == 1) {
        g_player_control.limit_50 = -0xF40;
        g_player_control.limit_count_5C = 3;
        g_player_control.limit_54 = g_player_control.limit_50 / limit_count;
        g_player_control.limit_60 = -0x1E80;
        g_player_control.limit_58 = 0;
        g_player_control.limit_64 = -0xA2A;
        func_8002CA88(15, 180);
    }

    actor->state_2D = 0x41;
    func_8003539C(actor, 1);
    g_player_ready = 1;
}
