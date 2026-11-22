#include "texture_font_slots.h"

int32_t func_80055E38(TextureFontPositionPayload *payload, int16_t position) {
    if (payload == 0) return 22;
    payload->y = position;
    return 0;
}
