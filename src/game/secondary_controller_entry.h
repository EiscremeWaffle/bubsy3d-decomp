#ifndef BUBSY3D_SECONDARY_CONTROLLER_ENTRY_H
#define BUBSY3D_SECONDARY_CONTROLLER_ENTRY_H

#include "controller_bitmap_gate.h"
#include <stdint.h>

typedef struct SecondaryControllerOwner {
    uint8_t unknown_00[0x20];
    uint32_t status_20;
} SecondaryControllerOwner;

typedef struct SecondaryControllerActor {
    uint8_t unknown_00[0x1c];
    SecondaryControllerOwner *owner_1c;
} SecondaryControllerActor;

typedef enum SecondaryControllerEntryResult {
    SECONDARY_CONTROLLER_ENTRY_EXIT,
    SECONDARY_CONTROLLER_ENTRY_CONTINUE
} SecondaryControllerEntryResult;

extern uint8_t g_secondary_controller_initialized;
extern int16_t g_secondary_controller_timer;
extern uint8_t g_secondary_controller_flag_506;
extern void *g_extra_model_handle;
extern const char g_secondary_controller_timeout_text[];

/* Bounded model through 0x800495F4, not a byte match; context is separate output storage. */
SecondaryControllerEntryResult secondary_controller_entry(
    uint32_t buttons, void **context
);

#endif
