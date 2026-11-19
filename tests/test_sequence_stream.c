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

typedef struct ProbeInput {
    uint32_t slot, key, context, mutations, changed_slot;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t calls;
    uint32_t arguments[9];
    uint32_t transfer[4];
    uint32_t keys[10];
    uint32_t actor_sequence, sequence_slots, actor_slot, sequence_lookup;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 5 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 28 * sizeof(uint32_t), "probe output layout");

const char g_sequence_stream_width_condition[] = "terrainWidth";
const char g_sequence_stream_source_path[] = "../f/egsfast.c";

static PlayerActorSequenceView *probe_actor;
static PlayerActorSequenceData *probe_sequence;
static PlayerActorSequenceData *probe_alternate;
static SequencePlaybackSlot (*probe_slots)[5];
static SequenceStreamDescriptor *probe_descriptors;
static ProbeOutput *probe_output;
static ProbeInput probe_input;
static uint8_t frames[4096], outputs[4096], metadata[4096], tables[4096];
static uint32_t lookups[1024];

static uint32_t offset(const void *pointer, const void *base) {
    return (uint32_t)((uintptr_t)pointer - (uintptr_t)base);
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0 && condition == g_sequence_stream_width_condition);
    assert(source == g_sequence_stream_source_path && line == 0xED);
    fputs("stream width assertion returned\n", stderr);
    fflush(stderr);
}

int32_t func_80010368(SequenceStreamArguments *arguments) {
    uint32_t *record = probe_output->arguments;
    assert(probe_output->calls++ == 0);
    record[0] = (uint32_t)arguments->quotient_00;
    record[1] = (uint32_t)arguments->remainder_04;
    record[2] = arguments->remainder_scaled_08;
    record[3] = arguments->quotient_scaled_0C;
    record[4] = (uint32_t)(arguments->descriptor_10 - probe_descriptors);
    record[5] = offset(arguments->table_14, tables);
    record[6] = arguments->stride_18;
    record[7] = arguments->offset_1C;
    record[8] = offset(arguments->lookup_20, lookups);
    if (probe_input.mutations & 1) {
        probe_sequence->second_frames_14 = (const uint8_t *)(lookups + 64);
    }
    if (probe_input.mutations & 2) {
        probe_actor->playback_slot_06 = (int8_t)probe_input.changed_slot;
    }
    if (probe_input.mutations & 4) {
        probe_sequence->playback_slots = &probe_slots[1][2];
    }
    if (probe_input.mutations & 8) {
        probe_actor->sequence_18 = probe_alternate;
    }
    if (probe_input.mutations & 16) {
        arguments->descriptor_10->output_08 = outputs + 0x700;
        arguments->descriptor_10->metadata_10 = metadata + 0x700;
    }
    if (probe_input.mutations & 32) {
        probe_sequence->first_frames_10 = frames + 0x900;
    }
    return -1;
}

int32_t func_800102BC(uint8_t *output, const uint8_t *input, void *entries, const uint32_t *lookup) {
    uint32_t *record = probe_output->transfer;
    assert(probe_output->calls++ == 1);
    record[0] = offset(output, outputs);
    record[1] = offset(input, frames + 0x800);
    record[2] = offset(entries, metadata);
    record[3] = offset(lookup, lookups);
    if (probe_input.mutations & 64) {
        probe_actor->playback_slot_06 = (int8_t)probe_input.changed_slot;
    }
    if (probe_input.mutations & 128) {
        probe_sequence->playback_slots = &probe_slots[1][2];
    }
    if (probe_input.mutations & 256) {
        probe_actor->sequence_18 = probe_alternate;
    }
    if (probe_input.mutations & 512) {
        probe_sequence->second_frames_14 = (const uint8_t *)(lookups + 128);
    }
    return -1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    PlayerActorSequenceView actor = {0};
    PlayerActorSequenceData sequence = {0}, alternate = {0};
    SequencePlaybackSlot slots[2][5] = {{{0}}};
    SequenceStreamOwner owners[5] = {{0}};
    SequenceStreamDescriptor descriptors[5] = {{0}};
    ProbeOutput output = {0};
    unsigned table, index;

    actor.sequence_18 = &sequence;
    actor.playback_slot_06 = (int8_t)input->slot;
    sequence.first_frames_10 = frames + 0x800;
    sequence.second_frames_14 = (const uint8_t *)lookups;
    sequence.stream_table_18 = tables + 0x100;
    sequence.playback_slots = &slots[0][2];
    for (index = 0; index < 5; ++index) {
        descriptors[index].output_08 = outputs + index * 0x40;
        descriptors[index].metadata_10 = metadata + index * 0x40;
        owners[index].descriptor_08 = &descriptors[index];
        for (table = 0; table < 2; ++table) {
            slots[table][index].stream_owner_04 = &owners[index];
            slots[table][index].key_10 = (uint8_t)(0x40 + table * 0x20 + index);
        }
    }
    probe_actor = &actor;
    probe_sequence = &sequence;
    probe_alternate = &alternate;
    probe_slots = slots;
    probe_descriptors = descriptors;
    probe_output = &output;
    probe_input = *input;
    func_80067DEC(&actor, (int32_t)input->key, input->context);
    for (table = 0; table < 2; ++table) {
        for (index = 0; index < 5; ++index) {
            output.keys[table * 5 + index] = slots[table][index].key_10;
        }
    }
    output.actor_sequence = actor.sequence_18 == &alternate;
    output.sequence_slots = sequence.playback_slots == &slots[1][2];
    output.actor_slot = (uint32_t)(int32_t)actor.playback_slot_06;
    output.sequence_lookup = offset(sequence.second_frames_14, lookups);
    return output;
}

static void self_test(void) {
    ProbeInput input = {(uint32_t)-1, 0, 0, 0, 0};
    ProbeOutput output = run_probe(&input);
    assert(output.calls == 0);
    input.slot = 0;
    input.key = (uint32_t)-17;
    input.context = 5;
    input.mutations = 1 | 16 | 64 | 128 | 256;
    input.changed_slot = (uint32_t)-2;
    output = run_probe(&input);
    assert(output.calls == 2);
    assert(output.arguments[0] == (uint32_t)-3 && output.arguments[1] == (uint32_t)-2);
    assert(output.arguments[2] == 0xFFFFF000u && output.arguments[3] == 0xFFFFE800u);
    assert(output.arguments[4] == 2 && output.arguments[5] == 0x100);
    assert(output.arguments[6] == 41 && output.arguments[7] == (uint32_t)-1000);
    assert(output.arguments[8] == 0 && output.transfer[0] == 0x80);
    assert(output.transfer[1] == (uint32_t)-8704 && output.transfer[2] == 0x80);
    assert(output.transfer[3] == 0x100 && output.keys[5] == (uint8_t)-17);
    assert(output.actor_sequence == 1 && output.sequence_slots == 1);
    assert(output.actor_slot == (uint32_t)-2 && output.sequence_lookup == 0x100);
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;
    if (argc == 1) {
        self_test();
        puts("stream backend probe passed");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--trap") == 0) {
        ProbeInput zero = {0, 1, 0x100, 0, 0};
        (void)run_probe(&zero);
        fputs("stream division unexpectedly returned\n", stderr);
        return 1;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) {
        fputs("expected --binary, --trap or no arguments\n", stderr);
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
            fputs("incomplete stream probe input\n", stderr);
            return 2;
        }
        output = run_probe(&input);
        if (fwrite(&output, 1, sizeof(output), stdout) != sizeof(output)) {
            perror("write stream probe output");
            return 2;
        }
    }
    if (ferror(stdin)) {
        perror("read stream probe input");
        return 2;
    }
    return 0;
}
