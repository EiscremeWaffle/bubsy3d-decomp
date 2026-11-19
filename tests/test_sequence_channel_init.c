#include "packed_channel_copy.h"
#include "player_model.h"
#include "sequence_playback.h"
#include "word_copy.h"

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

typedef struct ProbeInput {
    uint32_t enabled, flags, context, key, first_count, second_count;
    uint32_t mutations, changed_flags, changed_count;
} ProbeInput;

typedef struct ProbeCall {
    uint32_t kind, output, input, count;
} ProbeCall;

typedef struct ProbeOutput {
    uint32_t result, key, actor_sequence, flags, resource, call_count;
    ProbeCall calls[2];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 9 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 14 * sizeof(uint32_t), "probe output layout");

uint8_t g_sequence_stream_context;

static uint32_t input_words[256];
static uint32_t output_words[64];
static PlayerActorSequenceView *probe_actor;
static PlayerActorSequenceData *probe_sequence;
static PlayerActorSequenceData *probe_alternate;
static PlayerSequenceResource *probe_resources;
static PlayerSequenceChannels *probe_channels;
static ProbeOutput *probe_output;
static ProbeInput probe_input;

static void mutate_first_call(void) {
    if (probe_input.mutations & 1) {
        probe_sequence->flags_2C = probe_input.changed_flags;
    }
    if (probe_input.mutations & 2) {
        probe_sequence->resource_20 = &probe_resources[1];
    }
    if (probe_input.mutations & 4) {
        probe_sequence->second_frames_14 = (const uint8_t *)input_words + 0x300;
    }
    if (probe_input.mutations & 8) {
        probe_channels[0].second_count = (int32_t)probe_input.changed_count;
    }
    if (probe_input.mutations & 16) {
        probe_actor->sequence_18 = probe_alternate;
    }
    if (probe_input.mutations & 32) {
        probe_sequence->initialized_key_28 = -2;
    }
}

static int32_t record_copy(uint32_t kind, void *output, const void *input, uint32_t count) {
    ProbeCall *call;
    assert(probe_output->call_count < 2);
    call = &probe_output->calls[probe_output->call_count++];
    call->kind = kind;
    call->output = (uint32_t)((uintptr_t)output - (uintptr_t)output_words);
    call->input = (uint32_t)((uintptr_t)input - (uintptr_t)input_words);
    call->count = count;
    if (probe_output->call_count == 1) {
        mutate_first_call();
    }
    return -1;
}

int32_t func_80010220(uint32_t *output, const uint32_t *input, uint32_t count) {
    return record_copy(0, output, input, count);
}

int32_t func_80010244(uint8_t *output, const uint8_t *input, uint32_t count) {
    return record_copy(1, output, input, count);
}

int32_t func_80010278(uint8_t *output, const uint8_t *input, uint32_t count) {
    return record_copy(2, output, input, count);
}

void func_80067DEC(PlayerActorSequenceView *component, int32_t key, uint32_t context) {
    ProbeCall *call;
    assert(component == probe_actor && probe_output->call_count == 0);
    call = &probe_output->calls[probe_output->call_count++];
    call->kind = 3;
    call->input = (uint32_t)key;
    call->count = context;
    mutate_first_call();
}

static ProbeOutput run_probe(const ProbeInput *input) {
    PlayerActorSequenceView actor = {0};
    PlayerActorSequenceData sequence = {0};
    PlayerActorSequenceData alternate = {0};
    PlayerSequenceChannels channels[2] = {{0}};
    PlayerSequenceResource resources[2] = {0};
    ProbeOutput output = {0};

    actor.sequence_18 = &sequence;
    sequence.enabled_2B = (uint8_t)input->enabled;
    sequence.flags_2C = input->flags;
    sequence.first_frames_10 = (const uint8_t *)input_words + 0x100;
    sequence.second_frames_14 = (const uint8_t *)input_words + 0x200;
    sequence.resource_20 = &resources[0];
    sequence.initialized_key_28 = 0x5678;
    alternate.initialized_key_28 = 0x1234;
    resources[0].channels_08 = &channels[0];
    resources[1].channels_08 = &channels[1];
    channels[0].first_output = (uint8_t *)output_words + 0x20;
    channels[0].first_count = (int32_t)input->first_count;
    channels[0].second_output = (uint8_t *)output_words + 0x40;
    channels[0].second_count = (int32_t)input->second_count;
    channels[1].second_output = (uint8_t *)output_words + 0x80;
    channels[1].second_count = (int32_t)input->changed_count;
    g_sequence_stream_context = (uint8_t)input->context;
    probe_actor = &actor;
    probe_sequence = &sequence;
    probe_alternate = &alternate;
    probe_resources = resources;
    probe_channels = channels;
    probe_output = &output;
    probe_input = *input;

    output.result = (uint32_t)func_80052E54(&actor, (int32_t)input->key);
    output.key = (uint16_t)sequence.initialized_key_28;
    output.actor_sequence = actor.sequence_18 == &sequence ? 0 : 1;
    output.flags = sequence.flags_2C;
    output.resource = sequence.resource_20 == &resources[0] ? 0 : 1;
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.key = 2;
    input.first_count = 2;
    input.second_count = 3;
    output = run_probe(&input);
    assert(output.result == 1 && output.key == 0x5678 && output.call_count == 0);

    input.enabled = 1;
    input.flags = 4;
    input.context = 0xAB;
    output = run_probe(&input);
    assert(output.result == 0 && output.key == 0x5678 && output.call_count == 1);
    assert(output.calls[0].kind == 3 && output.calls[0].input == 2 && output.calls[0].count == 0xAB);

    input.flags = 0;
    input.changed_flags = 0x20000000u;
    input.changed_count = 99;
    input.mutations = 63;
    output = run_probe(&input);
    assert(output.result == 0 && output.key == 2 && output.actor_sequence == 1);
    assert(output.flags == 0x20000000u && output.resource == 1 && output.call_count == 2);
    assert(output.calls[0].kind == 0 && output.calls[0].output == 0x20);
    assert(output.calls[0].input == 0x120 && output.calls[0].count == 4);
    assert(output.calls[1].kind == 2 && output.calls[1].output == 0x80);
    assert(output.calls[1].input == 0x318 && output.calls[1].count == 3);
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("sequence channel initialization tests passed");
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
            fputs("incomplete sequence-channel probe input\n", stderr);
            return 2;
        }
        output = run_probe(&input);
        if (fwrite(&output, 1, sizeof(output), stdout) != sizeof(output)) {
            perror("write sequence-channel probe output");
            return 2;
        }
    }
    if (ferror(stdin)) {
        perror("read sequence-channel probe input");
        return 2;
    }
    return 0;
}
