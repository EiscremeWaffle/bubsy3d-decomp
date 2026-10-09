#include "player_model.h"

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
typedef struct SharedActorState2Target {
    uint8_t unknown_00_to_07[8];
    uint32_t state_08;
    uint8_t unknown_0C_to_0F[4];
    uint32_t state_10;
    uint8_t unknown_14_to_1B[8];
    void *component_1C;
    void *component_20;
    uint8_t unknown_24_to_27[4];
    void *shared_28;
    uint8_t unknown_2C[1];
    uint8_t state_2D;
} SharedActorState2Target;

_Static_assert(offsetof(SharedActorState2Target, component_20) == 0x20, "component pointer offset must match executable");
_Static_assert(offsetof(SharedActorState2Target, component_1C) == 0x1C, "component alias offset must match executable");
_Static_assert(offsetof(SharedActorState2Target, shared_28) == 0x28, "shared state pointer offset must match executable");
_Static_assert(offsetof(SharedActorState2Target, state_2D) == 0x2D, "state byte offset must match executable");

extern void func_8002CA88(int32_t mode, int32_t duration);
extern int32_t func_800283E0(void);
extern void func_800281D4(void *state_out, void *config_out);
extern void func_8002D5AC(void *config_entry, void *state, int32_t *value_out);
extern void func_80029790(void **model_out);
extern int32_t func_800297A0(int32_t mode, void *state);
extern int32_t func_8005170C(
    void *model,
    int32_t frame_count,
    const void *resource_a,
    const void *resource_b,
    int32_t argument_5,
    int32_t argument_6,
    uint32_t flags,
    void *component
);
extern void func_80053270(void *component);
extern int32_t func_800532CC(void *component, int32_t active, int32_t argument_2, int32_t argument_3, uint32_t flags);
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);
extern void func_80052BDC(void *sequence, int32_t mode);
extern void func_8006B8A4(void *renderer, void *configuration, void *state, void *resource_a, void *resource_b);
extern void func_80054C7C(int32_t component_value, int32_t mode, void *state);
extern void func_80035FE4(void *state);
extern int32_t func_80055280(void *component_value, int32_t mode, void *state);
extern void func_800289E8(void *actor_component, void *state);
extern void func_800549E8(void *component_value, void *state);
extern void func_80054978(void *component_value, void *state);
extern int32_t func_800222EC(void *actor_state, void *state);
extern void func_8003539C(SharedActorState2Target *actor, int32_t mode);

void shared_actor_state_2_handler(
    SharedActorState2Target *actor_argument,
    int32_t *component_value_out,
    void *unused_context
) {
    register SharedActorState2Target *actor __asm__("$18") = actor_argument;
    register int32_t divisor __asm__("$16") = 3;
    register volatile uint8_t *runtime_globals __asm__("$28");
    volatile uint32_t frame_storage[0x98 / sizeof(uint32_t)];
    volatile uint32_t *shared_words = (volatile uint32_t *)actor->shared_28;
    volatile uint8_t *runtime_state = (volatile uint8_t *)0x80186450u;
    const uint32_t *initial_config = (const uint32_t *)0x800129B8u;
    volatile uint8_t *level_config = *(volatile uint8_t * volatile *)0x800BE698u;
    uint8_t runtime_mode;
    void *model;
    int32_t loop_index;

    (void)unused_context;
    frame_storage[(0x60 - 0x20) / 4] = initial_config[0];
    frame_storage[(0x64 - 0x20) / 4] = initial_config[1];
    frame_storage[(0x68 - 0x20) / 4] = initial_config[2];
    frame_storage[(0x6C - 0x20) / 4] = initial_config[3];
    runtime_mode = (uint8_t)func_800283E0();
    runtime_globals[0x7EC] = runtime_mode;
    func_800281D4((void *)(frame_storage + (0x40 - 0x20) / 4), (void *)(frame_storage + (0x50 - 0x20) / 4));
    if (level_config[0x2A28] == 3) {
        func_8002D5AC(level_config + 9, (void *)(frame_storage + (0x40 - 0x20) / 4), (int32_t *)(frame_storage + (0x64 - 0x20) / 4));
        frame_storage[(0x64 - 0x20) / 4] = (uint32_t)-(int32_t)frame_storage[(0x64 - 0x20) / 4];
    }
    model = (void *)(frame_storage + (0x70 - 0x20) / 4);
    func_80029790((void **)&model);
    if (func_8005170C(
            model,
            0x14,
            runtime_mode == 0 ? (const void *)0x8009F98Cu : (const void *)0x800A091Cu,
            runtime_mode == 0 ? (const void *)0x8009F90Cu : (const void *)0x8009F950u,
            0,
            1,
            0x00800001u,
            (uint8_t *)actor + 0x20) != 0) {
        func_800556D0(1, (const char *)0x80012974u, (const char *)0x8001298Cu, 0x781);
    }
    *(volatile uint16_t *)((uint8_t *)actor->component_20 + 0x1C) =
        (uint16_t)(*(volatile uint16_t *)((uint8_t *)actor + 0x0C) + 0x2000);
    func_80053270(actor->component_20);
    func_800532CC(actor->component_20, 1, 0, 0, 0);

    {
        const uint32_t *second_config = (const uint32_t *)0x800129C8u;
        frame_storage[(0x78 - 0x20) / 4] = second_config[0];
        frame_storage[(0x7C - 0x20) / 4] = second_config[1];
        frame_storage[(0x80 - 0x20) / 4] = second_config[2];
        frame_storage[(0x84 - 0x20) / 4] = second_config[3];
        frame_storage[(0x88 - 0x20) / 4] = initial_config[0];
        frame_storage[(0x8C - 0x20) / 4] = initial_config[1];
        frame_storage[(0x90 - 0x20) / 4] = initial_config[2];
        frame_storage[(0x94 - 0x20) / 4] = initial_config[3];
        if ((uint8_t)func_800297A0(7, model) != 1) {
            func_800556D0(1, (const char *)0x800129D8u, (const char *)0x8001298Cu, 0x78E);
        }
        if (func_8005170C(model, 0x14, (const void *)0x800A10C4u, 0, 0, 1, 0,
                (void *)0x800CE68Cu) != 0) {
            func_800556D0(1, (const char *)0x80012974u, (const char *)0x8001298Cu, 0x798);
        }
        if ((uint8_t)func_800297A0(8, model) != 1) {
            func_800556D0(1, (const char *)0x800129D8u, (const char *)0x8001298Cu, 0x79B);
        }
        if (func_8005170C(model, 0x14, (const void *)0x800A10C4u, 0, 0, 1, 0,
                (void *)0x800CE690u) != 0) {
            func_800556D0(1, (const char *)0x80012974u, (const char *)0x8001298Cu, 0x7A5);
        }
        if (level_config[0x45] != 0) {
            if ((uint8_t)func_800297A0(0xB, model) != 1) {
                func_800556D0(1, (const char *)0x800129D8u, (const char *)0x8001298Cu, 0x7B7);
            }
            if (func_8005170C(model, 1, (const void *)0x800A10C4u, 0, 0, 1, 0,
                    (void *)0x800CE0C4u) != 0) {
                func_800556D0(1, (const char *)0x800129F4u, (const char *)0x8001298Cu, 0x7C1);
            }
        }
        if ((uint8_t)func_800297A0(0, model) != 1) {
            func_800556D0(1, (const char *)0x800129D8u, (const char *)0x8001298Cu, 0x7C6);
        }
        for (loop_index = 0; loop_index <= 0; loop_index++) {
            uint32_t *item = (uint32_t *)(0x800CE550u + loop_index * 4);
            if (func_8005170C(model, 0x14, (const void *)0x800A10C4u, 0, 0, 0, 0, item) != 0) {
                func_800556D0(1, (const char *)0x80012974u, (const char *)0x8001298Cu, 0x7D2);
            }
            func_80052BDC((void *)(uintptr_t)*item, 0);
        }
    }

    runtime_mode = runtime_globals[0x7EC];
    if (runtime_mode == 1) {
        const uint32_t *mode_config = (const uint32_t *)0x80012A10u;
        void *mode_resource = *(void * volatile *)(runtime_globals + 0x7FC);
        frame_storage[(0x98 - 0x20) / 4] = mode_config[0];
        frame_storage[(0x9C - 0x20) / 4] = mode_config[1];
        frame_storage[(0xA0 - 0x20) / 4] = mode_config[2];
        frame_storage[(0xA4 - 0x20) / 4] = mode_config[3];
        frame_storage[(0xA8 - 0x20) / 4] = initial_config[0];
        frame_storage[(0xAC - 0x20) / 4] = initial_config[1];
        frame_storage[(0xB0 - 0x20) / 4] = initial_config[2];
        frame_storage[(0xB4 - 0x20) / 4] = initial_config[3];
        if ((uint8_t)func_800297A0(0xD, model) != runtime_mode) {
            func_800556D0(1, (const char *)0x800129D8u, (const char *)0x8001298Cu, 0x7DD);
        }
        if (func_8005170C(model, 0x14, (const void *)0x800A1010u, 0, 0, 1, 1,
                (void *)0x800CE558u) != 0) {
            func_800556D0(1, (const char *)0x80012974u, (const char *)0x8001298Cu, 0x7E7);
        }
        func_8006B8A4((void *)0x8007ABFCu, mode_resource, (void *)0x801863E4u,
            (void *)(frame_storage + (0x98 - 0x20) / 4), (void *)(frame_storage + (0xA8 - 0x20) / 4));
        func_80052BDC(mode_resource, 1);
    }

    actor->component_1C = actor->component_20;
    *component_value_out = *(volatile int32_t *)((uint8_t *)actor->component_20 + 0x14);
    func_80054C7C(*component_value_out, 0, (void *)(frame_storage + (0x40 - 0x20) / 4));
    func_80035FE4((void *)(frame_storage + (0x40 - 0x20) / 4));
    {
        const uint16_t state = *(volatile uint16_t *)0x801D36F0u;
        if (state == 0x13 || state == 0x14) {
            if (func_80055280((void *)(uintptr_t)*component_value_out, 0,
                    (void *)(frame_storage + (0x50 - 0x20) / 4)) != 0) {
                func_800556D0(1, (const char *)0x80012974u, (const char *)0x8001298Cu, 0x801);
            }
        } else {
            func_800289E8((uint8_t *)actor + 0x0C,
                (void *)(frame_storage + (0x20 - 0x20) / 4));
            func_800549E8((void *)(uintptr_t)*component_value_out,
                (void *)(frame_storage + (0x20 - 0x20) / 4));
        }
    }
    if (level_config[0x2A28] == 3) {
        func_80054978((void *)(uintptr_t)*component_value_out,
            (void *)(frame_storage + (0x60 - 0x20) / 4));
        if (func_800222EC((uint8_t *)actor + 0x28,
                (void *)(frame_storage + (0x40 - 0x20) / 4)) != 1) {
            func_800556D0(1, (const char *)0x800129D8u, (const char *)0x8001298Cu, 0x810);
        }
    }

    actor->state_08 = 0;
    actor->state_10 = 0;
    shared_words[1] = (uint32_t)-2;
    shared_words[0x80 / 4] = 0;
    shared_words[0x7C / 4] = 0;
    shared_words[0x78 / 4] = 0;
    shared_words[0x30 / 4] = 0;
    shared_words[0x34 / 4] = 0;
    shared_words[0x40 / 4] = 0;
    shared_words[0x3C / 4] = 0;
    shared_words[0x38 / 4] = 0;
    shared_words[0x50 / 4] = 0;
    shared_words[0x4C / 4] = 0;
    shared_words[0x48 / 4] = 0;

    *(volatile int32_t *)0x80186458u = -1;
    *(volatile uint16_t *)(runtime_state + 0) = 0;
    *(volatile uint16_t *)(runtime_state + 2) = 0;
    runtime_state[0x10] = 0;
    runtime_state[0x12] = 0;
    runtime_state[0x13] = 0;
    runtime_state[0x11] = 0;
    runtime_state[0x29] = 0;
    runtime_state[0x2A] = 0;
    runtime_state[0x4D] = 0;
    runtime_state[0x68] = 0;
    runtime_state[0x6D] = 0;

    __asm__ volatile ("" : "=r"(divisor) : "0"(divisor));
    if (runtime_mode == 0) {
        const int32_t numerator = -0xCCC0;
        *(volatile int32_t *)(runtime_state + 0x50) = numerator;
        *(volatile int32_t *)(runtime_state + 0x54) = numerator / divisor;
        *(volatile int32_t *)(runtime_state + 0x58) = -0x4CC0;
        runtime_state[0x5C] = 3;
        *(volatile int32_t *)(runtime_state + 0x60) = (int32_t)0xFFD00300u;
        *(volatile int32_t *)(runtime_state + 0x64) = (int32_t)0xFFF00100u;
    } else if (runtime_mode == 1) {
        const int32_t numerator = -0xF40;
        *(volatile int32_t *)(runtime_state + 0x50) = numerator;
        *(volatile int32_t *)(runtime_state + 0x54) = numerator / divisor;
        runtime_state[0x5C] = 3;
        *(volatile int32_t *)(runtime_state + 0x60) = -0x1E80;
        *(volatile int32_t *)(runtime_state + 0x58) = 0;
        *(volatile int32_t *)(runtime_state + 0x64) = -0xA2A;
        func_8002CA88(0xF, 0xB4);
    }

    actor->state_2D = 0x41;
    func_8003539C(actor, 1);
    runtime_globals[0x36C] = 1;
}
#endif