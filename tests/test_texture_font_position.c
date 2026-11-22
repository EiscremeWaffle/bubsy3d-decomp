#include "texture_font_slots.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

typedef struct ProbeInput {
    uint32_t loaded, assertion_loaded, tag, x, y, matches, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, assertions, loaded, count, calls[100], tags[50], positions[100];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 7 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 254 * sizeof(uint32_t), "probe output layout");
_Static_assert(sizeof(TextureFontSlot) == 44, "slot size");
_Static_assert(offsetof(TextureFontSlot, data) == 4, "payload offset");
_Static_assert(offsetof(TextureFontPositionPayload, x) == 28, "x offset");
_Static_assert(offsetof(TextureFontPositionPayload, y) == 30, "y offset");

uint8_t g_texture_font_loaded;
TextureFontSlot g_texture_font_slots[TEXTURE_FONT_SLOT_COUNT];
const char g_texture_font_state_condition[] = "state";
const char g_texture_font_source_path[] = "source";

static ProbeInput current;
static ProbeOutput output;

#ifndef REAL_POSITION
static unsigned slot_index(TextureFontPositionPayload *payload) {
    unsigned index;
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; index++) {
        if (payload == &g_texture_font_slots[index].data.position_payload) return index;
    }
    assert(0);
    return 0;
}
#endif

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0 && condition == g_texture_font_state_condition);
    assert(source == g_texture_font_source_path && line == 0x17A);
    output.assertions++;
    g_texture_font_loaded = (uint8_t)current.assertion_loaded;
}

#ifndef REAL_POSITION
int32_t func_80055DF4(TextureFontPositionPayload *payload, int16_t position) {
    unsigned index = slot_index(payload);
    assert((uint16_t)position == (uint16_t)current.x);
    assert(output.count < 100);
    output.calls[output.count++] = index * 2;
    if (index == 0 && (current.mutations & 1)) g_texture_font_slots[1].tag = (int8_t)current.tag;
    if (index == 0 && (current.mutations & 2)) g_texture_font_loaded = 0;
    if (index == 0 && (current.mutations & 4)) g_texture_font_slots[0].tag = (int8_t)(current.tag ^ 0x80);
    return -1;
}
int32_t func_80055E38(TextureFontPositionPayload *payload, int16_t position) {
    unsigned index = slot_index(payload);
    assert((uint16_t)position == (uint16_t)current.y);
    assert(output.count < 100);
    output.calls[output.count++] = index * 2 + 1;
    if (index == 0 && (current.mutations & 8)) g_texture_font_slots[1].tag = (int8_t)(current.tag ^ 0x80);
    return -1;
}
#endif

static ProbeOutput run_probe(const ProbeInput *input) {
    static const unsigned selected[] = {0, 1, 25, 49};
    int32_t tag, x, y;
    unsigned index;
    current = *input;
    memset(&output, 0xFF, sizeof(output));
    output.assertions = output.count = 0;
    memset(g_texture_font_slots, 0, sizeof(g_texture_font_slots));
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; index++) {
        g_texture_font_slots[index].tag = (int8_t)(current.tag ^ 0x80);
    }
    for (index = 0; index < 4; index++) {
        if (current.matches & (1U << index)) g_texture_font_slots[selected[index]].tag = (int8_t)current.tag;
    }
    g_texture_font_loaded = (uint8_t)current.loaded;
    memcpy(&tag, &current.tag, sizeof(tag));
    memcpy(&x, &current.x, sizeof(x));
    memcpy(&y, &current.y, sizeof(y));
#ifdef REAL_POSITION
    assert(func_80055DF4(NULL, (int16_t)x) == 22);
    assert(func_80055E38(NULL, (int16_t)y) == 22);
#endif
    output.result = (uint32_t)func_800481D4(tag, x, y);
    output.loaded = g_texture_font_loaded;
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; index++) {
        output.tags[index] = (uint8_t)g_texture_font_slots[index].tag;
#ifdef REAL_POSITION
        output.positions[index * 2] = (uint16_t)g_texture_font_slots[index].data.position_payload.x;
        output.positions[index * 2 + 1] = (uint16_t)g_texture_font_slots[index].data.position_payload.y;
#endif
    }
    return output;
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t read_count;
    if (argc == 1) {
        memset(&input, 0, sizeof(input));
        input.loaded = 1;
        input.tag = 255;
        input.matches = 1;
        input.mutations = 1 | 2 | 4;
        result = run_probe(&input);
#ifdef REAL_POSITION
        assert(result.result == 1 && result.loaded == 1);
        assert(result.positions[0] == 0 && result.positions[1] == 0);
#else
        assert(result.result == 1 && result.loaded == 0 && result.count == 4);
        assert(result.calls[0] == 0 && result.calls[1] == 1 && result.calls[2] == 2 && result.calls[3] == 3);
#endif
        puts("texture font position tests passed");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) return 2;
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
        perror("set binary mode");
        return 2;
    }
#endif
    while ((read_count = fread(&input, 1, sizeof(input), stdin)) != 0) {
        if (read_count != sizeof(input)) {
            fputs("incomplete probe input\n", stderr);
            return 2;
        }
        result = run_probe(&input);
        if (fwrite(&result, sizeof(result), 1, stdout) != 1) {
            perror("write probe output");
            return 2;
        }
    }
    if (ferror(stdin)) {
        fputs("failed to read probe input\n", stderr);
        return 2;
    }
    return 0;
}
