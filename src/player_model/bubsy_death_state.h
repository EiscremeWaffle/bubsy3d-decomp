#ifndef BUBSY3D_DEATH_STATE_H
#define BUBSY3D_DEATH_STATE_H

#include <stdint.h>

int32_t bubsy_advance_death_state(int32_t death_state, int16_t level_id, int32_t counter_value);
int32_t bubsy_death_state_should_dispatch_entry_event(
    int32_t global_gate_3A4,
    uint8_t helper_result_low_byte
);
int32_t bubsy_death_state_should_run_level13_cleanup(int16_t level_id);

#endif