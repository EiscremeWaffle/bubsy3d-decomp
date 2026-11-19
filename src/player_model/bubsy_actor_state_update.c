#include "bubsy_actor_state_update.h"
#include "bubsy_grounded_state.h"
#include "game_transition.h"

typedef struct BubsyActorMessage {
    char text[0x80];
    int32_t value_80;
    int32_t value_84;
    int32_t value_88;
    int32_t value_8C;
    int32_t value_90;
    uint8_t value_94;
    uint8_t value_95;
    uint8_t value_96;
    uint8_t unknown_97;
    uint8_t value_98;
    uint8_t unknown_99[3];
    int32_t value_9C;
    int32_t value_A0;
    uint8_t unknown_A4;
    uint8_t value_A5;
    uint8_t value_A6;
    uint8_t unknown_A7[9];
} BubsyActorMessage;

extern int16_t g_actor_update_phase;
extern int32_t g_actor_update_counter;
extern int32_t g_actor_message_index;
extern int16_t g_actor_environment;
extern uint8_t g_actor_update_marker;
extern int32_t g_actor_update_value;
extern uint8_t g_stride_marker;
extern uint8_t g_stride_mode;
extern const char *g_actor_message_strings[];
extern const char g_stride_error_condition[];
extern const char g_stride_source_path[];

extern void func_80037214(int32_t mode, BubsyGroundedActor *actor);
extern void func_80052ABC(void *component, int32_t *sequence);
extern int32_t func_80052AEC(void *component, int32_t sequence);
extern void func_80052E18(void *component);
extern int32_t func_80052BDC(void *component, int32_t mode);
extern void func_8005290C(void *component, uint8_t *output);
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);
extern void func_80026A0C(int32_t mode, int32_t value);
extern void func_8002C488(void);
extern int32_t func_800222E0(void);
extern int32_t func_80082ECC(void);
extern void func_80082E8C(char *destination, const char *source);

void func_8003539C(void *actor_input, int32_t mode_input) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register BubsyGroundedActor *actor __asm__("$17") = actor_input;
#else
    BubsyGroundedActor *actor = actor_input;
#endif
    int32_t sequence;
    uint8_t update_sequence = (uint8_t)mode_input;

    if (g_actor_update_phase != 0) {
        update_sequence = 0;
        actor->state_10 = 0;
    } else if ((uint8_t)mode_input != 0) {
        g_stride_marker = 0;
        g_actor_update_counter = 0;
    }

    if (actor->state_10 != 0) {
        return;
    }

    g_actor_update_marker = 0;
    func_80037214(0, actor);
    func_80052ABC(actor->component_1C, &sequence);
    {
        uint32_t actor_flags = actor->flags_04;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register uint32_t shifted_7 __asm__("$3") = actor_flags >> 7;
        register uint32_t flag_7 __asm__("$4");
        register uint32_t flag_7_test __asm__("$2");
#else
        uint32_t shifted_7 = actor_flags >> 7;
        uint32_t flag_7;
        uint32_t flag_7_test;
#endif
        uint32_t flag_8 = (actor_flags >> 8) & 1u;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__("" : "=r"(shifted_7), "=r"(flag_8) : "0"(shifted_7), "1"(flag_8));
#endif
        flag_7 = shifted_7 & 1u;
        if (flag_8 != 0) {
            if (g_stride_mode == 0) {
                update_sequence = 0;
            }
        } else if (sequence == 0x32E && g_stride_mode == 0) {
            update_sequence = 1;
        }

        flag_7_test = (uint8_t)flag_7;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__("" : "=r"(flag_7_test) : "0"(flag_7_test), "r"(flag_7));
#endif
        if (flag_7_test != 0) {
            if (g_stride_mode == 0) {
                if (sequence != 0x309 && (actor->state_10 & 0x800000u) == 0) {
                    if (func_80052AEC(actor->component_1C, 0x309) != 0) {
                        func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0x41A);
                    }
                    func_80052E18(actor->component_1C);
                }
                goto event_16;
            }
            goto check_mode1;
        } else if (sequence == 0x309) {
            if (g_stride_mode != 0) {
                goto check_mode1;
            }
            update_sequence = 1;
        }
    }

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=m"(sequence) : "m"(sequence));
#endif
    if (g_stride_mode == 0 && sequence == 0x35 && g_actor_update_counter <= 0) {
        goto check_boundary;
    }
check_mode1:
    if (g_stride_mode == 1 && (sequence == 0x9A || sequence == 0x54)) {
check_boundary:
        func_8005290C(actor->component_1C, &update_sequence);
        if (update_sequence != 0) {
            g_actor_update_counter = (int32_t)((uint32_t)g_actor_update_counter + 1u);
            if ((g_stride_mode == 1 && g_actor_update_counter >= 12) ||
                g_actor_environment == 20 || (actor->flags_04 & 0x40u) != 0) {
                g_actor_update_counter = 0;
            }
            if ((g_stride_mode == 0 && g_actor_update_counter <= 0) ||
                (g_stride_mode == 1 && g_actor_update_counter >= 11)) {
                func_80026A0C(7, 0);
            }
        }
    }

    if (g_stride_mode == 0 && g_actor_update_counter > 0 &&
        g_actor_environment != 20 && (actor->flags_04 & 0x40u) == 0) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register int16_t *phase_pointer __asm__("$16") = &g_actor_update_phase;
        __asm__ volatile("" : "=r"(phase_pointer) : "0"(phase_pointer));
#else
        int16_t *phase_pointer = &g_actor_update_phase;
#endif
        if (*phase_pointer == 0) {
            int32_t value = -10;
            uint8_t zero = 0;
            *phase_pointer = 3;
            func_8002C488();
            func_8001B558(0x2C, 4, 0xFFFE, 0xFFFE, &value, &zero, &zero, &zero);
        } else if (*phase_pointer == 3) {
            if ((uint8_t)func_800222E0() != 0) {
                *phase_pointer = 2;
            }
        } else if (*phase_pointer == 2) {
            BubsyActorMessage message;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            register int32_t index __asm__("$3") = g_actor_message_index;
#else
            int32_t index = g_actor_message_index;
#endif
            if (index < 6) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                register int32_t next_index __asm__("$2");
#else
                int32_t next_index;
#endif
                const char *selected_string = g_actor_message_strings[index];
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                __asm__("" : "=r"(index), "=r"(selected_string) : "0"(index), "1"(selected_string));
#endif
                next_index = (int32_t)((uint32_t)index + 1u);
                g_actor_message_index = next_index;
                func_80082E8C(message.text, selected_string);
            } else {
                index = func_80082ECC() % 6;
                func_80082E8C(message.text, g_actor_message_strings[index]);
            }
            message.value_80 = 0x200;
            message.value_84 = 0xF0;
            message.value_90 = -1;
            message.value_98 = 0x28;
            message.value_9C = 0xFFFE;
            message.value_A0 = 0xFFFE;
            message.value_88 = 0;
            message.value_8C = 0;
            message.value_96 = 0;
            message.value_95 = 0;
            message.value_94 = 0;
            message.value_A5 = 1;
            message.value_A6 = 0;
            func_8001B558(4, 5, 0xFFFF, 0xFFFF, &message, 0, 0, 0);
            g_actor_update_phase = 0;
            g_actor_update_counter = 0;
            update_sequence = 1;
        }
    }

    if (update_sequence == 1) {
        int32_t result;
        if (g_stride_mode == 0) {
            result = func_80052AEC(actor->component_1C, 0x35);
        } else {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            register int32_t mode_value __asm__("$2") = -0xF40;
            __asm__("" : "=r"(mode_value) : "0"(mode_value));
#else
            int32_t mode_value = -0xF40;
#endif
            g_actor_update_value = mode_value;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            __asm__ volatile("" : : : "memory");
#endif
            if ((actor->flags_04 & 0x818u) != 0) {
                result = func_80052AEC(actor->component_1C, 0x9A);
            } else {
                result = func_80052AEC(actor->component_1C, 0x54);
            }
        }
        if (result != 0) {
            func_800556D0(0, g_stride_error_condition, g_stride_source_path, 0x495);
        }
        func_80052E18(actor->component_1C);
        func_80052BDC(actor->component_1C, 0);
    }

event_16:
    func_8001B558(0x16, 4, actor->heading_0C, actor->heading_0C, 0, 0, 0, 0);
}
