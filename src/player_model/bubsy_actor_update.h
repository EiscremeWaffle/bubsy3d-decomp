#ifndef BUBSY3D_BUBSY_ACTOR_UPDATE_H
#define BUBSY3D_BUBSY_ACTOR_UPDATE_H

#include <stddef.h>
#include <stdint.h>

typedef struct BubsyActorUpdateActorView {
    uint8_t unknown_00_to_03[4];
    uint32_t flags_04;
    uint8_t unknown_08_to_0F[8];
    uint32_t update_state_10;
} BubsyActorUpdateActorView;

_Static_assert(offsetof(BubsyActorUpdateActorView, flags_04) == 0x04, "actor flags offset must match the executable");
_Static_assert(offsetof(BubsyActorUpdateActorView, update_state_10) == 0x10, "actor update state offset must match the executable");

typedef struct BubsyActorUpdateEnvironment {
    int16_t update_gate_6454;
    uint8_t update_mode_647A;
    uint8_t actor_event_guard_6460;
    int32_t sequence_counter_38C;
} BubsyActorUpdateEnvironment;

typedef enum BubsyActorUpdateEntryResult {
    BUBSY_ACTOR_UPDATE_SKIP = 0,
    BUBSY_ACTOR_UPDATE_CONTINUE = 1,
} BubsyActorUpdateEntryResult;

typedef enum BubsyActorDefaultStateRoute {
    BUBSY_ACTOR_DEFAULT_ROUTE_SKIP = 0,
    BUBSY_ACTOR_DEFAULT_ROUTE_GROUNDED = 1,
    BUBSY_ACTOR_DEFAULT_ROUTE_SWIM = 2,
} BubsyActorDefaultStateRoute;

typedef enum BubsyGroundedCase37Action {
    BUBSY_GROUNDED_CASE37_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE37_INITIALIZE = 1,
    BUBSY_GROUNDED_CASE37_PROCESS_ACTOR = 2,
} BubsyGroundedCase37Action;

typedef enum BubsyGroundedCase21Action {
    BUBSY_GROUNDED_CASE21_CALL_2262C_0C = 1,
    BUBSY_GROUNDED_CASE21_CALL_2E7D8 = 2,
    BUBSY_GROUNDED_CASE21_CALL_22650_08 = 4,
    BUBSY_GROUNDED_CASE21_CALL_347EC_1 = 8,
} BubsyGroundedCase21Action;

typedef enum BubsyGroundedCase38Actions {
    BUBSY_GROUNDED_CASE38_CALL_39158 = 1,
    BUBSY_GROUNDED_CASE38_CALL_2262C_11 = 2,
} BubsyGroundedCase38Actions;

typedef enum BubsyGroundedCase12Actions {
    BUBSY_GROUNDED_CASE12_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE12_CALL_2262C_0C = 1,
    BUBSY_GROUNDED_CASE12_CALL_347EC_1 = 2,
} BubsyGroundedCase12Actions;

typedef enum BubsyGroundedCase17Actions {
    BUBSY_GROUNDED_CASE17_CALL_1C700 = 1,
    BUBSY_GROUNDED_CASE17_CALL_22650_02 = 2,
    BUBSY_GROUNDED_CASE17_CALL_347EC_1 = 4,
} BubsyGroundedCase17Actions;

typedef enum BubsyGroundedCase15Actions {
    BUBSY_GROUNDED_CASE15_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE15_SELECT_2C3 = 1,
    BUBSY_GROUNDED_CASE15_CALL_52E18 = 2,
    BUBSY_GROUNDED_CASE15_REQUEST_MOVE = 4,
    BUBSY_GROUNDED_CASE15_CALL_2262C_07 = 8,
    BUBSY_GROUNDED_CASE15_CALL_3539C_1 = 16,
} BubsyGroundedCase15Actions;

typedef enum BubsyGroundedCase6Action {
    BUBSY_GROUNDED_CASE6_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE6_CALL_358C8_MODE_0 = 1,
    BUBSY_GROUNDED_CASE6_CALL_35ACC = 2,
} BubsyGroundedCase6Action;

typedef enum BubsyGroundedCase20Actions {
    BUBSY_GROUNDED_CASE20_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE20_CALL_516F8_54AF0 = 1,
    BUBSY_GROUNDED_CASE20_CALL_22650_0C = 2,
    BUBSY_GROUNDED_CASE20_SELECT_349 = 4,
} BubsyGroundedCase20Actions;

typedef enum BubsyGroundedCase31Path {
    BUBSY_GROUNDED_CASE31_SKIP = 0,
    BUBSY_GROUNDED_CASE31_ALTERNATE = 1,
    BUBSY_GROUNDED_CASE31_COUNTER_BELOW_11 = 2,
    BUBSY_GROUNDED_CASE31_COUNTER_SEQUENCE_PATH = 3,
} BubsyGroundedCase31Path;

typedef enum BubsyGroundedCase31Actions {
    BUBSY_GROUNDED_CASE31_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE31_SET_DESCRIPTOR_02_TO_2 = 1,
    BUBSY_GROUNDED_CASE31_CALL_358C8_MODE_1 = 2,
    BUBSY_GROUNDED_CASE31_CALL_52AEC_0 = 4,
    BUBSY_GROUNDED_CASE31_CALL_52E18 = 8,
    BUBSY_GROUNDED_CASE31_CALL_22650_17 = 16,
    BUBSY_GROUNDED_CASE31_SET_LINKED_30_TO_MINUS_11 = 32,
    BUBSY_GROUNDED_CASE31_CALL_22944 = 64,
} BubsyGroundedCase31Actions;

typedef enum BubsyGroundedCases8_10Action {
    BUBSY_GROUNDED_CASE8_10_EPILOGUE = 0,
    BUBSY_GROUNDED_CASE8_10_SET_DESCRIPTOR_STATE = 1,
    BUBSY_GROUNDED_CASE8_CALL_358C8_MODE_3 = 2,
    BUBSY_GROUNDED_CASE10_CALL_358C8_MODE_2 = 3,
    BUBSY_GROUNDED_CASE8_10_CONTINUE_COUNTER_PATH = 4,
} BubsyGroundedCases8_10Action;

typedef enum BubsyGroundedCases8_10CounterActions {
    BUBSY_GROUNDED_CASE8_10_COUNTER_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE8_10_CALL_4FFEC = 1,
    BUBSY_GROUNDED_CASE8_10_CLEAR_GP_540 = 2,
    BUBSY_GROUNDED_CASE8_10_RESET_GP_COUNTER = 4,
    BUBSY_GROUNDED_CASE8_10_INCREMENT_GP_COUNTER = 8,
    BUBSY_GROUNDED_CASE8_10_CLAMP_GP_COUNTER = 16,
    BUBSY_GROUNDED_CASE8_10_CALL_34EF8 = 32,
} BubsyGroundedCases8_10CounterActions;

typedef enum BubsyGroundedCase32Actions {
    BUBSY_GROUNDED_CASE32_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE32_CLEAR_DESCRIPTOR_02 = 1,
    BUBSY_GROUNDED_CASE32_CALL_2262C_17 = 2,
    BUBSY_GROUNDED_CASE32_CALL_22650_10 = 4,
    BUBSY_GROUNDED_CASE32_CALL_347EC_1 = 8,
    BUBSY_GROUNDED_CASE32_CALL_3539C_1 = 16,
} BubsyGroundedCase32Actions;

typedef enum BubsyGroundedCase14Actions {
    BUBSY_GROUNDED_CASE14_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE14_CALL_22650_07 = 1,
    BUBSY_GROUNDED_CASE14_SELECT_2BE = 2,
    BUBSY_GROUNDED_CASE14_ASSERT_ON_SELECTION_FAILURE = 4,
    BUBSY_GROUNDED_CASE14_CALL_52E18 = 8,
} BubsyGroundedCase14Actions;

typedef enum BubsyGroundedCases33_35Actions {
    BUBSY_GROUNDED_CASE33_35_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE33_35_CALL_22650_0F = 1,
    BUBSY_GROUNDED_CASE33_35_CALL_22650_08 = 2,
    BUBSY_GROUNDED_CASE33_35_CALL_347EC_1 = 4,
} BubsyGroundedCases33_35Actions;

typedef enum BubsyGroundedCase7Actions {
    BUBSY_GROUNDED_CASE7_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE7_SET_ACTOR_STATE = 1,
    BUBSY_GROUNDED_CASE7_CALL_2269C_0F = 2,
} BubsyGroundedCase7Actions;

typedef enum BubsyGroundedCases9_11Actions {
    BUBSY_GROUNDED_CASE9_11_NO_ACTION = 0,
    BUBSY_GROUNDED_CASE9_11_SET_ACTOR_STATE = 1,
    BUBSY_GROUNDED_CASE9_11_SELECT_2B5 = 2,
    BUBSY_GROUNDED_CASE9_11_CALL_52E18 = 4,
    BUBSY_GROUNDED_CASE9_11_CALL_4FF50 = 8,
    BUBSY_GROUNDED_CASE9_11_CALL_34EF8 = 16,
    BUBSY_GROUNDED_CASE9_11_RESET_AND_CALL_3539C_1 = 32,
} BubsyGroundedCases9_11Actions;

typedef enum BubsyActorSharedUpdateEntryAction {
    BUBSY_ACTOR_SHARED_UPDATE_ENTRY_RETURN = 0,
    BUBSY_ACTOR_SHARED_UPDATE_ENTRY_SET_PROPERTIES_2_6 = 1,
    BUBSY_ACTOR_SHARED_UPDATE_ENTRY_CONTINUE = 2,
} BubsyActorSharedUpdateEntryAction;

typedef enum BubsyActor347ECModeEntryAction {
    BUBSY_ACTOR_347EC_MODE_ZERO_PATH = 0,
    BUBSY_ACTOR_347EC_NONZERO_ALTERNATE_PATH = 1,
    BUBSY_ACTOR_347EC_CALL_2269C_01 = 2,
} BubsyActor347ECModeEntryAction;

typedef enum BubsyActor347ECNonzeroActions {
    BUBSY_ACTOR_347EC_NONZERO_CALL_22C94 = 1,
    BUBSY_ACTOR_347EC_NONZERO_CLEAR_ACTOR_00 = 2,
    BUBSY_ACTOR_347EC_NONZERO_CALL_2269C_01 = 4,
    BUBSY_ACTOR_347EC_NONZERO_CALL_54B20 = 8,
    BUBSY_ACTOR_347EC_NONZERO_PROPERTIES_3_4 = 16,
    BUBSY_ACTOR_347EC_NONZERO_CALL_1B558_27_04 = 32,
} BubsyActor347ECNonzeroActions;

typedef struct BubsyActor347ECArgumentPlan {
    uint8_t actions;
    int32_t argument_a1;
    int32_t argument_a2;
    int32_t argument_a3;
    int32_t stack_argument_10;
    int32_t stack_argument_14;
    int32_t stack_argument_18;
    uint8_t set_global_37D;
    uint8_t update_properties_3_4;
    uint8_t actor_state_44_path;
} BubsyActor347ECArgumentPlan;

typedef enum BubsyActor347ECModeZeroActions {
    BUBSY_ACTOR_347EC_MODE_ZERO_ALTERNATE = 0,
    BUBSY_ACTOR_347EC_MODE_ZERO_SET_ACTOR_STATE_00 = 1,
    BUBSY_ACTOR_347EC_MODE_ZERO_PROPERTIES_3_4 = 2,
    BUBSY_ACTOR_347EC_MODE_ZERO_CALL_22C94 = 4,
    BUBSY_ACTOR_347EC_MODE_ZERO_REQUEST_MOVE = 8,
    BUBSY_ACTOR_347EC_MODE_ZERO_CALL_2925C = 16,
    BUBSY_ACTOR_347EC_MODE_ZERO_CALL_26A0C = 32,
    BUBSY_ACTOR_347EC_MODE_ZERO_SET_ACTOR_00_AND_RETURN = 64,
} BubsyActor347ECModeZeroActions;

typedef enum BubsyActor347ECModeZeroBit40Actions {
    BUBSY_ACTOR_347EC_ZERO40_CLEAR_GLOBALS_64C0_37C_37E = 1,
    BUBSY_ACTOR_347EC_ZERO40_CALL_PROPERTIES_5_8_A = 2,
    BUBSY_ACTOR_347EC_ZERO40_CALL_PROPERTY_C = 4,
    BUBSY_ACTOR_347EC_ZERO40_CLEAR_GLOBAL_6452 = 8,
    BUBSY_ACTOR_347EC_ZERO40_CALL_2E7FC = 16,
    BUBSY_ACTOR_347EC_ZERO40_ASSERT_BOUND = 32,
    BUBSY_ACTOR_347EC_ZERO40_SET_GLOBAL_380 = 64,
    BUBSY_ACTOR_347EC_ZERO40_SET_ACTOR_STATE_01 = 128,
    BUBSY_ACTOR_347EC_ZERO40_REQUEST_MOVE_05 = 256,
    BUBSY_ACTOR_347EC_ZERO40_SET_GLOBAL_37C = 512,
    BUBSY_ACTOR_347EC_ZERO40_CLEAR_ACTOR_FLAGS_03 = 1024,
    BUBSY_ACTOR_347EC_ZERO40_CALL_PROPERTIES_F_10 = 4096,
    BUBSY_ACTOR_347EC_ZERO40_CLEAR_GLOBAL_6464 = 8192,
    BUBSY_ACTOR_347EC_ZERO40_SET_GLOBAL_64B8 = 16384,
    BUBSY_ACTOR_347EC_ZERO40_CALL_3539C_MODE_1 = 32768,
} BubsyActor347ECModeZeroBit40Actions;

typedef struct BubsyActor347ECModeZeroBit40Plan {
    uint32_t actions;
    uint32_t updated_global_380;
    uint8_t actor_state_01;
    uint32_t actor_flags_04;
} BubsyActor347ECModeZeroBit40Plan;

typedef enum BubsyActor347ECFollowupActions {
    BUBSY_ACTOR_347EC_FOLLOWUP_MOVE_REQUEST = 1,
    BUBSY_ACTOR_347EC_FOLLOWUP_DISPATCH_SHARED = 2,
    BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_GLOBAL_37D = 4,
    BUBSY_ACTOR_347EC_FOLLOWUP_TRANSFORM_CONTEXT = 8,
    BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_ACTOR_1000 = 16,
    BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_GLOBAL_6452 = 32,
    BUBSY_ACTOR_347EC_FOLLOWUP_UPDATE_LINKED_4C = 64,
    BUBSY_ACTOR_347EC_FOLLOWUP_RESET_GLOBAL_64BA = 128,
    BUBSY_ACTOR_347EC_FOLLOWUP_CLEAR_ACTOR_100 = 256,
    BUBSY_ACTOR_347EC_FOLLOWUP_ENTER_STATE_BIT4_CLEAR_BLOCK = 512,
} BubsyActor347ECFollowupActions;

typedef enum BubsyActor347ECModeZeroAlternateActions {
    BUBSY_ACTOR_347EC_ZERO_ALT_CALL_22C94 = 1,
    BUBSY_ACTOR_347EC_ZERO_ALT_CLEAR_64BC = 2,
    BUBSY_ACTOR_347EC_ZERO_ALT_RESET_GLOBALS = 4,
    BUBSY_ACTOR_347EC_ZERO_ALT_CALL_2944C = 8,
    BUBSY_ACTOR_347EC_ZERO_ALT_CALL_1B558_27_04 = 16,
    BUBSY_ACTOR_347EC_ZERO_ALT_CALL_2269C_01 = 32,
    BUBSY_ACTOR_347EC_ZERO_ALT_ASSERT = 64,
} BubsyActor347ECModeZeroAlternateActions;

typedef struct BubsyActor347ECModeZeroAlternatePlan {
    uint16_t actions;
    uint16_t argument_a1;
    uint16_t argument_a2;
    uint16_t argument_a3;
    int32_t stack_argument_10;
    int32_t stack_argument_14;
    int32_t stack_argument_18;
    uint8_t actor_state_00;
    uint8_t actor_state_01;
    uint32_t updated_global_380;
} BubsyActor347ECModeZeroAlternatePlan;

typedef struct BubsyActor347ECModeZeroPlan {
    uint8_t actions;
    uint8_t actor_state_00;
    uint16_t selection_a1;
    uint16_t selection_a2;
    uint16_t selection_a3;
    int32_t selection_stack_10;
    int32_t selection_stack_14;
    int32_t selection_stack_18;
    int32_t move_a0;
    int32_t move_a1;
    int32_t property_a0;
    int32_t property_a1;
    int32_t property_a2;
    int32_t property_a3;
} BubsyActor347ECModeZeroPlan;

typedef struct BubsyActor347ECFollowupPlan {
    uint16_t actions;
    int16_t dispatch_mode;
    int32_t linked_value_4C;
} BubsyActor347ECFollowupPlan;

typedef enum BubsyActor347ECStateBit4ClearActions {
    BUBSY_ACTOR_347EC_STATE4_CLEAR_SELECT_2B0 = 1,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_ACTOR_00 = 2,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_SET_ACTOR_FLAG_04 = 4,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_CALL_1C6BC = 8,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_PROPERTIES_5_8_A = 16,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_RESET_GLOBALS = 32,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_PROPERTY_C = 64,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_GLOBAL_6452 = 128,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_CALL_2E7FC = 256,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_ASSERT_BOUND = 1024,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_UPDATE_COUNTER = 2048,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_SET_ACTOR_STATE_01 = 4096,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_MOVE_05 = 8192,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_SET_GLOBAL_37C = 16384,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_ACTOR_FLAGS_03 = 32768,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_PROPERTIES_F_10 = 65536,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_RESET_6464_64B8 = 131072,
    BUBSY_ACTOR_347EC_STATE4_CLEAR_UPDATE_ACTOR_MODE_1 = 262144,
} BubsyActor347ECStateBit4ClearActions;

typedef struct BubsyActor347ECStateBit4ClearPlan {
    uint32_t actions;
    uint8_t actor_state_00;
    uint8_t actor_state_01;
    uint8_t actor_flags_04;
    uint32_t global_380;
} BubsyActor347ECStateBit4ClearPlan;

BubsyActorUpdateEntryResult bubsy_actor_update_entry_gate(
    BubsyActorUpdateActorView *actor,
    uint8_t update_mode,
    BubsyActorUpdateEnvironment *environment,
    uint8_t *local_state_24
);
BubsyActorDefaultStateRoute bubsy_actor_state_route_default(
    const BubsyActorUpdateActorView *actor,
    uint16_t handler_state_id,
    int32_t handler_state_aux,
    uint8_t runtime_state_6461,
    uint8_t update_lock_6479,
    int16_t update_gate_6454,
    uint8_t runtime_mode
);
BubsyActorSharedUpdateEntryAction bubsy_actor_shared_update_entry_action(
    uint32_t actor_flags_04,
    uint8_t actor_update_lock_6479
);
BubsyActor347ECModeEntryAction bubsy_actor_347ec_select_mode_entry(
    uint8_t mode,
    uint32_t actor_state_10,
    uint8_t *actor_state_00
);
void bubsy_actor_347ec_build_nonzero_mode_plan(
    uint32_t actor_state_10,
    uint8_t global_37C_nonzero,
    uint8_t actor_state_00,
    BubsyActor347ECArgumentPlan *plan
);
uint8_t bubsy_actor_347ec_select_mode_zero_actions(
    uint32_t actor_state_10,
    uint8_t *actor_state_00
);
BubsyActor347ECModeZeroPlan bubsy_actor_347ec_build_mode_zero_plan(
    uint32_t actor_state_10
);
BubsyActor347ECModeZeroBit40Plan bubsy_actor_347ec_build_mode_zero_bit40_plan(
    uint32_t actor_state_10,
    uint32_t actor_flags_04,
    uint8_t actor_state_01,
    uint32_t global_380,
    uint32_t object_limit
);
BubsyActor347ECModeZeroAlternatePlan bubsy_actor_347ec_plan_mode_zero_alternate(
    int32_t global_64BC_nonzero,
    int32_t global_6490,
    uint8_t selection_result_low,
    uint8_t actor_state_00,
    int32_t global_6464_nonzero,
    uint32_t actor_flags_04,
    uint32_t global_380,
    uint32_t object_limit
);
void bubsy_actor_347ec_build_followup_plan(
    uint32_t actor_state_10,
    int32_t selection_result,
    int16_t global_state_6450,
    uint8_t global_37D,
    int16_t global_counter_64BA,
    BubsyActor347ECFollowupPlan *plan
);
void bubsy_actor_347ec_build_state_bit4_clear_plan(
    uint32_t actor_state_10,
    uint8_t actor_state_00,
    uint8_t actor_state_01,
    uint8_t actor_flags_04,
    uint32_t global_380,
    uint32_t object_limit,
    BubsyActor347ECStateBit4ClearPlan *plan
);
uint8_t bubsy_grounded_case38_update_descriptor(uint8_t *descriptor_state_11);
uint32_t bubsy_grounded_handler_case_target(uint16_t state_id);
int32_t bubsy_grounded_handler_should_dispatch_switch(
    uint16_t state_id,
    int32_t descriptor_lookup_word_04,
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    int16_t *descriptor_state_02
);
BubsyGroundedCase37Action bubsy_grounded_case37_select_action(
    uint32_t actor_flags_04,
    uint8_t *descriptor_state_11
);
int32_t bubsy_grounded_cases29_30_update_descriptor(
    uint16_t state_id,
    uint8_t *descriptor_state_10
);
int32_t bubsy_grounded_cases13_34_36_update_state(
    uint16_t state_id,
    uint8_t *actor_state_01,
    uint16_t *descriptor_state_00
);
BubsyGroundedCase21Action bubsy_grounded_case21_select_action(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    int32_t companion_state_28,
    int32_t normalized_dot,
    int16_t *descriptor_state_02
);
uint8_t bubsy_grounded_case12_update_state(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t *actor_state_01,
    int16_t *descriptor_state_02
);
uint8_t bubsy_grounded_case17_update_state(
    uint32_t actor_state_10,
    uint8_t *actor_state_00,
    uint8_t *actor_state_01,
    uint8_t *descriptor_state_14
);
int32_t bubsy_grounded_case16_should_dispatch(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t actor_state_01,
    uint8_t *descriptor_state_14
);
BubsyGroundedCase14Actions bubsy_grounded_case14_prepare(
    uint32_t actor_flags_04,
    uint32_t *actor_state_10,
    uint8_t descriptor_state_11,
    uint32_t *linked_state_30
);
uint8_t bubsy_grounded_case15_select_actions(
    int32_t current_sequence_key,
    uint8_t sequence_boundary_reached
);
BubsyGroundedCase6Action bubsy_grounded_case6_select_action(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t *descriptor_state_4C
);
uint8_t bubsy_grounded_case20_select_actions(
    uint8_t descriptor_state_0D,
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    int32_t current_sequence_key
);
BubsyGroundedCase31Path bubsy_grounded_case31_advance_counter(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t descriptor_state_4C,
    uint8_t *descriptor_counter_4D
);
BubsyGroundedCase31Actions bubsy_grounded_case31_select_alternate_action(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    int16_t *descriptor_state_02
);
uint8_t bubsy_grounded_case31_select_sequence_actions(
    int32_t current_sequence_key,
    int32_t *linked_counter_30
);
BubsyGroundedCases8_10Action bubsy_grounded_cases8_10_select_action(
    uint16_t state_id,
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t actor_state_01,
    int16_t *descriptor_state_02
);
BubsyGroundedCases8_10CounterActions bubsy_grounded_cases8_10_update_counter_path(
    uint16_t state_id,
    uint32_t actor_state_10,
    uint8_t descriptor_state_11,
    int32_t *global_gate_540,
    uint32_t *global_state_544,
    int16_t *global_counter_53C
);
uint8_t bubsy_grounded_case32_update_state(
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t *actor_state_01,
    uint8_t *descriptor_counter_4D,
    uint8_t *descriptor_state_4C,
    uint8_t *descriptor_state_68,
    int16_t *descriptor_state_02,
    int32_t *linked_counter_30
);
BubsyGroundedCase7Actions bubsy_grounded_case7_update_state(
    uint32_t actor_state_10,
    uint8_t *actor_state_01,
    uint8_t *descriptor_state_4C,
    uint8_t *descriptor_state_68,
    int16_t *descriptor_state_02
);
uint8_t bubsy_grounded_cases9_11_update_state(
    uint16_t state_id,
    int32_t current_sequence_key,
    uint32_t *actor_state_10,
    uint32_t *actor_state_14,
    uint8_t *actor_state_01,
    int16_t *descriptor_state_02,
    int32_t global_540_nonzero,
    int32_t global_53C_nonzero
);
BubsyGroundedCases33_35Actions bubsy_grounded_cases33_35_prepare(
    uint16_t state_id,
    uint32_t actor_flags_04,
    uint32_t actor_state_10,
    uint8_t actor_state_01,
    uint16_t *descriptor_state_00,
    int32_t *linked_counter_30
);
int32_t bubsy_actor_update_should_select_309(
    const BubsyActorUpdateActorView *actor,
    int32_t current_sequence_key,
    uint8_t runtime_mode
);
uint8_t bubsy_actor_update_local_state_for_sequence(
    const BubsyActorUpdateActorView *actor,
    int32_t current_sequence_key,
    uint8_t runtime_mode,
    uint8_t local_state_24
);
int32_t bubsy_actor_update_should_check_mode1_sequence_boundary(
    int32_t current_sequence_key,
    uint8_t runtime_mode
);
int32_t bubsy_actor_update_should_reset_sequence_counter(
    const BubsyActorUpdateActorView *actor,
    int32_t sequence_counter_after_increment,
    uint8_t runtime_mode,
    int16_t level_id
);
int32_t bubsy_actor_update_should_enter_level14_counter_path(
    const BubsyActorUpdateActorView *actor,
    int32_t sequence_counter,
    uint8_t runtime_mode,
    int16_t level_id,
    int16_t update_gate
);
int32_t bubsy_actor_update_sequence_index_for_local_state(
    const BubsyActorUpdateActorView *actor,
    uint8_t local_state_24,
    uint8_t runtime_mode
);
int32_t bubsy_actor_update_get_sequence_mode_override(
    uint8_t local_state_24,
    uint8_t runtime_mode,
    int32_t *override_out
);

#endif
