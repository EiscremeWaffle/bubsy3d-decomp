#include "texture_font_slots.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

uint8_t g_texture_font_loaded;
TextureFontSlot g_texture_font_slots[TEXTURE_FONT_SLOT_COUNT];
const char g_texture_font_error_condition[] = "operation";
const char g_texture_font_state_condition[] = "loaded";
const char g_texture_font_source_path[] = "source";

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(!"unexpected assertion");
}

static void test_real_slot_integration(void) {
    int index;
    memset(g_texture_font_slots, 0xA5, sizeof(g_texture_font_slots));
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; index++) {
        g_texture_font_slots[index].tag = -1;
    }
    g_texture_font_loaded = 1;
    g_texture_font_slots[0].tag = 3;
    g_texture_font_slots[49].tag = 3;
    g_texture_font_slots[0].data.payload[0x13] = 0x22;
    g_texture_font_slots[0].data.payload[0x14] = 0x33;
    g_texture_font_slots[0].data.payload[0x15] = 0x44;
    assert(func_800482D0(3, &g_texture_font_slots[0].data.payload[0x13]) == 1);
    for (index = 0; index < 3; index++) {
        assert(g_texture_font_slots[0].data.payload[0x14 + index] == 0x22);
        assert(g_texture_font_slots[49].data.payload[0x14 + index] == 0x22);
        assert(g_texture_font_slots[1].data.payload[0x14 + index] == 0xA5);
    }
}

int main(void) {
    uint8_t bytes[40], expected[40];
    unsigned offset, i;

    assert(func_80055EA8(NULL, NULL) == 0x16);
    for (offset = 0; offset + 3 <= sizeof(bytes); offset++) {
        for (i = 0; i < sizeof(bytes); i++) {
            bytes[i] = (uint8_t)(i * 37U + offset);
        }
        memcpy(expected, bytes, sizeof(bytes));
        expected[0x14] = expected[offset];
        expected[0x15] = expected[offset + 1];
        expected[0x16] = expected[offset + 2];
        assert(func_80055EA8(bytes, bytes + offset) == 0);
        assert(memcmp(bytes, expected, sizeof(bytes)) == 0);
    }
    test_real_slot_integration();
    puts("texture color payload tests passed");
    return 0;
}
