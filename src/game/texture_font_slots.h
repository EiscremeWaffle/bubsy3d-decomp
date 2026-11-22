#ifndef BUBSY3D_TEXTURE_FONT_SLOTS_H
#define BUBSY3D_TEXTURE_FONT_SLOTS_H

#include <stdint.h>

enum { TEXTURE_FONT_SLOT_COUNT = 50 };

typedef struct TextureFontPositionPayload {
    uint8_t before[28];
    int16_t x;
    int16_t y;
    uint8_t after[8];
} TextureFontPositionPayload;

typedef struct TextureFontSlot {
    int8_t tag;
    uint8_t reserved[3];
    union {
        uint8_t payload[40];
        TextureFontPositionPayload position_payload;
    } data;
} TextureFontSlot;

extern uint8_t g_texture_font_loaded;
extern TextureFontSlot g_texture_font_slots[TEXTURE_FONT_SLOT_COUNT];
extern uint32_t g_texture_font_count;
extern const char g_texture_font_error_condition[];
extern const char g_texture_font_state_condition[];
extern const char g_texture_font_source_path[];

int32_t func_800482D0(int32_t tag, const uint8_t color[3]);
int32_t func_800481D4(int32_t tag, int32_t first, int32_t second);
int32_t func_80055DF4(TextureFontPositionPayload *payload, int16_t position);
int32_t func_80055E38(TextureFontPositionPayload *payload, int16_t position);
int32_t func_800483C0(int32_t tag);
void func_80048488(void);
int32_t func_80048504(int32_t tag);
int32_t func_80048C48(int32_t tag);
int32_t func_80055EA8(uint8_t *payload, const uint8_t color[3]);

#endif
