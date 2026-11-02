#if defined(__mips__) || defined(BUBSY3D_MATCH_ORIGINAL_L0)

/* Uncredited C candidate: retain func_800347EC_match.c until linked bytes match. */
#include <stddef.h>
#include <stdint.h>

typedef struct BubsyActor347ECLayout {
    uint8_t state_00;
    uint8_t state_01;
    uint8_t unknown_02[2];
    uint32_t flags_04;
    uint8_t unknown_08[8];
    uint32_t state_10;
    uint8_t unknown_14[8];
    void *component_1C;
    uint8_t unknown_20[8];
    void *shared_28;
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
extern void func_8001C6BC(int32_t a0, int32_t a1, int32_t a2);
extern void func_8001B558(
    int32_t event_id,
    int32_t property_id,
    int32_t actor_value,
    int32_t actor_value_copy,
    int32_t argument_10,
    int32_t argument_14,
    int32_t argument_18,
    int32_t argument_1C
);
extern void func_8002944C(int32_t a0, int32_t a1);
extern void func_8003539C(void *actor, int32_t mode);
extern void func_80055680(const char *message);
extern void func_8002925C(int32_t a0, int32_t a1, int32_t a2, int32_t a3);
extern void func_80026A0C(int32_t a0, int32_t a1);

#define GLOBAL(type, address) global_##address
extern uint8_t global_0x80186479;
extern int32_t global_0x80186204[4];
extern int16_t global_0x80186450;
extern int16_t global_0x80186452;
extern int16_t global_0x801864BA;
extern uint8_t global_0x801864BC;
extern int32_t global_0x80186490;
extern uint32_t global_0x801864C0;
extern uint8_t global_0x801864B8;
extern uint8_t global_0x80186464;
extern uint32_t global_0x800BDE98;
extern const char global_0x80012958[];
extern const char global_0x8001293C[];
extern volatile uint32_t g_actor_transition_index;
register volatile uint8_t *gp __asm__("$28");

#define bubsy_actor_347ec_transition(actor, property_id) func_8001B558(0x27, property_id, *(int32_t *)((uint8_t *)(actor) + 0x0C), *(int32_t *)((uint8_t *)(actor) + 0x0C), 0, 0, 0, 0)

void func_800347EC(
    BubsyActor347ECLayout *input_actor,
    void *input_context,
    int32_t mode
) {
    register BubsyActor347ECLayout *actor __asm__("$17") = input_actor;
    register void *context __asm__("$19") = input_context;
    int32_t component_value;
    int32_t selector_result;
    int32_t selection_a1;
    register int32_t stack_10 __asm__("$9");
    register int32_t stack_14 __asm__("$8");
    register int32_t stack_18 __asm__("$3");
    register uint32_t state_word __asm__("$3");
    register int32_t saved_progress __asm__("$16");
    int16_t shared_mode;
    register void *shared_state __asm__("$18");
    register int32_t first_sequence __asm__("$20");
    register int32_t second_sequence __asm__("$21");
    int32_t mode_value;
    register int32_t mode_source __asm__("$6") = mode;
    register uint32_t state_mask __asm__("$2");
    uint32_t object_index;
    uint32_t object_limit;
    register int32_t *component_out __asm__("$5");
    register uint32_t active_state __asm__("$3");

    first_sequence = 0x29B;
    component_out = &component_value;
    __asm__ volatile("" : "=r"(actor), "=r"(context), "=r"(first_sequence), "=r"(component_out), "=r"(mode_source) : "0"(actor), "1"(context), "2"(first_sequence), "3"(component_out), "4"(mode_source));
    mode_value = mode_source;
    shared_state = actor->shared_28;
    func_80052ABC(actor->component_1C, component_out);

    second_sequence = 0x29B;
    if ((actor->flags_04 & 0x04u) != 0 || GLOBAL(uint8_t, 0x80186479) != 0) {
        return;
    }
    if ((actor->flags_04 & 0x1100u) != 0) {
        func_8002262C(actor, 2);
        func_8002262C(actor, 6);
        return;
    }

    state_mask = mode_value & 0xFF;
    __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
    if (state_mask == 0) {
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
                state_mask = state_word & 0x10000u;
                __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
                if (state_mask != 0) {
                    stack_10 = 0x2F8;
                    stack_14 = 0x2FF;
                    gp[0x37D] = 1;
                    stack_18 = 0x302;
                } else {
                    selection_a1 = 0x7D8;
                    stack_10 = first_sequence;
                    stack_14 = gp[0x37C] != 0 ? 0x33A : 0x2A0;
                    stack_18 = 0x2A7;
                }
            }
            __asm__ volatile("" : "=r"(stack_10), "=r"(stack_14), "=r"(stack_18) : "0"(stack_10), "1"(stack_14), "2"(stack_18));
            func_80022C94(actor, selection_a1, 1, 0, stack_10, stack_14, stack_18);
            bubsy_actor_347ec_transition(actor, 4);
            return;
        }

        if ((state_word & 0x9000u) != 0) {
            stack_14 = 0x2EA;
            stack_18 = 0x2EF;
        } else {
            state_mask = state_word & 0x10000u;
            __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
            if (state_mask != 0) {
                stack_14 = 0x2FF;
                stack_18 = 0x302;
            } else {
                stack_14 = 0x33A;
                stack_18 = 0x2A7;
            }
        }
        __asm__ volatile("" : "=r"(stack_14), "=r"(stack_18) : "0"(stack_14), "1"(stack_18));
        func_80022C94(actor, 0, 0, actor->state_00, 0, stack_14, stack_18);

        state_word = actor->state_10;
        if ((state_word & 0x04u) == 0) {
            goto shared_fallback;
        }
        if ((state_word & 0x8000u) != 0) {
            state_word = 0x2EF;
            __asm__ volatile("" : "=r"(state_word) : "0"(state_word));
            state_mask = component_value;
            __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
            saved_progress = *(int32_t *)((uint8_t *)shared_state + 0x30);
            if (state_mask != state_word) {
                shared_mode = GLOBAL(int16_t, 0x80186450);
                if (shared_mode != 0) {
                    func_800358C8(actor, context, shared_mode);
                }
            }
            goto restore_progress;
        }
        state_mask = state_word & 0x10000u;
        __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
        if (state_mask != 0) {
            state_word = 0x2FF;
            __asm__ volatile("" : "=r"(state_word) : "0"(state_word));
            state_mask = component_value;
            __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
            saved_progress = *(int32_t *)((uint8_t *)shared_state + 0x30);
            if (state_mask == state_word && gp[0x37D] != 0) {
                func_800358C8(actor, context, 1);
                gp[0x37D] = 0;
            }
restore_progress:
            *(int32_t *)((uint8_t *)shared_state + 0x30) = saved_progress;
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
            __asm__ volatile("" : : : "memory");
            GLOBAL(int16_t, 0x80186452) = 0;
            __asm__ volatile("" : : : "memory");
            func_8002E7FC();
            goto common_transition;
        }
        state_mask = state_word & 0x100u;
        __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
        if (state_mask != 0) {
            int16_t counter = GLOBAL(int16_t, 0x801864BA);

            if (counter >= 0xFE) {
                *(int32_t *)((uint8_t *)shared_state + 0x4C) =
                    counter == 0xFF ? 0x400 : -0x400;
                GLOBAL(int16_t, 0x801864BA) = 0;
                __asm__ volatile("" : : : "memory");
                actor->state_10 &= ~0x100u;
                __asm__ volatile("" : : : "memory");
            }
        }
common_transition:
        bubsy_actor_347ec_transition(actor, 4);
        return;
    }

shared_fallback:
    state_mask = state_word & 0x40u;
    __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
    if (state_mask != 0 && actor->state_00 == 1) {
        actor->state_00 = 0;
        func_80052AEC(actor->component_1C, 0x2B0);
        func_80052E18(actor->component_1C);
        func_8001C6BC(4, 3, -1);
        return;
    }
    func_8002262C(actor, 5);
    func_8002262C(actor, 8);
    func_8002262C(actor, 0xA);
    GLOBAL(uint32_t, 0x801864C0) = 0;
    __asm__ volatile("" : : : "memory");
    state_mask = actor->state_10;
    __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
    gp[0x37C] = 0;
    gp[0x37E] = 0;
    __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
    if ((state_mask & 0x1000u) != 0) {
        func_8002262C(actor, 0xC);
        GLOBAL(int16_t, 0x80186452) = 0;
        func_8002E7FC();
    }

    if ((actor->flags_04 & 0x02u) != 0) {
        object_index = *(volatile uint32_t *)(gp + 0x380) + 0x1Eu;
        object_limit = GLOBAL(uint32_t, 0x800BDE98);
        if (object_index < object_limit) {
            *(volatile uint32_t *)(gp + 0x380) = object_limit;
            actor->state_01 = 2;
            bubsy_actor_347ec_transition(actor, 5);
            gp[0x37C] = 1;
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
        GLOBAL(uint8_t, 0x80186464) = 0;
        GLOBAL(uint8_t, 0x801864B8) = 1;
    }
finish_fallback:
    if (gp[0x37C] == 0) {
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

    state_mask = state_word & 0x40u;
    __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
    if (state_mask == 0) {
        goto set_active;
    }
    if (GLOBAL(uint8_t, 0x801864BC) != 0) {
        selection_a1 = GLOBAL(int32_t, 0x80186490) < 0 ? 0x80 : 0x40;
        GLOBAL(uint8_t, 0x801864BC) = 0;
    } else {
        selection_a1 = 0;
    }
    selector_result = func_80022C94(actor, selection_a1 + 0x88, 0, 0, 0, 0x33A, 0x2A7) & 0xFF;
    __asm__ volatile("" : "=r"(selector_result) : "0"(selector_result));
    active_state = 1;
    __asm__ volatile("" : "=r"(active_state) : "0"(active_state));
    if (selector_result == active_state) {
        return;
    }

    active_state = 1;
    __asm__ volatile("" : "=r"(active_state) : "0"(active_state));
    state_mask = GLOBAL(uint8_t, 0x80186464);
    __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
    GLOBAL(uint32_t, 0x801864C0) = 0;
    gp[0x37E] = 0;
    GLOBAL(uint8_t, 0x801864B8) = active_state;
    if (state_mask != 0) {
        actor->state_01 = active_state;
    } else {
        state_mask = 2;
        __asm__ volatile("" : "=r"(state_mask) : "0"(state_mask));
        actor->state_01 = state_mask;
    }
    func_8002944C(4, 3);
    if ((actor->flags_04 & 0x02u) == 0) {
        return;
    }
    object_index = *(volatile uint32_t *)(gp + 0x380) + 0x1Eu;
    object_limit = GLOBAL(uint32_t, 0x800BDE98);
    if (object_index < object_limit) {
        g_actor_transition_index = object_limit;
        func_8001B558(0x27, 4,
            *(int32_t *)((uint8_t *)actor + 0x0C), *(int32_t *)((uint8_t *)actor + 0x0C),
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