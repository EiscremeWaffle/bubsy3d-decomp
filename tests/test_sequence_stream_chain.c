#include "input_events.h"
#include "sequence_stream.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

enum { FRAME_BYTES = 1536, HEIGHT_BYTES = 2048, LOOKUP_BYTES = 512 };
typedef struct ProbeInput {
    uint32_t key, width, slot, count, records[5];
    uint8_t frames[FRAME_BYTES], heights[HEIGHT_BYTES], lookup[LOOKUP_BYTES];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, flags, slot, node, initialized_key, count;
    uint32_t active[3], values[3];
    uint8_t grid[1024], expanded[1024], metadata[2048];
    uint32_t keys[3];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 36 + FRAME_BYTES + HEIGHT_BYTES + LOOKUP_BYTES, "input layout");
_Static_assert(sizeof(ProbeOutput) == 48 + 4096 + 12, "output layout");

const char g_sequence_stream_width_condition[] = "terrainWidth";
const char g_sequence_stream_source_path[] = "../f/egsfast.c";
const char g_sequence_playback_free_condition[] = "free node";
const char g_sequence_playback_slot_condition[] = "free slot";
const char g_sequence_playback_source_path[] = "sequence source";
SequencePlaybackNode *g_sequence_playback_active;
SequencePlaybackNode *g_sequence_playback_free;
SequencePlaybackNode g_sequence_playback_zero_address;
uint16_t g_sequence_playback_count;
uint8_t g_sequence_stream_context;
uint32_t g_input_sample_counter;
int16_t g_sequence_stream_lookup_count;
SequenceStreamLookupRecord g_sequence_stream_lookup_records[5];

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(0);
}

static ProbeOutput run_probe(const ProbeInput *input) {
    PlayerActorSequenceView actor = {0};
    PlayerActorSequenceData sequence = {0};
    SequencePlaybackSlot slots[3] = {{0}};
    SequencePlaybackNode nodes[3] = {{0}};
    SequenceStreamOwner owners[3] = {{0}};
    SequenceStreamDescriptor descriptor = {0};
    uint32_t frames[FRAME_BYTES / 4], lookup[LOOKUP_BYTES / 4];
    uint8_t heights[HEIGHT_BYTES];
    uint32_t grid[256], expanded[256], metadata[512];
    ProbeOutput output;
    unsigned index;
    assert(input->key < 3 && input->width > 0 && input->width <= 4);
    assert(input->slot < 3 && input->count >= 1 && input->count <= 5);
    memcpy(frames, input->frames, sizeof(frames));
    memcpy(heights, input->heights, sizeof(heights));
    memcpy(lookup, input->lookup, sizeof(lookup));
    memset(grid, 0xA5, sizeof(grid));
    memset(expanded, 0xC3, sizeof(expanded));
    memset(metadata, 0x5A, sizeof(metadata));
    for (index = 0; index < 5; ++index) {
        g_sequence_stream_lookup_records[index].value_10 = input->records[index];
    }
    g_sequence_stream_lookup_count = (int16_t)input->count;
    actor.sequence_18 = &sequence;
    actor.flags_04 = 0xED;
    actor.playback_slot_06 = -1;
    actor.cursor_08 = (int16_t)input->key;
    sequence.first_frames_10 = (const uint8_t *)frames;
    sequence.second_frames_14 = (const uint8_t *)lookup;
    sequence.stream_table_18 = heights;
    sequence.playback_slots = slots;
    sequence.initialized_key_28 = 0x55AA;
    sequence.enabled_2B = 1;
    sequence.flags_2C = 4 | (3u << 3);
    descriptor.surface_00 = grid;
    descriptor.output_08 = (uint8_t *)expanded;
    descriptor.metadata_10 = metadata;
    for (index = 0; index < 3; ++index) {
        slots[index].stream_owner_04 = &owners[index];
        slots[index].key_10 = (uint8_t)(0x40 + index);
        slots[index].timestamp_0C = index == input->slot ? -100 : (int32_t)index + 10;
        slots[index].value_14 = 0xDEADBEEFu;
        owners[index].descriptor_08 = &descriptor;
    }
    nodes[0].next_08 = &nodes[1];
    nodes[1].previous_04 = &nodes[0];
    g_sequence_playback_free = &nodes[0];
    g_sequence_playback_active = &nodes[2];
    g_sequence_playback_count = 1;
    g_sequence_stream_context = (uint8_t)input->width;
    memset(&g_sequence_playback_zero_address, 0, sizeof(g_sequence_playback_zero_address));
    output.result = (uint32_t)func_80052BDC(&actor, 0);
    output.flags = actor.flags_04;
    output.slot = (uint8_t)actor.playback_slot_06;
    output.node = actor.playback_node_00 == &nodes[0];
    output.initialized_key = (uint16_t)sequence.initialized_key_28;
    output.count = g_sequence_playback_count;
    memcpy(output.grid, grid, sizeof(grid));
    memcpy(output.expanded, expanded, sizeof(expanded));
    memcpy(output.metadata, metadata, sizeof(metadata));
    for (index = 0; index < 3; ++index) {
        output.active[index] = slots[index].active_08;
        output.values[index] = slots[index].value_14;
        output.keys[index] = slots[index].key_10;
    }
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;
    unsigned index;
    input.key = 2;
    input.width = 3;
    input.slot = 1;
    input.count = 1;
    for (index = 0; index < FRAME_BYTES; ++index) {
        input.frames[index] = (uint8_t)index;
    }
    output = run_probe(&input);
    assert(output.result == 0 && output.flags == 0xEC && output.slot == 1 && output.node == 1);
    assert(output.initialized_key == 0x55AA && output.count == 2);
    assert(output.active[1] == 1 && output.values[1] == 0);
    assert(output.keys[0] == 0x40 && output.keys[1] == 2 && output.keys[2] == 0x42);
    assert(output.grid[0] == 0 && output.grid[1] == 0x10);
    assert(output.expanded[0] == 0 && output.expanded[1] == 0);
    assert(output.expanded[2] == 0x40 && output.expanded[3] == 0);
    assert(output.expanded[4] == 0x20 && output.expanded[5] == 0);
    assert(output.metadata[0] == 0x5A && output.metadata[4] == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;
    if (argc == 1) {
        self_test();
        puts("complete streaming-chain probe passed");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) {
        fputs("expected --binary or no arguments\n", stderr);
        return 2;
    }
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
        perror("set binary mode");
        return 2;
    }
#endif
    while ((bytes_read = fread(&input, 1, sizeof(input), stdin)) != 0) {
        ProbeOutput output;
        if (bytes_read != sizeof(input)) {
            fputs("incomplete streaming-chain probe input\n", stderr);
            return 2;
        }
        output = run_probe(&input);
        if (fwrite(&output, 1, sizeof(output), stdout) != sizeof(output)) {
            perror("write streaming-chain probe output");
            return 2;
        }
    }
    if (ferror(stdin)) {
        perror("read streaming-chain probe input");
        return 2;
    }
    return 0;
}
