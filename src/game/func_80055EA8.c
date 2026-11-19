#include "texture_font_slots.h"

int32_t func_80055EA8(uint8_t *payload, const uint8_t color[3]) {
    if (payload == 0) {
        return 0x16;
    }
    payload[0x14] = color[0];
    payload[0x15] = color[1];
    payload[0x16] = color[2];
    return 0;
}
