#include "grounded_case37.h"

extern int16_t g_grounded_case37_counter;
extern GroundedCase37ObjectView *g_grounded_case37_object;
extern uint8_t g_runtime_state_61;
extern int32_t g_level_value_801E8994;
extern GroundedCase37ComponentView *g_level_pointer_801E89A4;
extern void *func_80028764(int32_t unused);
extern void *func_80040FC0(int32_t mode);
extern int32_t func_80054B20(void *component, int32_t mode, int32_t *translation);
extern int32_t func_80054C7C(void *component, int32_t mode, const int32_t *translation);
extern int32_t func_800516F8(void **value_out);
extern int32_t func_800550A4(void *component, const void *basis);
extern void func_800288D8(void *object);
extern void func_80052BDC(void *component, int32_t mode);
extern void func_8002BB64(int32_t value);
extern void func_8001B558(
    int32_t transition_group,
    int32_t transition_id,
    void *argument_2,
    int32_t argument_3,
    int32_t argument_4,
    int32_t argument_5,
    int32_t argument_6,
    int32_t argument_7
);

void func_80037B30(void *initial_object) {
    GroundedCase37ObjectView *current_object;
    GroundedCase37ComponentView *current_component_owner;
    void *current_component;
    void *selected_component;
    int32_t translation[4];
    void *basis;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register GroundedCase37ObjectView *reset_object __asm__("$3");
    register GroundedCase37ObjectView *finish_object __asm__("$4");
    register GroundedCase37ComponentView *component_flags __asm__("$3");
#else
    GroundedCase37ObjectView *reset_object;
    GroundedCase37ObjectView *finish_object;
    GroundedCase37ComponentView *component_flags;
#endif

    current_object = func_80028764(0);
    if (initial_object != 0) {
        g_grounded_case37_counter = 10;
        g_grounded_case37_object = (GroundedCase37ObjectView *)initial_object;
        return;
    }

    if (g_runtime_state_61 != 1 || g_grounded_case37_counter == 0) {
        return;
    }

    if (g_grounded_case37_object->state_2C == 0 ||
        (current_object->flags_04 & 0x4u) != 0 || current_object->state_10 != 0) {
        return;
    }

    if (g_level_value_801E8994 <= 0) {
        current_component_owner = func_80040FC0(0);
    } else {
        current_component_owner = g_level_pointer_801E89A4->owner_1C;
    }
    current_component_owner = current_component_owner->owner_1C;
    current_component = current_component_owner->transform_14;

    selected_component = g_grounded_case37_object->component_1C->transform_14;
    func_80054B20(current_component, 0, translation);
    func_80054C7C(selected_component, 0, translation);
    func_800516F8(&basis);
    func_800550A4(selected_component, basis);
    func_800288D8(g_grounded_case37_object->event_0C);
    func_80052BDC(g_grounded_case37_object->component_1C, 0);

    g_grounded_case37_object->state_2C = 0;
    reset_object = g_grounded_case37_object;
    reset_object->state_01 = 2;
    reset_object->word_28 = 0;
    finish_object = g_grounded_case37_object;
    component_flags = finish_object->component_1C;
    finish_object->state_2E = 0x0D20;
    component_flags->flags_04 &= 0xF7;
    component_flags = g_grounded_case37_object->component_1C;
    component_flags->flags_04 |= 0x04;

    g_grounded_case37_counter--;
    func_8002BB64(0x10);
    func_8001B558(
        0x17,
        5,
        g_grounded_case37_object->event_0C,
        0,
        0,
        0,
        0,
        0
    );
}