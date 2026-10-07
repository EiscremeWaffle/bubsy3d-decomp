#include "bubsy_death_state.h"

int32_t bubsy_death_state_normalize_initial(
    int32_t death_state,
    BubsyDeathStateAssertFailure assert_failure
) {
    if (death_state >= 0) {
        return death_state;
    }

    if (assert_failure != 0) {
        assert_failure(
            0,
            "gBubsyInfo.deathType >= 0",
            "../f/bubsy.c",
            0x912
        );
    }
    return 1;
}

int32_t bubsy_advance_death_state(int32_t death_state, int16_t level_id, int32_t counter_value) {
    death_state = bubsy_death_state_normalize_initial(death_state, 0);

    if (death_state == 1) {
        const int32_t remainder = counter_value % 3;
        if (remainder == 1) {
            death_state = 7;
        } else if (remainder == 2) {
            death_state = 9;
        }
    } else if (death_state == 2) {
        if (counter_value % 2 == 1) {
            death_state = 10;
        }
    } else if (death_state == 4) {
        if (level_id == 4 || level_id == 7 || level_id == 9 || level_id == 18) {
            death_state = 6;
        } else if (level_id == 6) {
            death_state = 8;
        } else if (level_id == 8) {
            death_state = 11;
        }
    } else if (level_id == 5 || level_id == 8) {
        death_state = 11;
    }

    if (death_state == 11 && (level_id == 4 || level_id == 6 || level_id == 8)) {
        death_state = counter_value % 2 == 0 ? 12 : 13;
    }

    return death_state;
}

int32_t bubsy_death_state_should_dispatch_entry_event(
    int32_t global_gate_3A4,
    uint8_t helper_result_low_byte
) {
    return global_gate_3A4 == -2 && helper_result_low_byte == 0;
}

int32_t bubsy_death_state_should_run_level13_cleanup(int16_t level_id) {
    return level_id == 0x13;
}

int32_t bubsy_death_state_should_run_main_loop(uint8_t state_6462) {
    return state_6462 == 0;
}

int32_t bubsy_death_state_uses_counter_entry_list(int32_t global_counter_3A4) {
    return global_counter_3A4 == -2 || global_counter_3A4 == -1;
}

int32_t bubsy_death_state_next_global_counter(
    int32_t global_counter_3A4,
    uint8_t helper_result_low_byte
) {
    if (global_counter_3A4 == -2) {
        return -1;
    }

    return helper_result_low_byte == 1 ? 0 : global_counter_3A4;
}

int32_t bubsy_death_state_should_dispatch_counter_entry_event(
    int32_t global_counter_3A4,
    uint8_t state_6462,
    int32_t entry_count,
    uint8_t entry_flag_20
) {
    return (global_counter_3A4 == -2 || global_counter_3A4 == -1) &&
        state_6462 == 0 &&
        entry_count > 0 &&
        entry_flag_20 == 0;
}