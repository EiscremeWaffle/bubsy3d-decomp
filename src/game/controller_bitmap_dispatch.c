/* Host model of the bitmap-dispatch portion of func_80023E94, not a byte match. */
#include "controller_bitmap_dispatch.h"

void controller_bitmap_dispatch(const ControllerBitmapDispatch *dispatch, uint32_t buttons) {
    uint32_t *bitmap = dispatch->bitmap;
    const volatile int16_t *selector = dispatch->selector;
    const int32_t *rows = dispatch->rows;
    const uint32_t (*pressed_events)[16] = dispatch->pressed_events;
    const uint32_t (*released_events)[16] = dispatch->released_events;
    ControllerBitmapEmit emit = dispatch->emit;
    void *context = dispatch->context;
    unsigned int bit;

    for (bit = 0; bit < 16; bit++) {
        uint32_t mask = 1u << bit;
        const uint32_t (*events)[16];
        uint32_t event;

        if (buttons & mask) {
            *bitmap |= mask;
            events = pressed_events;
        } else if (*bitmap & mask) {
            *bitmap &= ~mask;
            events = released_events;
        } else {
            continue;
        }
        event = events[rows[*selector]][bit];
        if (event != 0) {
            emit(context, (uint8_t)event, 4, 0xFFFE, 0, 0, 0, 0, 0);
        }
    }
}
