#include "game_transition.h"

void func_80023A0C(
    GameTransitionEventRecord *record,
    void *buttons,
    void *digital_pad_1,
    void *digital_pad_2,
    void *unused
) {
    (void)unused;
    record->payload.input.buttons = *(const uint32_t *)buttons;
    record->payload.input.digital_pad_1 = *(const uint8_t *)digital_pad_1;
    record->payload.input.digital_pad_2 = *(const uint8_t *)digital_pad_2;
}
