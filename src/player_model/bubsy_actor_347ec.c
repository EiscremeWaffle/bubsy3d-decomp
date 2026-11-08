#if defined(__mips__) || defined(BUBSY3D_MATCH_ORIGINAL_L0)

/*
 * Shared grounded transition handler. Byte-exact against L0 with GCC 2.6.3.
 * The remaining register pins and the entry barrier reproduce the original
 * global register allocation; no instructions are emitted by the barrier.
 */
#include <stddef.h>
#include <stdint.h>
#include "bubsy_actor_state_update.h"
#include "game_transition.h"

typedef struct BubsyActor347ECShared {
    uint8_t unknown_00[0x30];
    int32_t progress_30;
    uint8_t unknown_34[0x18];
    int32_t turn_speed_4C;
} BubsyActor347ECShared;

typedef struct BubsyActor347ECLayout {
    uint8_t state_00;
    uint8_t state_01;
    uint8_t unknown_02[2];
    uint32_t flags_04;
    uint8_t unknown_08[4];
    int32_t object_id_0C;
    uint32_t state_10;
    uint8_t unknown_14[8];
    void *component_1C;
    uint8_t unknown_20[8];
    BubsyActor347ECShared *shared_28;
} BubsyActor347ECLayout;

extern int32_t func_80052ABC(void *component, int32_t *value_out);
extern void func_8002262C(void *actor, int32_t property_id);
extern void func_8002269C(void *actor, int32_t mode);
extern void func_80054B20(void *context, int32_t mode, void *data);
extern int32_t func_80022C94(
    void *actor,
    int32_t a1,
    int32_t a2,
    int32_t a3,
    int32_t stack_10,
    int32_t stack_14,
    int32_t stack_18
);
extern void func_800358C8(void *actor, void *context, int32_t mode);
extern void func_800516F8(void *output);
extern void func_80054AF0(void *context, void *output);
extern void func_80055280(void *context, int32_t mode, void *state);
extern void func_8002E7FC(void);
extern void func_80052AEC(void *component, int32_t state_id);
extern void func_80052E18(void *component);
extern int32_t func_8001C6BC(int32_t a0, int32_t a1, int32_t a2);
extern void func_8002944C(int32_t a0, int32_t a1);
extern void func_80055680(const char *message);
extern void func_8002925C(int32_t a0, int32_t a1, int32_t a2, int32_t a3);
extern void func_80026A0C(int32_t a0, int32_t a1);

typedef struct PlayerGlobals80186450 {
    int16_t shared_mode_00;
    int16_t word_02;
    uint8_t unknown_04[16];
    uint8_t flag_14;
    uint8_t unknown_15[20];
    uint8_t flag_29;
    uint8_t unknown_2A[22];
    int32_t value_40;
    uint8_t unknown_44[36];
    uint8_t flag_68;
    uint8_t unknown_69[1];
    int16_t counter_6A;
    uint8_t flag_6C;
    uint8_t unknown_6D[3];
    uint32_t value_70;
} PlayerGlobals80186450;
extern PlayerGlobals80186450 g_player_80186450;
extern int32_t global_0x80186204[4];
extern uint32_t global_0x800BDE98;
extern const char global_0x80012958[];
extern const char global_0x8001293C[];
extern uint32_t g_actor_transition_index;
extern uint8_t g_actor_flag_800BE0D8;
extern uint8_t g_actor_flag_800BE0D9;
extern uint8_t g_actor_flag_800BE0DA;

#define bubsy_actor_347ec_transition(actor, property_id) func_8001B558(0x27, property_id, (actor)->object_id_0C, (actor)->object_id_0C, 0, 0, 0, 0)

void func_800347EC(
    BubsyActor347ECLayout *input_actor,
    void *input_context,
    int32_t mode
) {
    BubsyActor347ECLayout *actor = input_actor;
    void *context = input_context;
    int32_t component_value;
    int32_t selection_a1;
    register int32_t stack_10 __asm__("$9");
    int32_t stack_14;
    register int32_t stack_18 __asm__("$3");
    uint32_t state_word;
    int32_t saved_progress;
    int16_t shared_mode;
    register BubsyActor347ECShared *shared_state __asm__("$18");
    register int32_t first_sequence __asm__("$20");
    int32_t second_sequence;
    int32_t mode_value;
    register int32_t mode_source __asm__("$6") = mode;
    uint32_t fallback_state;
    int32_t expected_component;
    uint32_t object_index;
    uint32_t object_limit;

    first_sequence = 0x29B;
    __asm__("" : "=r"(actor), "=r"(context), "=r"(first_sequence), "=r"(mode_source) : "0"(actor), "1"(context), "2"(first_sequence), "3"(mode_source));
    mode_value = mode_source;
    shared_state = actor->shared_28;
    func_80052ABC(actor->component_1C, &component_value);

    second_sequence = 0x29B;
    if ((actor->flags_04 & 0x04u) != 0 || g_player_80186450.flag_29 != 0) {
        return;
    }
    if ((actor->flags_04 & 0x1100u) != 0) {
        func_8002262C(actor, 2);
        func_8002262C(actor, 6);
        return;
    }

    if ((mode_value & 0xFF) == 0) {
        goto mode_zero;
    }
    {
        state_word = actor->state_10;
        if ((state_word & 0x44) == 0) {
            actor->state_00 = 0;
            func_8002269C(actor, 1);
            func_80054B20(context, 0, global_0x80186204);
            if ((actor->state_10 & 0x18u) != 0) {
                func_8002262C(actor, 3);
                func_8002262C(actor, 4);
                first_sequence = 0x294;
            }

            state_word = actor->state_10;
            selection_a1 = 0x29D;
            if ((state_word & 0x9000u) != 0) {
                stack_10 = 0x2E3;
                stack_14 = 0x2EA;
                stack_18 = 0x2EF;
            } else {
                if ((state_word & 0x10000u) != 0) {
                    stack_10 = 0x2F8;
                    stack_14 = 0x2FF;
                    g_actor_flag_800BE0D9 = 1;
                    stack_18 = 0x302;
                } else {
                    selection_a1 = 0x7D8;
                    stack_10 = first_sequence;
                    stack_14 = g_actor_flag_800BE0D8 != 0 ? 0x33A : 0x2A0;
                    stack_18 = 0x2A7;
                }
            }

            func_80022C94(actor, selection_a1, 1, 0, stack_10, stack_14, stack_18);
            bubsy_actor_347ec_transition(actor, 4);
            return;
        }

        if ((state_word & 0x9000u) != 0) {
            stack_14 = 0x2EA;
            stack_18 = 0x2EF;
        } else {
            if ((state_word & 0x10000u) != 0) {
                stack_14 = 0x2FF;
                stack_18 = 0x302;
            } else {
                stack_14 = 0x33A;
                stack_18 = 0x2A7;
            }
        }

        func_80022C94(actor, 0, 0, actor->state_00, 0, stack_14, stack_18);

        state_word = actor->state_10;
        if ((state_word & 0x04u) == 0) {
            goto shared_fallback;
        }
        if ((state_word & 0x8000u) != 0) {
            expected_component = 0x2EF;
            saved_progress = shared_state->progress_30;
            if (component_value != expected_component) {
                shared_mode = g_player_80186450.shared_mode_00;
                if (shared_mode != 0) {
                    func_800358C8(actor, context, shared_mode);
                }
            }
            goto restore_progress;
        }
        if ((state_word & 0x10000u) != 0) {
            expected_component = 0x2FF;
            saved_progress = shared_state->progress_30;
            if (component_value == expected_component && g_actor_flag_800BE0D9 != 0) {
                func_800358C8(actor, context, 1);
                g_actor_flag_800BE0D9 = 0;
            }
restore_progress:
            shared_state->progress_30 = saved_progress;
            goto common_transition;
        }
        if ((state_word & 0x1000u) != 0) {
            int32_t transform_input;
            int32_t transform_state[3];

            if (component_value != 0x2EA) {
                goto common_transition;
            }
            func_800516F8(&transform_input);
            func_80054AF0((void *)(uintptr_t)transform_input, transform_state);
            transform_state[1] = 0;
            func_80055280(context, 0, transform_state);
            actor->state_10 &= ~0x1000u;
            g_player_80186450.word_02 = 0;
            func_8002E7FC();
            goto common_transition;
        }
        if ((state_word & 0x100u) != 0) {
            int16_t counter = g_player_80186450.counter_6A;

            if (counter >= 0xFE) {
                shared_state->turn_speed_4C =
                    counter == 0xFF ? 0x400 : -0x400;
                g_player_80186450.counter_6A = 0;
                actor->state_10 &= ~0x100u;
            }
        }
common_transition:
        bubsy_actor_347ec_transition(actor, 4);
        return;
    }

shared_fallback:
    if ((state_word & 0x40u) != 0 && actor->state_00 == 1) {
        actor->state_00 = 0;
        func_80052AEC(actor->component_1C, 0x2B0);
        func_80052E18(actor->component_1C);
        func_8001C6BC(4, 3, -1);
        return;
    }
    func_8002262C(actor, 5);
    func_8002262C(actor, 8);
    func_8002262C(actor, 0xA);
    g_player_80186450.value_70 = 0;
    fallback_state = actor->state_10;
    g_actor_flag_800BE0D8 = 0;
    g_actor_flag_800BE0DA = 0;

    if ((fallback_state & 0x1000u) != 0) {
        func_8002262C(actor, 0xC);
        g_player_80186450.word_02 = 0;
        func_8002E7FC();
    }

    if ((actor->flags_04 & 0x02u) != 0) {
        object_index = g_actor_transition_index + 0x1Eu;
        object_limit = global_0x800BDE98;
        if (object_index < object_limit) {
            g_actor_transition_index = object_limit;
            actor->state_01 = 2;
            bubsy_actor_347ec_transition(actor, 5);
            g_actor_flag_800BE0D8 = 1;
            actor->flags_04 &= ~0x02u;
            goto finish_fallback;
        } else {
            func_80055680(global_0x8001293C);
        }
    }

    if ((actor->state_10 & 0x18000u) != 0) {
        func_8002262C(actor, 0xF);
        func_8002262C(actor, 0x10);
    } else {
        g_player_80186450.flag_14 = 0;
        g_player_80186450.flag_68 = 1;
    }
finish_fallback:
    if (g_actor_flag_800BE0D8 == 0) {
        func_8003539C(actor, 1);
    }
    return;

mode_zero:
    state_word = actor->state_10;
    if ((state_word & 0x44u) == 0) {
        actor->state_00 = 1;
        if ((actor->state_10 & 0x18u) != 0) {
            func_8002262C(actor, 3);
            func_8002262C(actor, 4);
        }
        func_80022C94(actor, 0x680, 1, 0, second_sequence, 0x33A, 0x2A7);
        bubsy_actor_347ec_transition(actor, 4);
        func_8002925C(4, 3, 1, -1);
        func_80026A0C(4, 0);
        return;
    }

    if ((state_word & 0x40u) == 0) {
        goto set_active;
    }
    if (g_player_80186450.flag_6C != 0) {
        selection_a1 = g_player_80186450.value_40 < 0 ? 0x80 : 0x40;
        g_player_80186450.flag_6C = 0;
    } else {
        selection_a1 = 0;
    }
    if ((uint8_t)func_80022C94(actor, selection_a1 + 0x88, 0, 0, 0, 0x33A, 0x2A7) == 1) {
        return;
    }

    g_player_80186450.value_70 = 0;
    g_actor_flag_800BE0DA = 0;
    g_player_80186450.flag_68 = 1;
    if (g_player_80186450.flag_14 != 0) {
        actor->state_01 = 1;
    } else {
        actor->state_01 = 2;
    }
    func_8002944C(4, 3);
    if ((actor->flags_04 & 0x02u) == 0) {
        return;
    }
    object_index = g_actor_transition_index + 0x1Eu;
    object_limit = global_0x800BDE98;
    if (object_index < object_limit) {
        g_actor_transition_index = object_limit;
        func_8001B558(0x27, 4,
            actor->object_id_0C, actor->object_id_0C,
            0, 0, 0, 0);
        func_8002269C(actor, 1);
    } else {
        func_80055680(global_0x80012958);
    }
    return;
set_active:
    actor->state_00 = 1;
}

#endif