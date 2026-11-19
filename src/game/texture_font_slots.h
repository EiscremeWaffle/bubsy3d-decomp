#ifndef BUBSY3D_TEXTURE_FONT_SLOTS_H
#define BUBSY3D_TEXTURE_FONT_SLOTS_H

#include <stdint.h>

enum { TEXTURE_FONT_SLOT_COUNT = 50 };

typedef struct TextureFontSlot {
    int8_t tag;
    uint8_t reserved[3];
    uint8_t payload[40];
} TextureFontSlot;

extern uint8_t g_texture_font_loaded;
extern TextureFontSlot g_texture_font_slots[TEXTURE_FONT_SLOT_COUNT];
extern uint32_t g_texture_font_count;
extern const char g_texture_font_error_condition[];
extern const char g_texture_font_state_condition[];
extern const char g_texture_font_source_path[];

int32_t func_800482D0(int32_t tag, const uint8_t color[3]);
int32_t func_800483C0(int32_t tag);
void func_80048488(void);
int32_t func_80048504(int32_t tag);
int32_t func_80048C48(int32_t tag);
int32_t func_80055EA8(uint8_t *payload, const uint8_t color[3]);

#endif
