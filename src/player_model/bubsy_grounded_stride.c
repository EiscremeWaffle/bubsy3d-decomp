#include "bubsy_grounded_state.h"

extern uint8_t g_stride_mode;
extern uint8_t g_stride_override;
extern uint8_t g_stride_marker;
extern const char g_stride_error_condition[];
extern const char g_stride_source_path[];

extern void func_80052ABC(void *component, int32_t *output);
extern int32_t func_80052AEC(void *component, int32_t sequence);
extern void func_80052E18(void *component);
extern void func_800556D0(int32_t level, const char *condition, const char *source, uint32_t line);
extern void func_80054B20(void *transform, int32_t mode, int32_t *output);
extern void func_8001B558(int32_t state, int32_t mode, int32_t argument_2, int32_t argument_3,
    int32_t *argument_4, int32_t *argument_5, int32_t *argument_6, int32_t *argument_7);
extern void func_80022650(BubsyGroundedActor *actor, int32_t selector);
extern void func_8002262C(BubsyGroundedActor *actor, int32_t selector);
extern void func_80022944(BubsyGroundedActor *actor);

void func_80035ACC(BubsyGroundedActor *actor_input, void *transform_input) {
#if defined(__mips__)
    register BubsyGroundedActor *actor __asm__("$17") = actor_input;
    register BubsyGroundedShared *shared __asm__("$16") = actor->shared_28;
    register void *transform __asm__("$18") = transform_input;
#else
    BubsyGroundedActor *actor = actor_input;
    BubsyGroundedShared *shared = actor->shared_28;
    void *transform = transform_input;
#endif
    int32_t translation[4];
    int32_t sequence;
    int32_t event_value;
    int32_t stride_limit;
    uint32_t override;

    g_stride_marker = 0;
#if defined(__mips__)
    __asm__("" : "=r"(actor), "=r"(shared), "=r"(transform)
        : "0"(actor), "1"(shared), "2"(transform) : "memory");
#endif
    func_80052ABC(actor->component_1C, &sequence);
    if ((g_stride_mode == 0 && shared->stride_30 < 17) ||
        (g_stride_mode == 1 && shared->stride_30 < 12)) {
        if (g_stride_mode == 0 && sequence != 0x13) {
            if (func_80052AEC(actor->component_1C, 0x13) != 0) {
                func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0x536);
            }
            func_80052E18(actor->component_1C);
            func_80054B20(transform, 0, translation);
            event_value = 14;
            func_8001B558(26, 4, 0xFFFE, 0xFFFE, &event_value,
                &translation[0], &translation[1], &translation[2]);
        } else if (g_stride_mode == 1 && sequence != 0) {
            if (func_80052AEC(actor->component_1C, 0) != 0) {
                func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0x542);
            }
            func_80052E18(actor->component_1C);
        }
        if (g_stride_mode == 0) {
            shared->stride_30 = 17;
            shared->stride_delta_34 = 0x147;
            func_80022650(actor, 3);
            func_8002262C(actor, 4);
        } else {
            shared->stride_30 = 12;
            shared->stride_delta_34 = 0x147;
            func_8002262C(actor, 4);
        }
    } else if ((g_stride_mode == 0 && shared->stride_30 < 35) ||
        (g_stride_mode == 1 && shared->stride_30 < 12)) {
        if (g_stride_mode == 0 && sequence != 0x13) {
            if (func_80052AEC(actor->component_1C, 0x13) != 0) {
                func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0x55E);
            }
            func_80052E18(actor->component_1C);
        } else if (g_stride_mode == 1 && sequence != 0) {
            if (func_80052AEC(actor->component_1C, 0) != 0) {
                func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0x565);
            }
            func_80052E18(actor->component_1C);
        }
        if ((((uint8_t *)actor->component_1C)[4] & 0x20) != 0) {
            shared->stride_delta_34 = (int32_t)((uint32_t)shared->stride_delta_34 + 0x147u);
        }
    } else if ((g_stride_mode == 0 && shared->stride_30 >= 35) ||
        (g_stride_mode == 1 && shared->stride_30 >= 12)) {
        func_8002262C(actor, 3);
        func_80022650(actor, 4);
        if (g_stride_mode == 0 && sequence != 0x24) {
            if (func_80052AEC(actor->component_1C, 0x24) != 0) {
                func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0x578);
            }
            func_80052E18(actor->component_1C);
            func_80054B20(transform, 0, translation);
            event_value = 14;
            func_8001B558(26, 4, 0xFFFE, 0xFFFE, &event_value,
                &translation[0], &translation[1], &translation[2]);
        } else if (g_stride_mode == 1 && sequence != 0) {
            if (func_80052AEC(actor->component_1C, 0) != 0) {
                func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0x584);
            }
            func_80052E18(actor->component_1C);
        }
        shared->stride_delta_34 = 0;
        if (g_stride_mode == 0) {
            shared->stride_30 = 35;
        } else {
            shared->stride_30 = 12;
        }
    }

    override = g_stride_override;
    if (override == 1 && g_stride_mode == 0) {
        shared->stride_30 = 35;
        func_80022944(actor);
        return;
    } else if (override == 1) {
        shared->stride_30 = 26;
        func_80022944(actor);
        return;
    } else if (override == 0) {
        /* Keep the original independent mode samples at the cap boundary. */
        if ((*(volatile uint8_t *)&g_stride_mode == 0 && shared->stride_30 > (stride_limit = 35)) ||
            (g_stride_mode == 1 && shared->stride_30 > (stride_limit = 12))) {
            shared->stride_30 = stride_limit;
        }
    }
    func_80022944(actor);
}
