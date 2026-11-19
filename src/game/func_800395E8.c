/* Readable mode-one heading backend. */
#include "camera_slots.h"
#include "component_rotation.h"
#include "game_transition.h"
#include "heading_common_gates.h"
#include "heading_mode_one.h"
#include "mode_camera_helpers.h"
#include "sequence_queries.h"

struct ComponentBasisView;
struct ComponentTransformView;

extern void func_8001AE88(uint32_t *output);
extern uint8_t func_8001BCE4(void);
extern void func_8002CA88(int32_t value, int32_t duration);
extern void func_8002CB2C(int32_t value);
extern int32_t func_80052AEC(void *handle, int32_t state);
extern void func_80052E18(void *handle);
extern int32_t func_80052BDC(void *handle, int32_t mode);
extern int32_t func_80054B20(
    const struct ComponentTransformView *transform,
    const struct ComponentBasisView *basis,
    int32_t output[3]
);
extern int32_t func_80054C7C(
    struct ComponentTransformView *transform,
    const struct ComponentBasisView *basis,
    const int32_t input[3]
);
extern int32_t func_80054AF0(const int16_t *component, int32_t *output);
extern int32_t func_80082ECC(void);
extern void func_80022650(BubsyGroundedActor *actor, int32_t selector);
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

void func_800395E8(BubsyGroundedActor *actor, void *transform) {
    HeadingComponentView *initial_handle = g_mode_one_handle;
    void *mode_transform = initial_handle->transform_14;
    uint32_t now;
    int32_t position[3];
    int32_t vector[3];
    int32_t mode_position[3];
    int32_t first, second, third;
    int32_t initial_budget;
    uint8_t handle_flags;

    func_8001AE88(&now);
    if (func_8001BCE4() != 0) {
        return;
    }

    initial_budget = g_unknown_8009F7BC;
    if (initial_budget >= 1000) {
        func_8002CB2C(15);
        func_8002CA88(15, 999);
    } else {
        if (initial_budget <= 0) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            register int32_t phase_value __asm__("$3") = 4;
            register int32_t *phase __asm__("$5") = &g_interaction_previous_phase;
            __asm__ volatile("" : "=r"(phase_value) : "0"(phase_value));
            __asm__ volatile("" : "=r"(phase) : "0"(phase));
#else
            int32_t phase_value = 4;
            int32_t *phase = &g_interaction_previous_phase;
#endif
            if (*phase != phase_value) {
                *phase = phase_value;
                func_8002CB2C(15);
                func_8001C700(4, 12);
                func_8001B558(27, 4, actor->heading_0C, (int32_t)actor->heading_0C, 0, 0, 0, 0);
                goto update_position;
            }
        }
        if (g_unknown_8009F7BC < 21) {
            if (func_8001C7C4(4, 12) != 0) {
                func_8001C6BC(4, 12, -1);
            }
        } else if (func_8001C7C4(4, 12) == 0) {
            func_8001C700(4, 12);
        }
    }

update_position:
    func_80054B20(transform, 0, position);
    handle_flags = ((HeadingComponentView *)g_mode_one_handle)->unknown_00[4] & 3U;
    if (now - g_heading_mode_previous_tick > g_heading_mode_interval) {
        int32_t extra_y = (int32_t)((uint32_t)position[1] + 0xA0U);
        int32_t next_state;
        g_heading_mode_previous_tick = now;
        g_heading_mode_interval = (uint32_t)((func_80082ECC() % 7) * 60 + 2);
        next_state = (func_80082ECC() & 1) != 0 ? 12 : 13;
        func_8001B558(26, 4, 0xFFFE, 0xFFFE, &next_state, &position[0], &extra_y, &position[2]);
    }

    if (position[1] >= 0x500 && handle_flags != 0) {
        mode_position[0] = position[0];
        mode_position[1] = 0x580;
        mode_position[2] = position[2];
        func_80054C7C(mode_transform, 0, mode_position);
        func_80052AEC(g_mode_one_handle, 0);
        func_80052E18(g_mode_one_handle);
        func_80052BDC(g_mode_one_handle, 0);
    } else if (position[1] > 0x580) {
        position[1] = 0x580;
        func_80054C7C(transform, 0, position);
        func_8001B558(46, 4, actor->heading_0C, (int32_t)actor->heading_0C, 0, 0, 0, 0);
        func_80022650(actor, 20);
        mode_position[0] = position[0];
        mode_position[2] = position[2];
        mode_position[1] = position[1];
        func_80054C7C(mode_transform, 0, mode_position);
        func_80052AEC(g_mode_one_handle, 3);
        func_80052E18(g_mode_one_handle);
        actor->state_00 = 2;
    }

    g_heading_mode_counter += 1U;
    if (g_heading_mode_camera_component == 0) {
        HeadingModeCameraSeed seed = g_heading_mode_camera_seed;
        if (func_80054624(&g_heading_mode_camera_component) != 0) {
            func_800556D0(0, g_heading_mode_allocation_condition, g_heading_mode_source_path, 0xDDC);
        }
        vector[0] = -0x400;
        vector[1] = 0;
        vector[2] = 0;
        func_800547C8(g_heading_mode_camera_component, vector);
        func_80054AF0(g_heading_mode_camera_component, vector);
        func_8004EC20(2, seed.bytes, vector);
        g_heading_mode_direction = 0;
        g_heading_mode_oscillation = 0;
        g_heading_mode_counter = 0;
    } else if (g_heading_mode_counter == 15) {
        g_heading_mode_counter = 0;
        if (g_heading_mode_direction != 0) {
            if (g_heading_mode_oscillation < 91) {
                g_heading_mode_oscillation = (int32_t)((uint32_t)g_heading_mode_oscillation + 1U);
            } else {
                g_heading_mode_direction = 0;
            }
        } else if (g_heading_mode_oscillation < -90) {
            g_heading_mode_direction = 1;
        } else {
            g_heading_mode_oscillation = (int32_t)((uint32_t)g_heading_mode_oscillation - 1U);
        }
        vector[0] = 0;
        vector[1] = g_heading_mode_oscillation;
        vector[2] = 0;
        func_800548A0(g_heading_mode_camera_component, vector);
        func_80054AF0(g_heading_mode_camera_component, vector);
        func_8004EDEC(2, vector);
    }

    func_80028100(&first, &second, &third);
    func_8004EF10(
        (int32_t)((uint32_t)first + (uint32_t)position[1]),
        (int32_t)((uint32_t)second + (uint32_t)position[1]),
        (int32_t)((uint32_t)third + (uint32_t)position[1])
    );
}
