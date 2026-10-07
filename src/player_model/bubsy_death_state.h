#ifndef BUBSY3D_DEATH_STATE_H
#define BUBSY3D_DEATH_STATE_H

#include <stdint.h>

typedef void (*BubsyDeathStateAssertFailure)(
    int32_t failed,
    const char *condition,
    const char *source_path,
    uint32_t source_line
);

typedef struct BubsyDeathStateRuntimeView {
    uint32_t actor_flags_04;
    uint32_t actor_state_10;
    uint8_t actor_event_guard_6460;
    uint8_t state_6461;
    uint8_t state_6463;
    uint8_t state_6479;
    uint8_t state_64B8;
    uint8_t state_649D;
    uint16_t state_6452;
    uint32_t state_64C0;
} BubsyDeathStateRuntimeView;

int32_t bubsy_advance_death_state(int32_t death_state, int16_t level_id, int32_t counter_value);
int32_t bubsy_death_state_normalize_initial(
    int32_t death_state,
    BubsyDeathStateAssertFailure assert_failure
);
int32_t bubsy_death_state_should_dispatch_entry_event(
    int32_t global_gate_3A4,
    uint8_t helper_result_low_byte
);
int32_t bubsy_death_state_should_run_level13_cleanup(int16_t level_id);
int32_t bubsy_death_state_should_run_main_loop(uint8_t state_6462);
int32_t bubsy_death_state_uses_counter_entry_list(int32_t global_counter_3A4);
int32_t bubsy_death_state_next_global_counter(
    int32_t global_counter_3A4,
    uint8_t helper_result_low_byte
);
void bubsy_death_state_reset_runtime(
    BubsyDeathStateRuntimeView *runtime,
    uint8_t actor_mode_0C
);
int32_t bubsy_death_state_should_dispatch_counter_entry_event(
    int32_t global_counter_3A4,
    uint8_t state_6462,
    int32_t entry_count,
    uint8_t entry_flag_20
);

#endif