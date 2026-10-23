#include <stdint.h>

typedef struct GroundedCase37ObjectView {
    uint8_t state_00;
    uint8_t state_01;
    uint8_t padding_02[0x02];
    uint32_t flags_04;
    uint8_t padding_08[0x08];
    uint32_t state_10;
    uint8_t padding_14[0x08];
    void *component_1C;
    uint8_t padding_20[0x08];
    uint32_t word_28;
    uint8_t state_2C;
    uint8_t padding_2D;
    uint16_t state_2E;
} GroundedCase37ObjectView;

extern volatile int16_t g_grounded_case37_counter;
extern GroundedCase37ObjectView * volatile g_grounded_case37_object;
extern volatile uint8_t g_runtime_state_61;
extern volatile int32_t g_level_value_801E8994;
extern void * volatile g_level_pointer_801E89A4;
extern GroundedCase37ObjectView *func_80028764(int32_t unused);
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
    GroundedCase37ObjectView *selected_object;
    void *current_component_owner;
    void *current_component;
    void *selected_component;
    int32_t translation[4];
    void *basis;
    uint8_t *component_flags;

    current_object = func_80028764(0);
    if (initial_object != 0) {
        g_grounded_case37_counter = 10;
        g_grounded_case37_object = (GroundedCase37ObjectView *)initial_object;
        return;
    }

    if (g_runtime_state_61 != 1 || g_grounded_case37_counter == 0) {
        return;
    }

    selected_object = g_grounded_case37_object;
    if (selected_object == 0 || selected_object->state_2C == 0 ||
        (current_object->flags_04 & 0x4u) != 0 || current_object->state_10 != 0) {
        return;
    }

    if (g_level_value_801E8994 > 0) {
        current_component_owner = g_level_pointer_801E89A4;
        current_component_owner = *(void **)((uint8_t *)current_component_owner + 0x1C);
        current_component_owner = *(void **)((uint8_t *)current_component_owner + 0x1C);
        current_component = *(void **)((uint8_t *)current_component_owner + 0x14);
    } else {
        current_component_owner = func_80040FC0(0);
        current_component_owner = *(void **)((uint8_t *)current_component_owner + 0x1C);
        current_component = *(void **)((uint8_t *)current_component_owner + 0x14);
    }

    selected_component = *(void **)((uint8_t *)selected_object->component_1C + 0x14);
    func_80054B20(current_component, 0, translation);
    func_80054C7C(selected_component, 0, translation);
    func_800516F8(&basis);
    func_800550A4(selected_component, basis);
    func_800288D8(*(void **)((uint8_t *)selected_object + 0x0C));
    func_80052BDC(selected_object->component_1C, 0);

    selected_object->state_2C = 0;
    selected_object->state_01 = 2;
    selected_object->word_28 = 0;
    selected_object->state_2E = 0x0D20;
    component_flags = (uint8_t *)selected_object->component_1C + 4;
    *component_flags &= 0xF7;
    *component_flags |= 0x04;

    g_grounded_case37_counter--;
    func_8002BB64(0x10);
    func_8001B558(
        0x17,
        5,
        *(void **)((uint8_t *)selected_object + 0x0C),
        0,
        0,
        0,
        0,
        0
    );
}