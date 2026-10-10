#include "bubsy_actor_update.h"

uint32_t bubsy_grounded_handler_case_target(uint16_t state_id) {
    static const uint32_t targets[33] = {
        0x8004B990, 0x8004BE60, 0x8004BC48, 0x8004BE60, 0x8004BC48,
        0x8004BE60, 0x8004B70C, 0x8004B770, 0x8004B838, 0x8004B8D8,
        0x8004B77C, 0x8004B7F8, 0x8004BFD0, 0x8004BFD0, 0x8004B4D8,
        0x8004B578, 0x8004BFD0, 0x8004BFD0, 0x8004BFD0, 0x8004BFD0,
        0x8004BFD0, 0x8004BFD0, 0x8004BFD0, 0x8004BA18, 0x8004BA28,
        0x8004BA34, 0x8004BB68, 0x8004BDA4, 0x8004BE4C, 0x8004BDA4,
        0x8004BE4C, 0x8004B66C, 0x8004B6C0,
    };

    if (state_id < 6 || state_id > 38) {
        return 0x8004BFD0;
    }
    return targets[state_id - 6];
}

int32_t bubsy_grounded_handler_should_dispatch_switch(
    uint16_t state_id,
    int32_t descriptor_lookup_word_04,
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    int16_t *descriptor_state_02
) {
    if (descriptor_lookup_word_04 != 3) {
        return 1;
    }
    if (state_id == 14) {
        if ((actor_flags_04 & 0x1100u) != 0) {
            return 0;
        }
        if ((actor_state_10 & 0x1000u) != 0) {
            if (*descriptor_state_02 == 0) {
                *descriptor_state_02 = 1;
            }
            return 0;
        }
        return 1;
    }
    if (state_id == 15 && (actor_state_10 & 0x1000u) != 0) {
        *descriptor_state_02 = 0;
        return 0;
    }
    return 1;
}

uint8_t bubsy_grounded_case38_update_descriptor(uint8_t *descriptor_state_11) {
    uint8_t actions = BUBSY_GROUNDED_CASE38_CALL_39158;

    if (*descriptor_state_11 == 2) {
        *descriptor_state_11 = 4;
        return actions;
    }
    if (*descriptor_state_11 == 1) {
        *descriptor_state_11 = 0;
        return actions | BUBSY_GROUNDED_CASE38_CALL_2262C_11;
    }
    return actions;
}

BubsyGroundedCase37Action bubsy_grounded_case37_select_action(
    uint32_t actor_flags_04,
    uint8_t *descriptor_state_11
) {
    if ((actor_flags_04 & 0x1100u) != 0) {
        return BUBSY_GROUNDED_CASE37_NO_ACTION;
    }
    if (*descriptor_state_11 == 0) {
        *descriptor_state_11 = 1;
        return BUBSY_GROUNDED_CASE37_INITIALIZE;
    }
    if (*descriptor_state_11 == 5) {
        return BUBSY_GROUNDED_CASE37_NO_ACTION;
    }
    return BUBSY_GROUNDED_CASE37_PROCESS_ACTOR;
}

int32_t bubsy_grounded_cases29_30_update_descriptor(
    uint16_t state_id,
    uint8_t *descriptor_state_10
) {
    if (state_id == 29) {
        *descriptor_state_10 = 1;
        return 1;
    }
    if (state_id == 30) {
        *descriptor_state_10 = 0;
        return 1;
    }
    return 0;
}

int32_t bubsy_grounded_cases13_34_36_update_state(
    uint16_t state_id,
    uint8_t *actor_state_01,
    uint16_t *descriptor_state_00
) {
    if (state_id != 13 && state_id != 34 && state_id != 36) {
        return 0;
    }
    *actor_state_01 = 2;
    if (state_id != 13) {
        *descriptor_state_00 = 0;
    }
    return 1;
}

BubsyGroundedCase21Action bubsy_grounded_case21_select_action(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    int32_t companion_state_28,
    int32_t normalized_dot,
    int16_t *descriptor_state_02
) {
    BubsyGroundedCase21Action actions = BUBSY_GROUNDED_CASE21_CALL_2262C_0C;

    if ((actor_state_10 & 0x1000u) != 0 &&
        (actor_flags_04 & 0x40u) == 0 &&
        companion_state_28 == 0 &&
        normalized_dot < 0xCCC &&
        *descriptor_state_02 != 5) {
        actions = BUBSY_GROUNDED_CASE21_CALL_2E7D8 |
            BUBSY_GROUNDED_CASE21_CALL_22650_08 |
            BUBSY_GROUNDED_CASE21_CALL_347EC_1;
    }
    *descriptor_state_02 = 0;
    return actions;
}

uint8_t bubsy_grounded_case12_update_state(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t *actor_state_01,
    int16_t *descriptor_state_02
) {
    uint8_t actions = BUBSY_GROUNDED_CASE12_NO_ACTION;

    if ((actor_flags_04 & 0x1104u) != 0 ||
        (actor_flags_04 & 0x818u) == 0 ||
        (actor_state_10 & 0xA0084u) != 0) {
        return actions;
    }

    if ((actor_state_10 & 0x1000u) != 0) {
        actions |= BUBSY_GROUNDED_CASE12_CALL_2262C_0C;
    }
    *descriptor_state_02 = 5;
    if (*actor_state_01 != 1) {
        *actor_state_01 = 1;
        actions |= BUBSY_GROUNDED_CASE12_CALL_347EC_1;
    }
    return actions;
}

uint8_t bubsy_grounded_case17_update_state(
    uint32_t actor_state_10,
    uint8_t *actor_state_00,
    uint8_t *actor_state_01,
    uint8_t *descriptor_state_14
) {
    uint8_t actions = BUBSY_GROUNDED_CASE17_CALL_1C700;

    *descriptor_state_14 = 0;
    *actor_state_01 = 2;
    if ((actor_state_10 & 0x44u) != 0) {
        *actor_state_00 = 0;
        actions |= BUBSY_GROUNDED_CASE17_CALL_22650_02 |
            BUBSY_GROUNDED_CASE17_CALL_347EC_1;
    }
    return actions;
}

int32_t bubsy_grounded_case16_should_dispatch(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t actor_state_01,
    uint8_t *descriptor_state_14
) {
    if (*descriptor_state_14 == 0) {
        if ((actor_flags_04 & 0x2u) != 0) {
            return 0;
        }
        *descriptor_state_14 = 1;
    }
    return (actor_flags_04 & 0x1104u) == 0 &&
        (actor_state_10 & 0xA9180u) == 0 && actor_state_01 != 1;
}

BubsyGroundedCase14Actions bubsy_grounded_case14_prepare(
    uint32_t actor_flags_04,
    uint32_t *actor_state_10,
    uint8_t descriptor_state_11,
    uint32_t *linked_state_30
) {
    if ((*actor_state_10 & 0x010000C4u) != 0 ||
        (actor_flags_04 & 0x818u) == 0 ||
        descriptor_state_11 == 2 ||
        (actor_flags_04 & 0x1104u) != 0) {
        return BUBSY_GROUNDED_CASE14_NO_ACTION;
    }
    *actor_state_10 = 0;
    *linked_state_30 = 0;
    return BUBSY_GROUNDED_CASE14_CALL_22650_07 |
        BUBSY_GROUNDED_CASE14_SELECT_2BE |
        BUBSY_GROUNDED_CASE14_ASSERT_ON_SELECTION_FAILURE |
        BUBSY_GROUNDED_CASE14_CALL_52E18;
}

uint8_t bubsy_grounded_case15_select_actions(
    int32_t current_sequence_key,
    uint8_t sequence_boundary_reached
) {
    if (current_sequence_key == 0x2BE) {
        return BUBSY_GROUNDED_CASE15_SELECT_2C3 |
            BUBSY_GROUNDED_CASE15_CALL_52E18 |
            BUBSY_GROUNDED_CASE15_REQUEST_MOVE;
    }
    if (sequence_boundary_reached == 1) {
        return BUBSY_GROUNDED_CASE15_CALL_2262C_07 |
            BUBSY_GROUNDED_CASE15_CALL_3539C_1;
    }
    return BUBSY_GROUNDED_CASE15_REQUEST_MOVE;
}

BubsyGroundedCase6Action bubsy_grounded_case6_select_action(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t *descriptor_state_4C
) {
    if ((actor_flags_04 & 0x4u) != 0 ||
        (actor_state_10 & 0x82000u) != 0 ||
        (actor_flags_04 & 0x1140u) != 0) {
        return BUBSY_GROUNDED_CASE6_NO_ACTION;
    }
    *descriptor_state_4C = 1;
    if ((actor_state_10 & 0x44u) != 0) {
        return BUBSY_GROUNDED_CASE6_CALL_358C8_MODE_0;
    }
    if ((actor_state_10 & 0x01020080u) == 0) {
        return BUBSY_GROUNDED_CASE6_CALL_35ACC;
    }
    return BUBSY_GROUNDED_CASE6_NO_ACTION;
}

uint8_t bubsy_grounded_case20_select_actions(
    uint8_t descriptor_state_0D,
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    int32_t current_sequence_key
) {
    uint8_t actions;

    if (descriptor_state_0D != 0 || actor_state_10 != 0 ||
        (actor_flags_04 & 0x1100u) != 0) {
        return BUBSY_GROUNDED_CASE20_NO_ACTION;
    }
    actions = BUBSY_GROUNDED_CASE20_CALL_516F8_54AF0 |
        BUBSY_GROUNDED_CASE20_CALL_22650_0C;
    if ((actor_flags_04 & 0x80u) == 0 && current_sequence_key != 0x349) {
        actions |= BUBSY_GROUNDED_CASE20_SELECT_349;
    }
    return actions;
}

BubsyGroundedCase31Path bubsy_grounded_case31_advance_counter(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t descriptor_state_4C,
    uint8_t *descriptor_counter_4D
) {
    uint8_t previous = *descriptor_counter_4D;

    if ((actor_flags_04 & 0x1104u) != 0 ||
        (actor_state_10 & 0xA0080u) != 0 || descriptor_state_4C != 0) {
        return BUBSY_GROUNDED_CASE31_SKIP;
    }
    if ((actor_flags_04 & 0x40u) != 0 || (actor_state_10 & 0x1044u) != 0) {
        return BUBSY_GROUNDED_CASE31_ALTERNATE;
    }
    *descriptor_counter_4D = (uint8_t)(previous + 1);
    return previous < 11 ?
        BUBSY_GROUNDED_CASE31_COUNTER_BELOW_11 :
        BUBSY_GROUNDED_CASE31_COUNTER_SEQUENCE_PATH;
}

BubsyGroundedCase31Actions bubsy_grounded_case31_select_alternate_action(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    int16_t *descriptor_state_02
) {
    if ((actor_flags_04 & 0x40u) == 0 && (actor_state_10 & 0x1044u) == 0) {
        return BUBSY_GROUNDED_CASE31_NO_ACTION;
    }
    if ((actor_state_10 & 0x1000u) != 0) {
        if (*descriptor_state_02 != 0) {
            return BUBSY_GROUNDED_CASE31_NO_ACTION;
        }
        *descriptor_state_02 = 2;
        return BUBSY_GROUNDED_CASE31_SET_DESCRIPTOR_02_TO_2;
    }
    if ((actor_state_10 & 0x44u) != 0 &&
        (actor_state_10 & 0x10000u) == 0 &&
        (actor_flags_04 & 0x40u) == 0) {
        return BUBSY_GROUNDED_CASE31_CALL_358C8_MODE_1;
    }
    return BUBSY_GROUNDED_CASE31_NO_ACTION;
}

uint8_t bubsy_grounded_case31_select_sequence_actions(
    int32_t current_sequence_key,
    int32_t *linked_counter_30
) {
    uint8_t actions = BUBSY_GROUNDED_CASE31_SET_LINKED_30_TO_MINUS_11 |
        BUBSY_GROUNDED_CASE31_CALL_22944;

    *linked_counter_30 = -0x11;
    if (current_sequence_key != 0) {
        actions |= BUBSY_GROUNDED_CASE31_CALL_52AEC_0 |
            BUBSY_GROUNDED_CASE31_CALL_52E18 |
            BUBSY_GROUNDED_CASE31_CALL_22650_17;
    }
    return actions;
}

BubsyGroundedCases8_10Action bubsy_grounded_cases8_10_select_action(
    uint16_t state_id,
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t actor_state_01,
    int16_t *descriptor_state_02
) {
    if ((state_id != 8 && state_id != 10) ||
        (actor_flags_04 & 0x4u) != 0 ||
        (actor_state_10 & 0x82000u) != 0 ||
        (actor_flags_04 & 0x1100u) != 0) {
        return BUBSY_GROUNDED_CASE8_10_EPILOGUE;
    }
    if ((actor_state_10 & 0x1000u) != 0) {
        if (*descriptor_state_02 != 0 || actor_state_01 == 1) {
            return BUBSY_GROUNDED_CASE8_10_EPILOGUE;
        }
        *descriptor_state_02 = state_id == 8 ? 3 : 4;
        return BUBSY_GROUNDED_CASE8_10_SET_DESCRIPTOR_STATE;
    }
    if ((actor_state_10 & 0x8104u) == 4) {
        return state_id == 8 ?
            BUBSY_GROUNDED_CASE8_CALL_358C8_MODE_3 :
            BUBSY_GROUNDED_CASE10_CALL_358C8_MODE_2;
    }
    if ((actor_state_10 & 0x80u) != 0) {
        return BUBSY_GROUNDED_CASE8_10_EPILOGUE;
    }
    return BUBSY_GROUNDED_CASE8_10_CONTINUE_COUNTER_PATH;
}

BubsyGroundedCases8_10CounterActions bubsy_grounded_cases8_10_update_counter_path(
    uint16_t state_id,
    uint32_t actor_state_10,
    uint8_t descriptor_state_11,
    int32_t *global_gate_540,
    uint32_t *global_state_544,
    int16_t *global_counter_53C
) {
    BubsyGroundedCases8_10CounterActions actions =
        BUBSY_GROUNDED_CASE8_10_CLEAR_GP_540 |
        BUBSY_GROUNDED_CASE8_10_INCREMENT_GP_COUNTER |
        BUBSY_GROUNDED_CASE8_10_CALL_34EF8;
    int32_t next_counter;

    if ((state_id != 8 && state_id != 10) ||
        (actor_state_10 & 0x80u) != 0 || descriptor_state_11 == 5) {
        return BUBSY_GROUNDED_CASE8_10_COUNTER_NO_ACTION;
    }
    if (*global_gate_540 != 0) {
        actions |= BUBSY_GROUNDED_CASE8_10_CALL_4FFEC;
    }
    *global_gate_540 = 0;
    if (*global_state_544 != state_id) {
        *global_state_544 = state_id;
        *global_counter_53C = 0;
        actions |= BUBSY_GROUNDED_CASE8_10_RESET_GP_COUNTER;
    }
    next_counter = (int32_t)*global_counter_53C + 1;
    if (next_counter > INT16_MAX) {
        next_counter -= 0x10000;
    }
    *global_counter_53C = (int16_t)next_counter;
    if (next_counter >= 0x29) {
        *global_counter_53C = 0x28;
        actions |= BUBSY_GROUNDED_CASE8_10_CLAMP_GP_COUNTER;
    }
    return actions;
}

uint8_t bubsy_grounded_case32_update_state(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t *actor_state_01,
    uint8_t *descriptor_counter_4D,
    uint8_t *descriptor_state_4C,
    uint8_t *descriptor_state_68,
    int16_t *descriptor_state_02,
    int32_t *linked_counter_30
) {
    uint8_t actions = BUBSY_GROUNDED_CASE32_NO_ACTION;

    if ((actor_state_10 & 0x20000u) != 0 || (actor_flags_04 & 0x40u) != 0) {
        return actions;
    }
    if ((actor_state_10 & 0x1000u) != 0) {
        *descriptor_state_02 = 0;
        return BUBSY_GROUNDED_CASE32_CLEAR_DESCRIPTOR_02;
    }
    if ((actor_state_10 & 0x44u) != 0) {
        *descriptor_state_4C = 0;
        *descriptor_state_68 = 1;
        return BUBSY_GROUNDED_CASE32_CALL_2262C_17;
    }

    if (*descriptor_counter_4D < 11 && (actor_state_10 & 0x01002000u) == 0) {
        *linked_counter_30 = 0;
        *actor_state_01 = 2;
        *descriptor_counter_4D = 0;
        actions = BUBSY_GROUNDED_CASE32_CALL_22650_10 |
            BUBSY_GROUNDED_CASE32_CALL_347EC_1;
    } else {
        *descriptor_counter_4D = 0;
        actions = BUBSY_GROUNDED_CASE32_CALL_2262C_17 |
            BUBSY_GROUNDED_CASE32_CALL_3539C_1;
    }
    return actions;
}

BubsyGroundedCase7Actions bubsy_grounded_case7_update_state(
    uint32_t actor_state_10,
    uint8_t *actor_state_01,
    uint8_t *descriptor_state_4C,
    uint8_t *descriptor_state_68,
    int16_t *descriptor_state_02
) {
    if ((actor_state_10 & 0x1000u) != 0) {
        *actor_state_01 = 2;
        *descriptor_state_02 = 0;
        return BUBSY_GROUNDED_CASE7_SET_ACTOR_STATE;
    }
    *descriptor_state_4C = 0;
    *descriptor_state_68 = 1;
    return BUBSY_GROUNDED_CASE7_CALL_2269C_0F;
}

uint8_t bubsy_grounded_cases9_11_update_state(
    uint16_t state_id,
    int32_t current_sequence_key,
    uint32_t *actor_state_10,
    uint32_t *actor_state_14,
    uint8_t *actor_state_01,
    int16_t *descriptor_state_02,
    int32_t global_540_nonzero,
    int32_t global_53C_nonzero
) {
    uint8_t actions = BUBSY_GROUNDED_CASE9_11_NO_ACTION;
    uint32_t previous_state_10;
    int32_t sequence_matches;

    if (state_id != 9 && state_id != 11) {
        return actions;
    }
    if ((*actor_state_10 & 0x1000u) != 0) {
        *actor_state_01 = 2;
        *descriptor_state_02 = 0;
        return BUBSY_GROUNDED_CASE9_11_SET_ACTOR_STATE;
    }

    sequence_matches = state_id == 9 ?
        current_sequence_key == 0x2BB : current_sequence_key == 0x2B8;
    if (sequence_matches) {
        actions |= BUBSY_GROUNDED_CASE9_11_SELECT_2B5 |
            BUBSY_GROUNDED_CASE9_11_CALL_52E18;
    }

    previous_state_10 = *actor_state_10;
    *actor_state_10 &= ~0x500u;
    if ((previous_state_10 & 0x220u) != 0 && global_53C_nonzero != 0) {
        if (global_540_nonzero == 0) {
            actions |= BUBSY_GROUNDED_CASE9_11_CALL_4FF50;
        }
        actions |= BUBSY_GROUNDED_CASE9_11_CALL_34EF8;
    } else {
        *actor_state_14 = 0;
        *actor_state_10 &= ~0x220u;
        actions |= BUBSY_GROUNDED_CASE9_11_RESET_AND_CALL_3539C_1;
    }
    return actions;
}

BubsyGroundedCases33_35Actions bubsy_grounded_cases33_35_prepare(
    uint16_t state_id,
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t actor_state_01,
    uint16_t *descriptor_state_00,
    int32_t *linked_counter_30
) {
    if ((state_id != 33 && state_id != 35) ||
        (actor_flags_04 & 0x4u) != 0 ||
        (actor_state_10 & 0x01080000u) != 0 ||
        (actor_flags_04 & 0x1140u) != 0 ||
        actor_state_10 != 0 ||
        actor_state_01 == 1) {
        return BUBSY_GROUNDED_CASE33_35_NO_ACTION;
    }
    *descriptor_state_00 = state_id == 33 ? 3 : 2;
    *linked_counter_30 = 0;
    return BUBSY_GROUNDED_CASE33_35_CALL_22650_0F |
        BUBSY_GROUNDED_CASE33_35_CALL_22650_08 |
        BUBSY_GROUNDED_CASE33_35_CALL_347EC_1;
}

BubsyActorDefaultStateRoute bubsy_actor_state_route_default(
    const BubsyActorUpdateActorView *actor,
    uint16_t handler_state_id,
    int32_t handler_state_aux,
    uint8_t runtime_state_6461,
    uint8_t update_lock_6479,
    int16_t update_gate_6454,
    uint8_t runtime_mode
) {
    if (runtime_state_6461 != 5 ||
        (actor->update_state_10 & 0x00010000u) != 0 ||
        update_lock_6479 != 0 ||
        update_gate_6454 != 0 ||
        (actor->flags_04 & 0x00000100u) != 0) {
        return BUBSY_ACTOR_DEFAULT_ROUTE_SKIP;
    }

    if (handler_state_id == 0x25 ||
        (handler_state_id == 0x26 && handler_state_aux != -2)) {
        return runtime_mode == 0 ?
            BUBSY_ACTOR_DEFAULT_ROUTE_GROUNDED : BUBSY_ACTOR_DEFAULT_ROUTE_SWIM;
    }

    return BUBSY_ACTOR_DEFAULT_ROUTE_SKIP;
}

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
#include "player_model.h"

typedef struct SharedActorDispatchTarget {
    uint8_t unknown_00_to_03[4];
    uint32_t flags_04;
    uint8_t unknown_08_to_0F[8];
    uint32_t state_10;
    uint8_t unknown_14_to_1B[8];
    void *component_1C;
    uint8_t unknown_20_to_27[8];
    PlayerActorSharedStateView *shared_28;
} SharedActorDispatchTarget;

_Static_assert(offsetof(SharedActorDispatchTarget, component_1C) == 0x1C, "actor component offset must match executable");
_Static_assert(offsetof(SharedActorDispatchTarget, shared_28) == 0x28, "shared state pointer offset must match executable");

extern int32_t func_80052ABC(void *component, int32_t *sequence_key_out);
extern void func_800546F4(void *context, void *plan);
extern void func_800547C8(void *context, void *plan);
extern void func_80022944(void *actor);
extern void func_800549E8(void *context, void *plan);

void shared_actor_state_dispatch(
    SharedActorDispatchTarget *actor,
    void *context,
    int32_t mode
) {
    register SharedActorDispatchTarget *actor_value __asm__("$18") = actor;
    register void *context_value __asm__("$19") = context;
    register int32_t mode_value __asm__("$17") = mode;
    register void *component_value __asm__("$4") = actor_value->component_1C;
    register PlayerActorSharedStateView *shared __asm__("$16") = actor_value->shared_28;
    register volatile uint8_t *runtime_globals __asm__("$28");
    register int32_t saved_progress_30 __asm__("$20");
    register int32_t saved_progress_34 __asm__("$21");
    int32_t sequence_key;
    uint32_t plan_storage[12];

    __asm__ volatile ("" : : "r"(actor_value));
    __asm__ volatile ("" : : "r"(context_value));
    __asm__ volatile ("" : : "r"(mode_value));
    func_80052ABC(component_value, &sequence_key);
    if (sequence_key == 0x2A7 || (actor_value->flags_04 & 0x1100u) != 0) {
        shared->progress_30 = 0;
        shared->progress_34 = 0;
        return;
    }

    saved_progress_34 = shared->progress_34;
    if (mode_value == 1) {
        goto mode_one;
    }
    if (mode_value < 2) {
        if (mode_value == 0) {
            goto mode_zero;
        }
        shared->progress_34 = saved_progress_34;
        goto epilogue;
    }
    if (mode_value >= 4) {
        goto epilogue;
    }
    goto mode_two_or_three;

mode_zero:
    if ((actor_value->state_10 & 0x18000u) != 0) {
        goto epilogue;
    }
    if (shared->progress_30 < 5) {
        shared->progress_30 = 5;
    }
    if (shared->progress_30 >= 0x11) {
        shared->progress_30 = 0x11;
        shared->progress_34 = 0;
    } else {
        shared->progress_34 = 0x198C;
    }
    goto mode_callback;

mode_one:
    if ((actor_value->state_10 & 0x8000u) != 0) {
        goto epilogue;
    }
    if ((actor_value->state_10 & 0x10000u) != 0) {
        shared->progress_30 = -0x34;
    } else if (shared->progress_30 < -0x11) {
        shared->progress_34 = 0;
    } else {
        shared->progress_34 = -0x198C;
    }

mode_callback:
    func_80022944(actor_value);
    shared->progress_34 = saved_progress_34;
    goto epilogue;

mode_two_or_three:
    if ((actor_value->state_10 & 0x10000u) == 0) {
        saved_progress_30 = shared->progress_30;
        plan_storage[10] = 0;
        plan_storage[8] = 0;
        func_800546F4(context_value, plan_storage);
        plan_storage[9] = mode_value == 3 ? 0x400 : -0x400;
        func_800547C8(context_value, plan_storage + 8);
        if ((actor_value->state_10 & 0x8000u) == 0) {
            shared->progress_30 = 0xE;
        } else if (runtime_globals[0x7EC] != 0) {
            shared->progress_30 = 6;
        } else {
            shared->progress_30 = 8;
        }
        func_80022944(actor_value);
        func_800549E8(context_value, plan_storage);
        shared->progress_30 = saved_progress_30;
        shared->progress_34 = saved_progress_34;
    }

epilogue:
    return;
}
#endif

BubsyActorUpdateEntryResult bubsy_actor_update_entry_gate(
    BubsyActorUpdateActorView *actor,
    uint8_t update_mode,
    BubsyActorUpdateEnvironment *environment,
    uint8_t *local_state_24
) {
    if (environment->update_gate_6454 != 0) {
        *local_state_24 = 0;
        actor->update_state_10 = 0;
    } else {
        *local_state_24 = update_mode;
        if (update_mode != 0) {
            environment->update_mode_647A = 0;
            environment->sequence_counter_38C = 0;
        }
    }

    if (actor->update_state_10 != 0) {
        return BUBSY_ACTOR_UPDATE_SKIP;
    }

    environment->actor_event_guard_6460 = 0;
    return BUBSY_ACTOR_UPDATE_CONTINUE;
}

int32_t bubsy_actor_update_should_select_309(
    const BubsyActorUpdateActorView *actor,
    int32_t current_sequence_key,
    uint8_t runtime_mode
) {
    return (actor->flags_04 & 0x80u) != 0 &&
        runtime_mode == 0 &&
        current_sequence_key != 0x309 &&
        (actor->update_state_10 & 0x00800000u) == 0;
}

uint8_t bubsy_actor_update_local_state_for_sequence(
    const BubsyActorUpdateActorView *actor,
    int32_t current_sequence_key,
    uint8_t runtime_mode,
    uint8_t local_state_24
) {
    if ((actor->flags_04 & 0x100u) != 0) {
        return runtime_mode == 0 ? 0 : local_state_24;
    }

    if (current_sequence_key == 0x32E && runtime_mode == 0) {
        return 1;
    }

    return local_state_24;
}

int32_t bubsy_actor_update_should_check_mode1_sequence_boundary(
    int32_t current_sequence_key,
    uint8_t runtime_mode
) {
    return runtime_mode == 1 &&
        (current_sequence_key == 0x9A || current_sequence_key == 0x54);
}

int32_t bubsy_actor_update_should_reset_sequence_counter(
    const BubsyActorUpdateActorView *actor,
    int32_t sequence_counter_after_increment,
    uint8_t runtime_mode,
    int16_t level_id
) {
    return (runtime_mode == 1 && sequence_counter_after_increment >= 12) ||
        level_id == 0x14 ||
        (actor->flags_04 & 0x40u) != 0;
}

int32_t bubsy_actor_update_should_enter_level14_counter_path(
    const BubsyActorUpdateActorView *actor,
    int32_t sequence_counter,
    uint8_t runtime_mode,
    int16_t level_id,
    int16_t update_gate
) {
    return runtime_mode == 0 &&
        sequence_counter > 0 &&
        level_id == 0x14 &&
        (actor->flags_04 & 0x40u) == 0 &&
        update_gate == 0;
}

int32_t bubsy_actor_update_sequence_index_for_local_state(
    const BubsyActorUpdateActorView *actor,
    uint8_t local_state_24,
    uint8_t runtime_mode
) {
    if (local_state_24 != 1) {
        return -1;
    }

    if (runtime_mode == 0) {
        return 0x35;
    }

    return (actor->flags_04 & 0x818u) != 0 ? 0x9A : 0x54;
}

int32_t bubsy_actor_update_get_sequence_mode_override(
    uint8_t local_state_24,
    uint8_t runtime_mode,
    int32_t *override_out
) {
    if (local_state_24 != 1 || runtime_mode == 0) {
        return 0;
    }

    *override_out = -0xF40;
    return 1;
}
