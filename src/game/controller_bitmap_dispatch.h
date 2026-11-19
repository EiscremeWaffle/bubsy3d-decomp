#ifndef BUBSY3D_CONTROLLER_BITMAP_DISPATCH_H
#define BUBSY3D_CONTROLLER_BITMAP_DISPATCH_H

#include <stdint.h>

typedef void (*ControllerBitmapEmit)(
    void *context, int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7);

typedef struct ControllerBitmapDispatch {
    uint32_t *bitmap;
    const volatile int16_t *selector;
    const int32_t *rows;
    const uint32_t (*pressed_events)[16];
    const uint32_t (*released_events)[16];
    ControllerBitmapEmit emit;
    void *context;
} ControllerBitmapDispatch;

/* The selector and row must identify valid entries, as in the original loop. */
void controller_bitmap_dispatch(const ControllerBitmapDispatch *dispatch, uint32_t buttons);

#endif
