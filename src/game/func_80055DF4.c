#include "texture_font_slots.h"

int32_t func_80055DF4(TextureFontPositionPayload *payload, int16_t position) {
    if (payload == 0) return 22;
    payload->x = position;
    return 0;
}
