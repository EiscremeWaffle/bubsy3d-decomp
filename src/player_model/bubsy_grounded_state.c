#include "bubsy_grounded_state.h"

typedef union BubsyGroundedFlags {
    uint32_t word;
    struct {
        unsigned int low_5 : 5;
        unsigned int bit_5 : 1;
        unsigned int middle_2 : 2;
        unsigned int bit_8 : 1;
        unsigned int bit_9 : 1;
        unsigned int bit_10 : 1;
        unsigned int high_21 : 21;
    } bits;
} BubsyGroundedFlags;

extern int16_t g_grounded_counter;
extern void *g_grounded_event;
extern int32_t g_grounded_counter_state;
extern int32_t g_grounded_reference_vector[4];
extern int32_t g_grounded_current_vector[4];
extern const char g_grounded_error_none[];
extern const char g_grounded_source_path[];

extern void func_8003AA1C(BubsyGroundedDescriptor **output);
extern void *func_8001DCE0(void);
extern void func_800516F8(void **output);
extern void func_80054AF0(void *component, int32_t *output);
extern void func_8005395C(const int32_t *input, int32_t *output);
extern void func_800100A0(const int32_t *left, const int32_t *right, int32_t *output);
extern void func_80052ABC(void *component, int32_t *output);
extern int32_t func_80052AEC(void *component, int32_t property);
extern void func_80052E18(void *component);
extern void func_8005290C(void *component, uint8_t *output);
extern void func_8002262C(BubsyGroundedActor *actor, int32_t selector);
extern void func_80022650(BubsyGroundedActor *actor, int32_t selector);
extern void func_8002269C(BubsyGroundedActor *actor, int32_t selector);
extern void func_8002E7D8(void);
extern void func_80037B30(int32_t mode);
extern void func_80039158(BubsyGroundedActor *actor, void *context);
extern void func_800347EC(BubsyGroundedActor *actor, void *context, int32_t mode);
extern void func_800358C8(BubsyGroundedActor *actor, void *context, int32_t mode);
extern void func_80035ACC(BubsyGroundedActor *actor, void *context);
extern void func_8001C700(int32_t selection, int32_t state);
extern void func_8001B558(int32_t state, int32_t mode, uint32_t flags, uint32_t heading,
    int32_t argument_4, int32_t argument_5, int32_t argument_6, int32_t argument_7);
extern void func_800556D0(int32_t level, const char *condition, const char *source, uint32_t line);
extern void func_80022944(BubsyGroundedActor *actor);
extern void func_8003539C(BubsyGroundedActor *actor, int32_t mode);
extern void func_8004FFEC(void *event);
extern void func_8004FF50(void (*callback)(BubsyGroundedActor *, BubsyGroundedState *, void *, int32_t),
    int32_t mode, void **output);
extern void func_80034EF8(BubsyGroundedActor *actor, BubsyGroundedState *state, void *context, int32_t counter);
extern void func_8004B2A4(BubsyGroundedActor *actor, BubsyGroundedState *state, void *context, int32_t counter);

void bubsy_grounded_state_handler(
    BubsyGroundedState *state,
    BubsyGroundedActor *actor,
    void *context_argument,
    BubsyGroundedLookup *lookup_argument,
    BubsyGroundedShared *shared
) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    /* Preserve the original incoming argument allocation before the descriptor call. */
    register void *context __asm__("$19") = context_argument;
    register BubsyGroundedLookup *lookup __asm__("$20") = lookup_argument;
#else
    void *context = context_argument;
    BubsyGroundedLookup *lookup = lookup_argument;
#endif
    BubsyGroundedDescriptor *descriptor;
    int32_t lookup_value;
    void *reference_component;
    int32_t reference_sequence;
    void *current_component;
    int32_t dot;
    int32_t transition_sequence;
    uint8_t boundary;
    int32_t counter_sequence;
    int32_t release_sequence;
    BubsyGroundedActor *companion;
    uint8_t skip = 1;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__("" : "=r"(context), "=r"(lookup) : "0"(context), "1"(lookup));
#endif
    func_8003AA1C(&descriptor);
    lookup_value = lookup->words[lookup->fields.index_14 + 1];
    if (lookup_value == 3) {
        switch (state->id) {
        case 14:
            if ((actor->flags_04 & 0x1100) == 0) {
                if ((actor->state_10 & 0x1000) != 0) {
                    if (descriptor->state_02 == 0) {
                        descriptor->state_02 = 1;
                    }
                } else {
                    skip = 0;
                }
            }
            break;
        case 15:
            if ((actor->state_10 & 0x1000) != 0) {
                descriptor->state_02 = 0;
            } else {
                skip = 0;
            }
            break;
        default:
            skip = 0;
            break;
        }
    } else if ((uint32_t)lookup_value < 2 || lookup_value == 2) {
        if (state->id == 6) {
            if ((actor->flags_04 & 0x1100) == 0) {
                if ((actor->state_10 & 0x1000) != 0) {
                    if (descriptor->state_02 == 0) {
                        descriptor->state_02 = 1;
                    }
                } else {
                    skip = 0;
                }
            }
        } else {
            skip = 0;
        }
    }
    if (skip) {
        return;
    }

    switch (state->id) {
    case 20:
        if (lookup->fields.gate_0D != 0 || actor->state_10 != 0 || (actor->flags_04 & 0x1100) != 0) {
            break;
        }
        func_800516F8(&reference_component);
        func_80054AF0(reference_component, g_grounded_reference_vector);
        func_80052ABC(actor->component_1C, &reference_sequence);
        func_80022650(actor, 12);
        if ((actor->flags_04 & 0x80) == 0 && reference_sequence != 0x349) {
            func_80052AEC(actor->component_1C, 0x349);
        }
        break;
    case 21:
        companion = func_8001DCE0();
        func_800516F8(&current_component);
        func_80054AF0(current_component, g_grounded_current_vector);
        func_8005395C(g_grounded_reference_vector, g_grounded_reference_vector);
        func_8005395C(g_grounded_current_vector, g_grounded_current_vector);
        func_800100A0(g_grounded_reference_vector, g_grounded_current_vector, &dot);
        if ((actor->state_10 & 0x1000) != 0 && (actor->flags_04 & 0x40) == 0 &&
            (companion == 0 || companion->shared_28 == 0) && dot < 0xCCC &&
            descriptor->state_02 != 5) {
            func_8002E7D8();
            descriptor->state_02 = 0;
            func_80022650(actor, 8);
            goto dispatch_shared;
        }
        func_8002262C(actor, 12);
        goto clear_descriptor_state;
    case 37: {
        uint8_t descriptor_state;
        if ((actor->flags_04 & 0x1100) != 0) {
            break;
        }
        descriptor_state = descriptor->state_11;
        if (descriptor_state == 0) {
            descriptor->state_11 = 1;
            func_80037B30(0);
            break;
        }
        if (descriptor_state == 5) {
            break;
        }
        goto process_descriptor;
    }
    case 38: {
        BubsyGroundedDescriptor *current_descriptor = descriptor;
        int32_t descriptor_state = current_descriptor->state_11;
        if (descriptor_state == 2) {
            current_descriptor->state_11 = 4;
        } else if (descriptor_state == 1) {
            current_descriptor->state_11 = 0;
            func_8002262C(actor, 17);
        }
process_descriptor:
        func_80039158(actor, context);
        break;
    }
    case 12:
        if ((actor->flags_04 & 0x1104) != 0 || (actor->flags_04 & 0x818) == 0 ||
            (actor->state_10 & 0xA0084) != 0) {
            break;
        }
        if ((actor->state_10 & 0x1000) != 0) {
            func_8002262C(actor, 12);
            descriptor->state_02 = 5;
        }
        if (actor->state_01 != 1) {
            actor->state_01 = 1;
            goto dispatch_shared;
        }
        break;
    case 13:
        actor->state_01 = 2;
        break;
    case 16:
        if (descriptor->state_14 == 0 && (actor->flags_04 & 2) != 0) {
            break;
        }
        descriptor->state_14 = 1;
        if ((actor->flags_04 & 0x1104) == 0 && (actor->state_10 & 0xA9180) == 0 &&
            actor->state_01 != 1) {
            func_800347EC(actor, context, 0);
        }
        break;
    case 17:
        descriptor->state_14 = 0;
        actor->state_01 = 2;
        func_8001C700(4, 3);
        if ((actor->state_10 & 0x44) != 0) {
            func_80022650(actor, 2);
            actor->state_00 = 0;
            goto dispatch_shared;
        }
        break;
    case 14:
        if ((actor->state_10 & 0x10000C4) != 0 || (actor->flags_04 & 0x818) == 0 ||
            descriptor->state_11 == 2 || (actor->flags_04 & 0x1104) != 0) {
            break;
        }
        actor->state_10 = 0;
        actor->shared_28->stride_30 = 0;
        func_80022650(actor, 7);
        if (func_80052AEC(actor->component_1C, 0x2BE) != 0) {
            func_800556D0(1, g_grounded_error_none, g_grounded_source_path, 0x1B8);
        }
        func_80052E18(actor->component_1C);
        break;
    case 15:
        func_80052ABC(actor->component_1C, &transition_sequence);
        if (transition_sequence == 0x2BE) {
            if (func_80052AEC(actor->component_1C, 0x2C3) != 0) {
                func_800556D0(1, g_grounded_error_none, g_grounded_source_path, 0x1C7);
            }
            func_80052E18(actor->component_1C);
        } else {
            func_8005290C(actor->component_1C, &boundary);
            if (boundary == 1) {
                func_8002262C(actor, 7);
                func_8003539C(actor, 1);
                break;
            }
        }
        func_8001B558(15, 4, 0xFFFE, actor->heading_0C, 0, 0, 0, 0);
        break;
    case 6: {
        uint32_t actor_state;
        if ((actor->flags_04 & 4) != 0 || (actor->state_10 & 0x82000) != 0 ||
            (actor->flags_04 & 0x1140) != 0) {
            break;
        }
        descriptor->state_4C = 1;
        actor_state = actor->state_10;
        if ((actor_state & 0x44) != 0) {
            func_800358C8(actor, context, 0);
        } else if ((actor_state & 0x1020080) == 0) {
            func_80035ACC(actor, context);
        }
        break;
    }
    case 29:
        descriptor->state_10 = 1;
        break;
    case 30:
        descriptor->state_10 = 0;
        break;
    case 31: {
        uint32_t actor_state;
        if ((actor->flags_04 & 0x1104) != 0 || (actor->state_10 & 0xA0080) != 0 ||
            descriptor->state_4C != 0) {
            break;
        }
        actor_state = actor->state_10;
        if ((actor->flags_04 & 0x40) == 0 && (actor_state & 0x1044) == 0) {
            uint8_t previous = descriptor->counter_4D++;
            if (previous >= 11) {
                func_80052ABC(actor->component_1C, &counter_sequence);
                if (counter_sequence != 0) {
                    func_80052AEC(actor->component_1C, 0);
                    func_80052E18(actor->component_1C);
                }
                func_80022650(actor, 23);
                shared->stride_30 = -17;
                func_80022944(actor);
                break;
            }
            actor_state = actor->state_10;
        }
        if ((actor_state & 0x1000) != 0) {
            if (descriptor->state_02 == 0) {
                descriptor->state_02 = 2;
            }
        } else if ((actor_state & 0x44) != 0 && (actor_state & 0x10000) == 0 &&
            (actor->flags_04 & 0x40) == 0) {
            func_800358C8(actor, context, 1);
        }
        break;
    }
    case 32: {
        uint32_t actor_state = actor->state_10;
        if ((actor_state & 0x20000) == 0 && (actor->flags_04 & 0x40) == 0) {
            if ((actor_state & 0x1000) != 0) {
                goto clear_descriptor_state;
            }
            if ((actor_state & 0x44) != 0) {
                func_8002262C(actor, 23);
                goto set_descriptor_68;
            }
            if (descriptor->counter_4D < 11 && (actor_state & 0x1002000) == 0) {
                shared->stride_30 = 0;
                func_80022650(actor, 16);
                func_800347EC(actor, context, 1);
                actor->state_01 = 2;
            } else {
                func_8002262C(actor, 23);
                func_8003539C(actor, 1);
            }
            descriptor->counter_4D = 0;
        } else {
            actor_state &= 0x21000;
            if (actor_state == 0x1000) {
                goto clear_descriptor_state;
            }
        }
        break;
    }
    case 8:
    case 10: {
        uint32_t actor_state;
        if ((actor->flags_04 & 4) != 0) {
            break;
        }
        actor_state = actor->state_10;
        if ((actor_state & 0x82000) != 0 || (actor->flags_04 & 0x1100) != 0) {
            break;
        }
        if ((actor_state & 0x1000) != 0) {
            BubsyGroundedDescriptor *current_descriptor = descriptor;
            if (current_descriptor->state_02 == 0 && actor->state_01 != 1) {
                if (state->id == 8) {
                    current_descriptor->state_02 = 3;
                } else {
                    current_descriptor->state_02 = 4;
                }
            }
            break;
        }
        if ((actor_state & 0x8104) == 4) {
            if (state->id == 8) {
                func_800358C8(actor, context, 3);
            } else {
                func_800358C8(actor, context, 2);
            }
            break;
        }
        if ((actor_state & 0x80) != 0 || descriptor->state_11 == 5) {
            break;
        }
        if (g_grounded_event != 0) {
            func_8004FFEC(g_grounded_event);
            g_grounded_event = 0;
        }
        if (g_grounded_counter_state != state->id) {
            g_grounded_counter = 0;
            g_grounded_counter_state = state->id;
        }
        g_grounded_counter = (int16_t)(uint16_t)((uint16_t)g_grounded_counter + 1u);
        if (g_grounded_counter > 40) {
            g_grounded_counter = 40;
        }
        func_80034EF8(actor, state, context, g_grounded_counter);
        break;
    }
    case 33:
    case 35:
        if ((actor->flags_04 & 4) != 0 || (actor->state_10 & 0x1080000) != 0 ||
            (actor->flags_04 & 0x1140) != 0 || actor->state_10 != 0 || actor->state_01 == 1) {
            break;
        }
        if (state->id == 33) {
            descriptor->state_00 = 3;
        } else {
            descriptor->state_00 = 2;
        }
        shared->stride_30 = 0;
        func_80022650(actor, 15);
        func_80022650(actor, 8);
dispatch_shared:
        func_800347EC(actor, context, 1);
        break;
    case 34:
    case 36:
        actor->state_01 = 2;
        descriptor->state_00 = 0;
        break;
    case 7:
    case 9:
    case 11:
        if ((actor->state_10 & 0x1000) != 0) {
            actor->state_01 = 2;
clear_descriptor_state:
            descriptor->state_02 = 0;
            break;
        }
        if (state->id == 7) {
            func_8002269C(actor, 15);
            descriptor->state_4C = 0;
set_descriptor_68:
            descriptor->state_68 = 1;
            break;
        }
        if (state->id != 9 && state->id != 11) {
            break;
        }
        func_80052ABC(actor->component_1C, &release_sequence);
        if ((state->id == 11 && release_sequence == 0x2B8) ||
            (state->id == 9 && release_sequence == 0x2BB)) {
            func_80052AEC(actor->component_1C, 0x2B5);
            func_80052E18(actor->component_1C);
        }
        {
            uint32_t previous = actor->state_10;
            BubsyGroundedFlags flags;
            flags.word = previous;
            flags.bits.bit_8 = 0;
            flags.bits.bit_10 = 0;
            actor->state_10 = flags.word;
            if ((previous & 0x220) != 0) {
                if (g_grounded_event == 0 && g_grounded_counter != 0) {
                    func_8004FF50(func_8004B2A4, 1, &g_grounded_event);
                }
                if (g_grounded_counter != 0) {
                    func_80034EF8(actor, state, context, g_grounded_counter);
                    break;
                }
            }
        }
        {
            BubsyGroundedFlags flags;
            flags.word = actor->state_10;
            actor->state_14 = 0;
            flags.bits.bit_9 = 0;
            flags.bits.bit_5 = 0;
            actor->state_10 = flags.word;
        }
        func_8003539C(actor, 1);
        break;
    default:
        break;
    }

}
