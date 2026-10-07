#include "bubsy_death_state.h"

int32_t bubsy_advance_death_state(int32_t death_state, int16_t level_id, int32_t counter_value) {
    if (death_state < 0) {
        death_state = 1;
    }

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