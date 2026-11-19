#include "input_events.h"
#include "player_model.h"
#include "sequence_playback.h"

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

enum { SLOT_COUNT = 31, NODE_COUNT = 3 };

typedef struct ProbeSlot {
    uint32_t active, timestamp, key, value;
} ProbeSlot;

typedef struct ProbeInput {
    uint32_t actor_present, sequence_present;
    int32_t mode;
    uint32_t actor_flags, has_node;
    int32_t slot;
    uint32_t sequence_flags;
    int32_t cursor, entry;
    uint32_t table_present, sample, count, free_present, mutations;
    ProbeSlot slots[SLOT_COUNT];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, actor_flags, slot, actor_node, sequence;
    uint32_t active, free_head, count;
    uint32_t zero_owner, zero_previous, zero_next;
    uint32_t assertions, assertion_code[2], assertion_line[2];
    uint32_t initialize_calls, initialize_key;
    uint32_t previous[NODE_COUNT], next[NODE_COUNT], owner[NODE_COUNT];
    ProbeSlot slots[SLOT_COUNT];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 138 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 151 * sizeof(uint32_t), "probe output layout");
_Static_assert(offsetof(SequencePlaybackSlot, active_08) == 8, "slot active offset");
_Static_assert(offsetof(SequencePlaybackSlot, timestamp_0C) == 12, "slot timestamp offset");
_Static_assert(offsetof(SequencePlaybackSlot, key_10) == 16, "slot key offset");
_Static_assert(offsetof(SequencePlaybackSlot, value_14) == 20, "slot value offset");
#if UINTPTR_MAX == 0xFFFFFFFFu
_Static_assert(sizeof(SequencePlaybackSlot) == 24, "target slot size");
#else
_Static_assert(offsetof(SequencePlaybackSlot, stream_owner_04) == 24, "host slot owner");
#endif

SequencePlaybackNode *g_sequence_playback_active;
SequencePlaybackNode *g_sequence_playback_free;
SequencePlaybackNode g_sequence_playback_zero_address;
uint16_t g_sequence_playback_count;
uint32_t g_input_sample_counter;
const char g_sequence_playback_free_condition[] = "free node";
const char g_sequence_playback_slot_condition[] = "free slot";
const char g_sequence_playback_source_path[] = "sequence source";

static PlayerActorSequenceView *probe_actor;
static PlayerActorSequenceData *probe_alternate;
static SequencePlaybackNode *probe_nodes;
static uint32_t probe_mutations;
static ProbeOutput *probe_output;

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    uint32_t index = probe_output->assertions++;
    assert(index < 2 && source == g_sequence_playback_source_path);
    if (line == 0xC68) {
        assert(failed == 1 && condition == g_sequence_playback_free_condition);
        if (probe_mutations & 1) {
            probe_actor->sequence_18 = probe_alternate;
        }
        if (probe_mutations & 2) {
            g_sequence_playback_active = &probe_nodes[0];
            g_sequence_playback_count = 0xFFFF;
        }
    } else {
        assert(line == 0xB3F && failed == 0 && condition == g_sequence_playback_slot_condition);
        if (probe_mutations & 4) {
            probe_actor->playback_slot_06 = 4;
        }
    }
    probe_output->assertion_code[index] = (uint32_t)failed;
    probe_output->assertion_line[index] = line;
}

int32_t func_80052E54(PlayerActorSequenceView *component, int32_t key) {
    assert(component == probe_actor);
    ++probe_output->initialize_calls;
    probe_output->initialize_key = (uint32_t)key;
    return 17;
}

static uint32_t node_index(const SequencePlaybackNode *nodes, const SequencePlaybackNode *node) {
    uint32_t index;
    if (node == 0) {
        return 0;
    }
    for (index = 0; index < NODE_COUNT; ++index) {
        if (node == &nodes[index]) {
            return index + 1;
        }
    }
    assert(0);
    return 0;
}

static uint32_t owner_index(const PlayerActorSequenceView *actor, const void *owner) {
    assert(owner == 0 || owner == actor);
    return owner != 0;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    PlayerActorSequenceView actor = {0};
    PlayerActorSequenceData sequence = {0};
    PlayerActorSequenceData alternate = {0};
    SequencePlaybackSlot slots[SLOT_COUNT] = {{0}};
    SequencePlaybackNode nodes[NODE_COUNT] = {{0}};
    int32_t entry = input->entry;
    ProbeOutput output = {0};
    uint32_t index;

    sequence.entries = &entry;
    sequence.flags_2C = input->sequence_flags;
    sequence.playback_slots = input->table_present ? slots : 0;
    alternate.flags_2C = 4;
    actor.sequence_18 = input->sequence_present ? &sequence : 0;
    actor.flags_04 = (uint8_t)input->actor_flags;
    actor.playback_slot_06 = (int8_t)input->slot;
    actor.cursor_08 = (int16_t)input->cursor;
    actor.playback_node_00 = input->has_node ? &nodes[0] : 0;

    for (index = 0; index < SLOT_COUNT; ++index) {
        slots[index].active_08 = (uint8_t)input->slots[index].active;
        slots[index].timestamp_0C = (int32_t)input->slots[index].timestamp;
        slots[index].key_10 = (uint8_t)input->slots[index].key;
        slots[index].value_14 = input->slots[index].value;
    }
    nodes[0].owner_00 = &actor;
    nodes[1].next_08 = &nodes[2];
    nodes[2].previous_04 = &nodes[1];
    g_sequence_playback_zero_address.owner_00 = 0;
    g_sequence_playback_zero_address.previous_04 = &nodes[2];
    g_sequence_playback_zero_address.next_08 = &nodes[2];
    g_sequence_playback_active = input->has_node ? &nodes[0] : 0;
    g_sequence_playback_free = input->free_present ? &nodes[1] : 0;
    g_sequence_playback_count = (uint16_t)input->count;
    g_input_sample_counter = input->sample;

    probe_actor = &actor;
    probe_alternate = &alternate;
    probe_nodes = nodes;
    probe_mutations = input->mutations;
    probe_output = &output;
    output.result = (uint32_t)func_80052BDC(input->actor_present ? &actor : 0, input->mode);
    output.actor_flags = actor.flags_04;
    output.slot = (uint8_t)actor.playback_slot_06;
    output.actor_node = node_index(nodes, actor.playback_node_00);
    output.sequence = actor.sequence_18 == &sequence ? 1 : actor.sequence_18 == &alternate ? 2 : 0;
    output.active = node_index(nodes, g_sequence_playback_active);
    output.free_head = node_index(nodes, g_sequence_playback_free);
    output.count = g_sequence_playback_count;
    output.zero_owner = owner_index(&actor, g_sequence_playback_zero_address.owner_00);
    output.zero_previous = node_index(nodes, g_sequence_playback_zero_address.previous_04);
    output.zero_next = node_index(nodes, g_sequence_playback_zero_address.next_08);
    for (index = 0; index < NODE_COUNT; ++index) {
        output.previous[index] = node_index(nodes, nodes[index].previous_04);
        output.next[index] = node_index(nodes, nodes[index].next_08);
        output.owner[index] = owner_index(&actor, nodes[index].owner_00);
    }
    for (index = 0; index < SLOT_COUNT; ++index) {
        output.slots[index].active = slots[index].active_08;
        output.slots[index].timestamp = (uint32_t)slots[index].timestamp_0C;
        output.slots[index].key = slots[index].key_10;
        output.slots[index].value = slots[index].value_14;
    }
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    output = run_probe(&input);
    assert(output.result == 1 && output.actor_flags == 0);

    memset(&input, 0, sizeof(input));
    input.actor_present = input.sequence_present = input.table_present = input.free_present = 1;
    input.sequence_flags = 3 << 3;
    input.slot = -1;
    input.entry = 2017;
    input.slots[0].timestamp = 50;
    input.slots[1].timestamp = 40;
    input.slots[1].key = 17;
    input.slots[1].value = 99;
    output = run_probe(&input);
    assert(output.result == 0 && output.actor_node == 2 && output.slot == 1);
    assert(output.slots[1].active == 1 && output.slots[1].value == 99);
    assert(output.initialize_calls == 0 && output.zero_previous == 2);

    input.slots[1].key = 18;
    output = run_probe(&input);
    assert(output.slot == 2 && output.slots[2].active == 1);
    assert(output.initialize_calls == 1 && output.initialize_key == 17);

    memset(&input, 0, sizeof(input));
    input.actor_present = input.sequence_present = input.table_present = input.has_node = 1;
    input.mode = 1;
    input.slot = 0;
    input.sample = 0xFFFFFFFFU;
    input.count = 1;
    input.slots[0].active = 1;
    output = run_probe(&input);
    assert(output.actor_node == 0 && output.slot == 0xFF && output.free_head == 1);
    assert(output.slots[0].active == 0 && output.slots[0].timestamp == 0xFFFFFFFFU);
    assert(output.count == 0 && output.zero_previous == 1);

    memset(&input, 0, sizeof(input));
    input.actor_present = input.sequence_present = input.table_present = 1;
    input.slot = -1;
    input.mutations = 7;
    output = run_probe(&input);
    assert(output.assertions == 2 && output.assertion_line[0] == 0xC68);
    assert(output.assertion_line[1] == 0xB3F && output.sequence == 2);
    assert(output.slot == 0xFF && output.count == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("sequence mode tests passed");
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
            fputs("incomplete probe input\n", stderr);
            return 2;
        }
        output = run_probe(&input);
        if (fwrite(&output, sizeof(output), 1, stdout) != 1) {
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
