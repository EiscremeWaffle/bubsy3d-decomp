#if defined(__mips__)

#include <stddef.h>
#include <stdint.h>

typedef struct BubsyActor347ECLayout {
    volatile uint8_t state_00;
    volatile uint8_t state_01;
    uint8_t unknown_02[2];
    volatile uint32_t flags_04;
    uint8_t unknown_08[8];
    volatile uint32_t state_10;
    uint8_t unknown_14[8];
    void *component_1C;
    uint8_t unknown_20[8];
    void *shared_28;
} BubsyActor347ECLayout;

_Static_assert(offsetof(BubsyActor347ECLayout, flags_04) == 0x04, "actor flag offset");
_Static_assert(offsetof(BubsyActor347ECLayout, state_10) == 0x10, "actor state offset");
_Static_assert(offsetof(BubsyActor347ECLayout, component_1C) == 0x1C, "component offset");
_Static_assert(offsetof(BubsyActor347ECLayout, shared_28) == 0x28, "shared-state offset");

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

#define GLOBAL(type, address) (*(volatile type *)(uintptr_t)(address))

static void bubsy_actor_347ec_transition(
    BubsyActor347ECLayout *actor,
    int32_t property_id
) {
    int32_t actor_value = *(volatile int32_t *)((volatile uint8_t *)actor + 0x0C);

    func_8001B558(0x27, property_id, actor_value, actor_value, 0, 0, 0, 0);
}

void bubsy_actor_347ec_reconstructed(
    BubsyActor347ECLayout *actor,
    void *context,
    int32_t mode
) {
    volatile uint8_t *gp;
    int32_t component_value;
    int32_t selector_result;
    int32_t selection_a1;
    int32_t stack_10;
    int32_t stack_14;
    int32_t stack_18;
    int32_t state_word;
    int32_t saved_progress;
    int16_t shared_mode;
    void *shared_state;
    int32_t *shared_progress;
    uint32_t object_index;
    uint32_t object_limit;

    __asm__ volatile("move %0, $gp" : "=r"(gp));
    shared_state = actor->shared_28;
    func_80052ABC(actor->component_1C, &component_value);

    if ((actor->flags_04 & 0x04u) != 0 || GLOBAL(uint8_t, 0x80186479) != 0) {
        return;
    }
    if ((actor->flags_04 & 0x1100u) != 0) {
        func_8002262C(actor, 2);
        func_8002262C(actor, 6);
        return;
    }

    if ((mode & 0xFF) != 0) {
        state_word = actor->state_10;
        if ((state_word & 0x44) == 0) {
            actor->state_00 = 0;
            func_8002269C(actor, 1);
            func_80054B20(context, 0, (void *)(uintptr_t)0x80186204);
            stack_10 = 0x29B;
            if ((actor->state_10 & 0x18u) != 0) {
                func_8002262C(actor, 3);
                func_8002262C(actor, 4);
                stack_10 = 0x294;
            }

            if ((actor->state_10 & 0x9000u) != 0) {
                selection_a1 = 0x29D;
                stack_10 = 0x2E3;
                stack_14 = 0x2EA;
                stack_18 = 0x2EF;
            } else if ((actor->state_10 & 0x10000u) != 0) {
                selection_a1 = 0x29D;
                stack_10 = 0x2F8;
                stack_14 = 0x2FF;
                stack_18 = 0x302;
                gp[0x37D] = 1;
            } else {
                selection_a1 = 0x7D8;
                stack_14 = gp[0x37C] != 0 ? 0x33A : 0x2A0;
                stack_18 = 0x2A7;
            }
            func_80022C94(actor, selection_a1, 1, 0, stack_10, stack_14, stack_18);
            bubsy_actor_347ec_transition(actor, 4);
            return;
        }

        if ((state_word & 0x9000u) != 0) {
            stack_14 = 0x2EA;
            stack_18 = 0x2EF;
        } else if ((state_word & 0x10000u) != 0) {
            stack_14 = 0x2FF;
            stack_18 = 0x302;
        } else {
            stack_14 = 0x33A;
            stack_18 = 0x2A7;
        }
        func_80022C94(actor, 0, 0, actor->state_00, 0, stack_14, stack_18);

        state_word = actor->state_10;
        if ((state_word & 0x04u) == 0) {
            goto shared_fallback;
        }
        shared_progress = (int32_t *)((uint8_t *)shared_state + 0x30);
        if ((state_word & 0x8000u) != 0) {
            saved_progress = *shared_progress;
            if (component_value != 0x2EF) {
                shared_mode = GLOBAL(int16_t, 0x80186450);
                if (shared_mode != 0) {
                    func_800358C8(actor, context, shared_mode);
                }
            }
            *shared_progress = saved_progress;
            bubsy_actor_347ec_transition(actor, 4);
            return;
        }
        if ((state_word & 0x10000u) != 0) {
            saved_progress = *shared_progress;
            if (component_value == 0x2FF && gp[0x37D] != 0) {
                func_800358C8(actor, context, 1);
                gp[0x37D] = 0;
            }
            *shared_progress = saved_progress;
            bubsy_actor_347ec_transition(actor, 4);
            return;
        }
        if ((state_word & 0x1000u) != 0 && component_value == 0x2EA) {
            int32_t transform_input;
            int32_t transform_state[3];

            func_800516F8(&transform_input);
            func_80054AF0((void *)(uintptr_t)transform_input, transform_state);
            transform_state[1] = 0;
            func_80055280(context, 0, transform_state);
            actor->state_10 &= ~0x1000u;
            GLOBAL(int16_t, 0x80186452) = 0;
            func_8002E7FC();
            bubsy_actor_347ec_transition(actor, 4);
            return;
        }
        if ((state_word & 0x100u) != 0) {
            int16_t counter = GLOBAL(int16_t, 0x801864BA);

            if (counter >= 0xFE) {
                *(volatile int32_t *)((volatile uint8_t *)shared_state + 0x4C) =
                    counter == 0xFF ? 0x400 : -0x400;
                GLOBAL(int16_t, 0x801864BA) = 0;
                actor->state_10 &= ~0x100u;
                bubsy_actor_347ec_transition(actor, 4);
                return;
            }
        }
        bubsy_actor_347ec_transition(actor, 4);
        return;
    }

    state_word = actor->state_10;
    if ((state_word & 0x44u) == 0) {
        actor->state_00 = 1;
        if ((state_word & 0x18u) != 0) {
            func_8002262C(actor, 3);
            func_8002262C(actor, 4);
        }
        func_80022C94(actor, 0x680, 1, 0, 0x29B, 0x33A, 0x2A7);
        bubsy_actor_347ec_transition(actor, 4);
        func_8002925C(4, 3, 1, -1);
        func_80026A0C(4, 0);
        return;
    }

    if ((state_word & 0x40u) == 0) {
        actor->state_00 = 1;
        return;
    }
    if (GLOBAL(uint8_t, 0x801864BC) != 0) {
        selection_a1 = GLOBAL(int32_t, 0x80186490) < 0 ? 0x108 : 0xC8;
        GLOBAL(uint8_t, 0x801864BC) = 0;
    } else {
        selection_a1 = 0x88;
    }
    selector_result = func_80022C94(actor, selection_a1, 0, 0, 0, 0x33A, 0x2A7) & 0xFF;
    if (selector_result == 1) {
        return;
    }

    GLOBAL(uint32_t, 0x801864C0) = 0;
    gp[0x37E] = 0;
    GLOBAL(uint8_t, 0x801864B8) = 1;
    actor->state_01 = GLOBAL(uint8_t, 0x80186464) != 0 ? 1 : 2;
    func_8002944C(4, 3);
    if ((actor->flags_04 & 0x02u) == 0) {
        return;
    }
    object_index = *(volatile uint32_t *)(gp + 0x380) + 0x1Eu;
    object_limit = GLOBAL(uint32_t, 0x800BDE98);
    if (object_index < object_limit) {
        *(volatile uint32_t *)(gp + 0x380) = object_limit;
        actor->state_01 = 2;
        bubsy_actor_347ec_transition(actor, 4);
        func_8002269C(actor, 1);
    } else {
        func_80055680((const char *)(uintptr_t)0x80012958);
    }
    return;

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
    GLOBAL(uint32_t, 0x801864C0) = 0;
    gp[0x37C] = 0;
    gp[0x37E] = 0;
    if ((actor->state_10 & 0x1000u) != 0) {
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
            actor->flags_04 &= ~0x03u;
        } else {
            func_80055680((const char *)(uintptr_t)0x8001293C);
        }
    }

    if ((actor->state_10 & 0x18000u) != 0) {
        func_8002262C(actor, 0xF);
        func_8002262C(actor, 0x10);
    } else {
        GLOBAL(uint8_t, 0x80186464) = 0;
        GLOBAL(uint8_t, 0x801864B8) = 1;
    }
    if (gp[0x37C] == 0) {
        func_8003539C(actor, 1);
    }
}

#endif