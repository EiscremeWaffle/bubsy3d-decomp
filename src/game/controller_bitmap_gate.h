#ifndef BUBSY3D_CONTROLLER_BITMAP_GATE_H
#define BUBSY3D_CONTROLLER_BITMAP_GATE_H

#include "cyclic_entries.h"
#include <stddef.h>
#include <stdint.h>

typedef struct ControllerGameContext {
    uint8_t unknown_00[0x12];
    uint8_t blocked_12;
    uint8_t toggle_13;
    uint8_t unknown_14[0x15];
    uint8_t blocked_29;
    uint8_t unknown_2a;
    uint8_t blocked_2b;
    uint8_t unknown_2c[0x41];
    uint8_t mode_6d;
} ControllerGameContext;

typedef struct ControllerRuntimeFlags {
    uint8_t unknown_0000[0x52];
    uint8_t flash_byte_0052;
    uint8_t unknown_0053[0x2A2A - 0x53];
    uint8_t buttons_2a2a;
} ControllerRuntimeFlags;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(ControllerGameContext, toggle_13) == 0x13, "controller toggle offset");
_Static_assert(offsetof(ControllerGameContext, blocked_29) == 0x29, "controller gate offset");
_Static_assert(offsetof(ControllerGameContext, blocked_2b) == 0x2B, "controller gate offset");
_Static_assert(offsetof(ControllerGameContext, mode_6d) == 0x6D, "controller mode offset");
_Static_assert(offsetof(ControllerRuntimeFlags, flash_byte_0052) == 0x52, "controller flash offset");
_Static_assert(offsetof(ControllerRuntimeFlags, buttons_2a2a) == 0x2A2A, "controller flags offset");
#endif

extern ControllerRuntimeFlags *volatile g_controller_runtime_flags;
extern volatile uint8_t g_controller_button_hold;
extern volatile uint8_t g_controller_action_latched; /* Also the mode-one flash flag. */
extern volatile uint32_t g_controller_lifecycle_state;
extern volatile int32_t g_controller_cycle_cursor;
extern const char g_controller_cycle_condition[];
extern const char g_controller_cycle_source[];

/*
 * Returns nonzero only when the callback proceeds to the bitmap loop.
 * The caller supplies initialized, separate storage for the original stack entry.
 */
int32_t controller_gate_bitmap(
    ControllerGameContext *context, uint8_t mode, uint32_t buttons, GameCyclicEntry *cycle_entry
);

#endif
