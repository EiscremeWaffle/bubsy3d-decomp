#include "input_events.h"
#include "player_model.h"
#include "sequence_playback.h"

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

enum { FRAME_BYTES = 64, OUTPUT_WORDS = 8 };

typedef struct ProbeInput {
    uint32_t format, key, enabled, matching_slot;
    uint8_t first_frames[FRAME_BYTES], second_frames[FRAME_BYTES];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, flags, slot, node, initialized_key, count;
    uint32_t active[2], value[2];
    uint32_t first_output[OUTPUT_WORDS], second_output[OUTPUT_WORDS];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 16 + 2 * FRAME_BYTES, "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 26 * sizeof(uint32_t), "probe output layout");

SequencePlaybackNode *g_sequence_playback_active;
SequencePlaybackNode *g_sequence_playback_free;
SequencePlaybackNode g_sequence_playback_zero_address;
uint16_t g_sequence_playback_count;
uint8_t g_sequence_stream_context;
uint32_t g_input_sample_counter;
const char g_sequence_playback_free_condition[] = "free node";
const char g_sequence_playback_slot_condition[] = "free slot";
const char g_sequence_playback_source_path[] = "sequence source";

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(0);
}

void func_80067DEC(PlayerActorSequenceView *component, int32_t key, uint32_t context) {
    (void)component;
    (void)key;
    (void)context;
    assert(0);
}

static ProbeOutput run_probe(const ProbeInput *input) {
    PlayerActorSequenceView actor = {0};
    PlayerActorSequenceData sequence = {0};
    PlayerSequenceChannels channels = {0};
    PlayerSequenceResource resource = {0};
    SequencePlaybackSlot slots[2] = {{0}};
    SequencePlaybackNode nodes[3] = {{0}};
    uint32_t first_frames[FRAME_BYTES / sizeof(uint32_t)];
    uint32_t second_frames[FRAME_BYTES / sizeof(uint32_t)];
    uint32_t first_output[OUTPUT_WORDS];
    uint32_t second_output[OUTPUT_WORDS];
    int32_t entry = (int32_t)input->key;
    ProbeOutput output = {0};
    uint32_t index;

    assert((input->format & ~0x60000000u) == 0 && input->key < 3);
    memcpy(first_frames, input->first_frames, FRAME_BYTES);
    memcpy(second_frames, input->second_frames, FRAME_BYTES);
    memset(first_output, 0xA5, sizeof(first_output));
    memset(second_output, 0x5A, sizeof(second_output));
    actor.sequence_18 = &sequence;
    actor.flags_04 = 0xED;
    actor.playback_slot_06 = -1;
    sequence.entries = &entry;
    sequence.flags_2C = input->format | (2u << 3);
    sequence.playback_slots = slots;
    sequence.enabled_2B = (uint8_t)input->enabled;
    sequence.initialized_key_28 = 0x55AA;
    sequence.first_frames_10 = (const uint8_t *)first_frames;
    sequence.second_frames_14 = (const uint8_t *)second_frames;
    sequence.resource_20 = &resource;
    resource.channels_08 = &channels;
    channels.first_output = (uint8_t *)first_output;
    channels.first_count = 2;
    channels.second_output = (uint8_t *)second_output;
    channels.second_count = 2;
    slots[0].timestamp_0C = 7;
    slots[1].timestamp_0C = -10;
    slots[0].key_10 = input->matching_slot ? (uint8_t)input->key : 0xC0;
    slots[1].key_10 = 0xC1;
    slots[0].value_14 = slots[1].value_14 = 0xDEADBEEFu;
    nodes[0].next_08 = &nodes[1];
    nodes[1].previous_04 = &nodes[0];
    g_sequence_playback_free = &nodes[0];
    g_sequence_playback_active = &nodes[2];
    g_sequence_playback_count = 1;
    memset(&g_sequence_playback_zero_address, 0, sizeof(g_sequence_playback_zero_address));

    output.result = (uint32_t)func_80052BDC(&actor, 0);
    output.flags = actor.flags_04;
    output.slot = (uint8_t)actor.playback_slot_06;
    output.node = actor.playback_node_00 == &nodes[0];
    output.initialized_key = (uint16_t)sequence.initialized_key_28;
    output.count = g_sequence_playback_count;
    for (index = 0; index < 2; ++index) {
        output.active[index] = slots[index].active_08;
        output.value[index] = slots[index].value_14;
    }
    memcpy(output.first_output, first_output, sizeof(first_output));
    memcpy(output.second_output, second_output, sizeof(second_output));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;
    uint32_t index;

    for (index = 0; index < FRAME_BYTES; ++index) {
        input.first_frames[index] = (uint8_t)(index * 7);
        input.second_frames[index] = (uint8_t)(255 - index * 3);
    }
    input.key = 1;
    input.enabled = 1;
    output = run_probe(&input);
    assert(output.result == 0 && output.flags == 0xEC && output.slot == 1 && output.node == 1);
    assert(output.count == 2 && output.active[1] == 1 && output.value[1] == 0);
    assert(output.initialized_key == 1);
    assert(memcmp(output.first_output, input.first_frames + 16, 16) == 0);
    assert(memcmp(output.second_output, input.second_frames + 16, 16) == 0);
    input.matching_slot = 1;
    output = run_probe(&input);
    assert(output.slot == 0 && output.active[0] == 1 && output.value[0] == 0xDEADBEEFu);
    assert(output.initialized_key == 0x55AA && output.first_output[0] == 0xA5A5A5A5u);
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("real sequence-channel path tests passed");
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
            fputs("incomplete real-channel probe input\n", stderr);
            return 2;
        }
        output = run_probe(&input);
        if (fwrite(&output, 1, sizeof(output), stdout) != sizeof(output)) {
            perror("write real-channel probe output");
            return 2;
        }
    }
    if (ferror(stdin)) {
        perror("read real-channel probe input");
        return 2;
    }
    return 0;
}
