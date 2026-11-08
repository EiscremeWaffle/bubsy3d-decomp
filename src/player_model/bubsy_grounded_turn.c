#include "bubsy_grounded_turn.h"

extern uint8_t g_stride_marker;
extern uint8_t g_stride_mode;
extern uint8_t g_turn_direction;
extern uint8_t g_runtime_state_61;
extern int16_t g_turn_counter;
extern const char g_stride_error_condition[];
extern const char g_stride_source_path[];

extern void func_8001AE88(uint32_t *value);
extern void func_8002262C(BubsyGroundedActor *actor, int32_t selector);
extern void func_80022650(BubsyGroundedActor *actor, int32_t selector);
extern void func_80052ABC(void *component, int32_t *sequence);
extern int32_t func_80052AEC(void *component, int32_t sequence);
extern void func_80052E18(void *component);
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

void func_80034EF8(
    BubsyGroundedActor *actor_input,
    BubsyGroundedState *state,
    void *context,
    int32_t counter_input
) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register BubsyGroundedActor *actor __asm__("$16") = actor_input;
    int32_t turn = 0;
    register int32_t counter __asm__("$18");
    register int32_t saved_argument __asm__("$7") = counter_input;
    int32_t direction;
    register BubsyGroundedTurnShared *shared __asm__("$20");
    register int32_t direction_test __asm__("$2");
#else
    BubsyGroundedActor *actor = actor_input;
    int32_t turn = 0;
    int32_t counter = counter_input;
    int32_t direction;
    BubsyGroundedTurnShared *shared;
    int32_t direction_test;
#endif
    uint32_t sample;
    uint32_t state_bits;
    int32_t sequence;
    int32_t selection_result;
    int32_t old_direction;

    (void)context;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(actor), "=r"(turn) : "0"(actor), "1"(turn));
#endif
    shared = (BubsyGroundedTurnShared *)actor->shared_28;
    g_stride_marker = 0;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : : : "memory");
#endif
    direction_test = state->id == 8;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__("" : "=r"(direction_test) : "0"(direction_test));
#endif
    direction = direction_test;
    old_direction = g_turn_direction;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(saved_argument), "=r"(old_direction) : "0"(saved_argument), "1"(old_direction));
    counter = saved_argument;
#endif
    if (old_direction != direction) {
        g_turn_direction = direction;
        func_8002262C(actor_input, 5);
    }

    if (g_stride_mode == 1) {
        func_8001AE88(&sample);
        state_bits = actor->state_10;
        if ((state_bits & 0x18u) != 0) {
            if ((state_bits & 0x200u) == 0) {
                goto start_timed_state;
            }
            if (sample - actor->state_14 >= 5u) {
                turn = 39;
                actor->state_14 = sample;
            }
        } else if ((state_bits & 0x40u) != 0) {
            if (sample - actor->state_14 >= 5u) {
                turn = 28;
                actor->state_14 = sample;
            }
        } else {
            func_80022650(actor, 5);
            turn = 22;
        }
        goto publish;
    }

    state_bits = actor->state_10;
    if ((state_bits & 0x818u) != 0 && (state_bits & 0x40u) == 0) {
        double scale;
        int32_t value;
        if ((state_bits & 0x200u) == 0) {
start_timed_state:
            func_80022650(actor, 9);
            func_8001AE88(&actor->state_14);
            goto publish;
        }
        value = (int16_t)counter;
        if (value < 5) {
            scale = value * 0.2;
        } else {
            scale = (value - 5) * 0.06 + 1.0;
        }
        turn = (int32_t)(scale * 4096.0) / 360;
        goto publish;
    }

    if ((state_bits & 0x40u) != 0) {
        if (g_stride_mode == 0) {
            func_80052ABC(actor->component_1C, &sequence);
            if ((uint8_t)direction == 1) {
                if (sequence == 0x2B8) {
                    selection_result = func_80052AEC(actor->component_1C, 0x2B5);
                } else {
                    selection_result = func_80052AEC(actor->component_1C, 0x2BB);
                }
            } else if (sequence == 0x2BB) {
                selection_result = func_80052AEC(actor->component_1C, 0x2B5);
            } else {
                selection_result = func_80052AEC(actor->component_1C, 0x2B8);
            }
            if (selection_result != 0) {
                func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0x375);
            }
            func_80052E18(actor->component_1C);
            turn = 22;
            func_80022650(actor, 10);
            goto publish;
        }
    }

    if ((*(volatile uint32_t *)&actor->state_10 & 0x20u) == 0) {
        uint32_t runtime_state;
        func_80022650(actor, 5);
        runtime_state = g_runtime_state_61;
        g_turn_counter = 0;
        if (runtime_state != 2 && (actor->flags_04 & 0x80u) == 0) {
            if ((uint8_t)direction == 1) {
                if (g_stride_mode == 0) {
                    if (func_80052AEC(actor->component_1C, 0x288) != 0) {
                        func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0x38E);
                    }
                    func_80052E18(actor->component_1C);
                }
            } else if (g_stride_mode == 0) {
                if (func_80052AEC(actor->component_1C, 0x28E) != 0) {
                    func_800556D0(1, g_stride_error_condition, g_stride_source_path, 0x397);
                }
                func_80052E18(actor->component_1C);
            }
        }
        goto publish;
    }

    func_80052ABC(actor->component_1C, &sequence);
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : : : "$17");
#endif
    counter = 56;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__("" : "=r"(counter) : "0"(counter));
#endif
    turn = 11;
    if ((actor->flags_04 & 0x80u) == 0 && sequence == 0x309 && g_stride_mode == 0) {
        if ((uint8_t)direction == 1) {
            func_80052AEC(actor->component_1C, 0x288);
        } else {
            func_80052AEC(actor->component_1C, 0x28E);
        }
        func_80052E18(actor->component_1C);
    }
    if (g_runtime_state_61 == 2) {
        turn *= 2;
    } else {
        uint32_t previous = (uint16_t)g_turn_counter;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__("" : "=r"(previous) : "0"(previous));
#endif
        turn += (int16_t)previous;
        g_turn_counter = (int16_t)(previous + 1u);
        if (turn > counter) {
            turn = counter;
        }
    }

publish:
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : : "r"(counter));
#endif
    if (turn != 0) {
        if ((uint8_t)direction == 1) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            __asm__ volatile("" : : : "memory");
#endif
            shared->turn_4C = turn;
        } else {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            register int32_t negative __asm__("$2") = -turn;
            __asm__("" : "=r"(negative) : "0"(negative));
#else
            int32_t negative = -turn;
#endif
            shared->turn_4C = negative;
        }
    }
}
