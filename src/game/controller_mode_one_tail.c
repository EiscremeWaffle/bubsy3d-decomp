/* Bounded host models of mode-one callback branches, not byte matches. */
#include "controller_mode_one_tail.h"
#include "texture_font_slots.h"

extern void func_800486D0(void *text, int32_t x, int32_t y, int8_t *tag);
extern uint8_t func_80023A30(int32_t interval);
extern void func_80023C98(void);
extern int32_t func_80048504(int32_t tag);
extern int32_t func_80048C48(int32_t tag);
extern void func_8001BC08(void);
extern void func_8001BC28(void);
extern void func_8002C9D4(int32_t value);
extern void func_8002C370(void);
extern int32_t func_800481D4(int32_t tag, int32_t first, int32_t second);
extern void func_80027F40(void);
extern int32_t func_80061A64(int32_t value);
extern void func_8001B558(
    int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
);
extern void func_8006BB84(int32_t value);
extern void func_8004F688(void);
extern void func_80027114(int32_t first, int32_t second);
extern int32_t func_80047050(int32_t value);
extern int32_t func_80062890(void);
extern volatile uint8_t g_controller_timeout_ticks;
extern int32_t g_unknown_800BDF10;
extern int32_t g_unknown_800BDF14;
extern volatile uint8_t g_unknown_800BE680;

const char g_controller_button_prompt_text[] = "prompt";
const char g_controller_display_text_b[] = "display-b";
const char g_controller_display_text_c[] = "display-c";
static const char g_controller_state_three_text_a[] = "state-three-a";
static const char g_controller_state_three_text_b[] = "state-three-b";

static void controller_mode_one_state_three_setup(ControllerGameContext *context) {
    int32_t event_data[2];
    int index;

    func_80048504(g_controller_prompt_tag);
    func_80048C48(g_controller_prompt_tag);
    func_80027F40();
    context->blocked_2b = 1;
    event_data[0] = g_unknown_800BDF10;
    event_data[1] = g_unknown_800BDF14;
    func_80061A64(30);
    g_unknown_800BE680 = 0;
    func_8001B558(4, 5, 0xFFFF, 0xFFFF, event_data, 0, 0, 0);
    for (index = 0; index < 4; index++) {
        func_8006BB84(0);
        func_8004F688();
    }

    g_controller_runtime_flags->flash_byte_0052 = 255;
    func_80027114(1, 1);
    g_controller_pulse_value = 128;
    g_controller_pulse_enabled = 1;
    g_controller_pulse_selection = 1;
    g_controller_pulse_direction = 1;
    context->mode_6d = 4;
    func_800486D0((void *)g_controller_state_three_text_a, 305, 35, &g_controller_pulse_tag_a);
    func_800486D0((void *)g_controller_state_three_text_b, 345, 60, &g_controller_pulse_tag_b);
    func_800483C0(g_controller_pulse_tag_a);
    func_800483C0(g_controller_pulse_tag_b);
}

ControllerModeOneTailResult controller_mode_one_tail(ControllerGameContext *context, uint8_t mode) {
    uint8_t state;
    uint8_t value;
    uint8_t color[3];
    int32_t tag;

    if (mode != 1) {
        return CONTROLLER_MODE_ONE_COMMON_GATE;
    }
    state = context->mode_6d;
    if (state == 2) {
        return CONTROLLER_MODE_ONE_COMMON_GATE;
    }
    if (state == 1) {
        func_800486D0(g_controller_prompt_text, 180, 100, &g_controller_prompt_tag);
        func_800483C0(g_controller_prompt_tag);
        context->mode_6d = 2;
    } else if (state == 3) {
        controller_mode_one_state_three_setup(context);
    }
    if (g_controller_pulse_enabled == 0) {
        return CONTROLLER_MODE_ONE_EXIT;
    }
    value = g_controller_pulse_value;
    if (value < 33) {
        g_controller_pulse_value = 32;
        g_controller_pulse_direction = 1;
    } else if (value >= 128) {
        g_controller_pulse_value = 128;
        g_controller_pulse_direction = 0;
    }
    value = g_controller_pulse_value;
    if (g_controller_pulse_direction == 0) {
        value = (uint8_t)(value - 8U);
    } else {
        value = (uint8_t)(value + 8U);
    }
    g_controller_pulse_value = value;
    value = g_controller_pulse_value;
    color[0] = value;
    color[1] = value;
    color[2] = value;
    tag = g_controller_pulse_selection != 0 ? g_controller_pulse_tag_a : g_controller_pulse_tag_b;
    func_800482D0(tag, color);
    return CONTROLLER_MODE_ONE_EXIT;
}

ControllerModeOneTailResult controller_mode_one_button_color(uint32_t buttons) {
    uint8_t color[3];
    int32_t tag;

    if (g_controller_pulse_enabled == 0
        || (buttons & ((1U << 14) | (1U << 12))) == 0) {
        return CONTROLLER_MODE_ONE_BUTTON_UNMODELED;
    }

    g_controller_pulse_value = 128;
    color[0] = 128;
    color[1] = 128;
    color[2] = 128;
    if ((buttons & (1U << 12)) != 0) {
        tag = g_controller_pulse_tag_a;
        func_800482D0(tag, color);
        g_controller_pulse_selection = 1;
    } else {
        tag = g_controller_pulse_tag_b;
        func_800482D0(tag, color);
        g_controller_pulse_selection = 0;
    }
    return CONTROLLER_MODE_ONE_EXIT;
}

ControllerModeOneTailResult controller_mode_one_enabled_pulse_button(
    uint32_t buttons,
    uint8_t mode,
    ControllerGameContext *context
) {
    if (g_controller_pulse_enabled == 0
        || (buttons & ((1U << 14) | (1U << 12))) != 0) {
        return CONTROLLER_MODE_ONE_BUTTON_UNMODELED;
    }

    if ((buttons & ((1U << 6) | (1U << 11))) == 0) {
        return CONTROLLER_MODE_ONE_EXIT;
    }

    if (func_80023A30(30) == 0) {
        return CONTROLLER_MODE_ONE_EXIT;
    }

    if (context->mode_6d != 0) {
        int32_t event_data[2] = {0x200, 0x1E0};

        func_80062890();
        func_8001B558(5, 5, 0xFFFF, 0xFFFF, &event_data[0], &event_data[1], 0, 0);
        if (g_controller_pulse_selection != 0) {
            g_controller_lifecycle_state = 3;
            func_80047050(0);
            func_8001B558(1, 5, 0xFFFF, 0xFFFF, 0, 0, 0, 0);
            return CONTROLLER_MODE_ONE_EXIT;
        }

        context->mode_6d = 0;
        g_controller_lifecycle_state = 2;
        func_80047050(0);
        func_8001B558(1, 5, 0xFFFF, 0xFFFF, 0, 0, 0, 0);
    }

    if (g_controller_pulse_selection != 0) {
        g_controller_lifecycle_state = mode;
        func_80047050(1);
        func_8001B558(1, 5, 0xFFFF, 0xFFFF, 0, 0, 0, 0);
        return CONTROLLER_MODE_ONE_EXIT;
    }

    g_controller_timeout_ticks = 20;
    func_80023C98();
    g_controller_flash_enabled = 0;
    g_controller_pulse_enabled = 0;
    func_8001BC28();
    func_8002C9D4(0);
    func_80027114(0, 1);
    func_80048504(g_controller_prompt_tag);
    func_80048C48(g_controller_prompt_tag);
    func_80048504(g_controller_pulse_tag_a);
    func_80048C48(g_controller_pulse_tag_a);
    func_80048504(g_controller_pulse_tag_b);
    func_80048C48(g_controller_pulse_tag_b);
    func_8002C370();
    return CONTROLLER_MODE_ONE_EXIT;
}

ControllerModeOneTailResult controller_mode_one_disabled_pulse_button(uint32_t buttons) {
    if (g_controller_pulse_enabled != 0) {
        return CONTROLLER_MODE_ONE_BUTTON_UNMODELED;
    }

    if ((buttons & (1U << 11)) != 0) {
        if (func_80023A30(30) != 0) {
            g_controller_flash_enabled = 0;
            func_80023C98();
        }
        return CONTROLLER_MODE_ONE_EXIT;
    }

    if ((buttons & (1U << 8)) == 0 || g_controller_flash_enabled == 0) {
        return CONTROLLER_MODE_ONE_EXIT;
    }

    if (func_80023A30(30) == 0) {
        return CONTROLLER_MODE_ONE_EXIT;
    }

    g_controller_runtime_flags->flash_byte_0052 = 255;
    func_80048504(g_controller_button_tag);
    func_80048C48(g_controller_button_tag);
    func_8001BC08();
    func_8002C9D4(1);
    g_controller_pulse_enabled = 1;
    g_controller_pulse_selection = 0;
    g_controller_pulse_value = 128;
    g_controller_pulse_direction = 1;
    func_800486D0((void *)g_controller_button_prompt_text, 160, 180, &g_controller_prompt_tag);
    func_800481D4(g_controller_prompt_tag, 0x1000, 0x1000);
    func_800483C0(g_controller_prompt_tag);
    func_800486D0((void *)g_controller_display_text_b, 240, 230, &g_controller_pulse_tag_a);
    func_800486D0((void *)g_controller_display_text_c, 240, 275, &g_controller_pulse_tag_b);
    func_800481D4(g_controller_pulse_tag_a, 0x1000, 0x1000);
    func_800481D4(g_controller_pulse_tag_b, 0x1000, 0x1000);
    func_800483C0(g_controller_pulse_tag_a);
    func_800483C0(g_controller_pulse_tag_b);
    return CONTROLLER_MODE_ONE_EXIT;
}
