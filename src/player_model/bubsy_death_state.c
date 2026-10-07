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

void bubsy_death_state_reset_runtime(
    BubsyDeathStateRuntimeView *runtime,
    uint8_t actor_mode_0C
) {
    runtime->actor_flags_04 = 0;
    runtime->actor_state_10 = 0;
    runtime->actor_event_guard_6460 = 0;
    runtime->state_6461 = 0;
    runtime->state_6463 = actor_mode_0C != 0;
    runtime->state_6479 = 0;
    runtime->state_64B8 = 0;
    runtime->state_649D = 0;
    runtime->state_6452 = 0;
    runtime->state_64C0 = 0;
}

void bubsy_death_state_finalize_runtime(BubsyDeathStateFinalizationView *runtime) {
    unsigned int index;

    *runtime->actor_state_2C = 0;
    *runtime->actor_component_state_20 = 0;
    for (index = 0; index < 11; index++) {
        *runtime->nested_state_words[index] = 0;
    }
    *runtime->global_counter_3A4 = -2;
    *runtime->state_6462 = 1;
    *runtime->state_6464 = 0;
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

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
extern void *func_80026648(void *actor, void *context);
extern int32_t func_800222E0(void);
extern int32_t func_80082ECC(void);
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
extern void func_8004733C(int32_t value);
extern void func_8001A670(void);
extern void func_800556D0(
    int32_t failed,
    const char *condition,
    const char *source_path,
    uint32_t source_line
);

void bubsy_handle_death_state(void *actor, void *context) {
    volatile int32_t *death_type = (volatile int32_t *)0x80186458u;
    volatile const int16_t *level_id = (volatile const int16_t *)0x801D36F0u;
    volatile int32_t *global_counter = (volatile int32_t *)0x800BE100u;
    volatile uint8_t *state_6462 = (volatile uint8_t *)0x80186462u;
    volatile uint8_t *actor_bytes = (volatile uint8_t *)actor;
    volatile uint8_t *nested_state = *(volatile uint8_t **)(actor_bytes + 0x28);
    void *death_info = func_80026648(actor, context);
    BubsyDeathStateRuntimeView runtime;
    volatile uint8_t *actor_component;
    int32_t actor_value;

    if (bubsy_death_state_should_dispatch_entry_event(
            *global_counter,
            (uint8_t)func_800222E0()
        )) {
        actor_value = *(volatile int32_t *)(actor_bytes + 0x0C);
        func_8001B558(0x1B, 4, actor_value, actor_value, 0, 0, 0, 0);
        return;
    }
    if (bubsy_death_state_should_run_level13_cleanup(*level_id)) {
        func_8004733C(0);
        return;
    }
    if (!bubsy_death_state_should_run_main_loop(*state_6462)) {
        return;
    }
    if (bubsy_death_state_uses_counter_entry_list(*global_counter)) {
        volatile int32_t *entry_count = (volatile int32_t *)0x8018645Cu;
        int32_t count = *entry_count;
        uint8_t entry_flag = 1;

        if (count > 0) {
            volatile const uint8_t *entry_list =
                *(volatile const uint8_t * volatile *)0x801D89A4u;
            const uint32_t entry_offset = (uint32_t)count * 36u + 0x20u;
            entry_flag = entry_list[entry_offset];
        }
        if (bubsy_death_state_should_dispatch_counter_entry_event(
                *global_counter,
                *state_6462,
                count,
                entry_flag
            )) {
            actor_value = *(volatile int32_t *)(actor_bytes + 0x0C);
            func_8001B558(0x1B, 5, actor_value, count, 0, 0, 0, 0);
        }
        *entry_count = -1;
        if (*global_counter == -2) {
            *global_counter = -1;
        } else if ((uint8_t)func_800222E0() == 1) {
            *global_counter = 0;
        }
        actor_value = *(volatile int32_t *)(actor_bytes + 0x0C);
        func_8001B558(0x1B, 0, actor_value, actor_value, 0, 0, 0, 0);
        return;
    }
    if (*death_type < 0) {
        func_800556D0(
            0,
            (const char *)0x80012A20u,
            (const char *)0x8001298Cu,
            0x912
        );
    }
    *death_type = bubsy_advance_death_state(*death_type, *level_id, func_80082ECC());

    runtime.actor_flags_04 = *(volatile uint32_t *)(actor_bytes + 0x04);
    runtime.actor_state_10 = *(volatile uint32_t *)(actor_bytes + 0x10);
    runtime.actor_event_guard_6460 = *(volatile uint8_t *)0x80186460u;
    runtime.state_6461 = *(volatile uint8_t *)0x80186461u;
    runtime.state_6463 = *(volatile uint8_t *)0x80186463u;
    runtime.state_6479 = *(volatile uint8_t *)0x80186479u;
    runtime.state_64B8 = *(volatile uint8_t *)0x801864B8u;
    runtime.state_649D = *(volatile uint8_t *)0x8018649Du;
    runtime.state_6452 = *(volatile uint16_t *)0x80186452u;
    runtime.state_64C0 = *(volatile uint32_t *)0x801864C0u;
    bubsy_death_state_reset_runtime(
        &runtime,
        *((volatile const uint8_t *)death_info + 0x0C)
    );
    *(volatile uint32_t *)(actor_bytes + 0x04) = runtime.actor_flags_04;
    *(volatile uint32_t *)(actor_bytes + 0x10) = runtime.actor_state_10;
    *(volatile uint8_t *)0x80186460u = runtime.actor_event_guard_6460;
    *(volatile uint8_t *)0x80186461u = runtime.state_6461;
    *(volatile uint8_t *)0x80186463u = runtime.state_6463;
    *(volatile uint8_t *)0x80186479u = runtime.state_6479;
    *(volatile uint8_t *)0x801864B8u = runtime.state_64B8;
    *(volatile uint8_t *)0x8018649Du = runtime.state_649D;
    *(volatile uint16_t *)0x80186452u = runtime.state_6452;
    *(volatile uint32_t *)0x801864C0u = runtime.state_64C0;

    *state_6462 = 1;
    actor_component = *(volatile uint8_t **)(actor_bytes + 0x1C);
    actor_bytes[0x2C] = 0;
    *global_counter = -2;
    *(volatile uint32_t *)(actor_component + 0x20) = 0;
    *(volatile uint32_t *)(nested_state + 0x40) = 0;
    *(volatile uint32_t *)(nested_state + 0x3C) = 0;
    *(volatile uint32_t *)(nested_state + 0x38) = 0;
    *(volatile uint32_t *)(nested_state + 0x80) = 0;
    *(volatile uint32_t *)(nested_state + 0x7C) = 0;
    *(volatile uint32_t *)(nested_state + 0x78) = 0;
    *(volatile uint32_t *)(nested_state + 0xA0) = 0;
    *(volatile uint32_t *)(nested_state + 0x9C) = 0;
    *(volatile uint32_t *)(nested_state + 0x98) = 0;
    *(volatile uint32_t *)(nested_state + 0x30) = 0;
    *(volatile uint32_t *)(nested_state + 0x34) = 0;
    *(volatile uint8_t *)0x80186464u = 0;
    func_8001A670();
}
#endif
