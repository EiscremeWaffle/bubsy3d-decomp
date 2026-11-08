#include "texture_font_slots.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

TextureFontSlot g_texture_font_slots[TEXTURE_FONT_SLOT_COUNT];

typedef struct Call {
    int function;
    int32_t tag;
} Call;

static Call calls[2 * TEXTURE_FONT_SLOT_COUNT];
static size_t call_count;
static int mutate;

static void record(int function, int32_t tag) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count].function = function;
    calls[call_count].tag = tag;
    ++call_count;
}

int32_t func_80048504(int32_t tag) {
    record(1, tag);
    if (mutate) {
        mutate = 0;
        g_texture_font_slots[0].tag = 99;
        g_texture_font_slots[1].tag = -5;
        g_texture_font_slots[2].tag = -1;
        g_texture_font_slots[49].tag = 7;
    }
    return -1;
}

int32_t func_80048C48(int32_t tag) {
    record(2, tag);
    return 0;
}

static void clear_slots(void) {
    int32_t index;
    memset(g_texture_font_slots, 0xA5, sizeof(g_texture_font_slots));
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
        g_texture_font_slots[index].tag = -1;
    }
    call_count = 0;
    mutate = 0;
}

int main(void) {
    unsigned int value;
    for (value = 0; value < 256; ++value) {
        int32_t tag = value < 128 ? (int32_t)value : (int32_t)value - 256;
        clear_slots();
        g_texture_font_slots[0].tag = (int8_t)tag;
        func_80048488();
        if (tag == -1) {
            assert(call_count == 0);
        } else {
            assert(call_count == 2);
            assert(calls[0].function == 1 && calls[0].tag == tag);
            assert(calls[1].function == 2 && calls[1].tag == tag);
        }
    }

    clear_slots();
    g_texture_font_slots[0].tag = 5;
    g_texture_font_slots[2].tag = 9;
    mutate = 1;
    func_80048488();
    assert(call_count == 6);
    assert(calls[0].function == 1 && calls[0].tag == 5);
    assert(calls[1].function == 2 && calls[1].tag == 5);
    assert(calls[2].function == 1 && calls[2].tag == -5);
    assert(calls[3].function == 2 && calls[3].tag == -5);
    assert(calls[4].function == 1 && calls[4].tag == 7);
    assert(calls[5].function == 2 && calls[5].tag == 7);
    puts("tagged-slot cleanup sweep tests passed");
    return 0;
}
