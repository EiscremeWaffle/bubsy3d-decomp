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

enum { NODE_COUNT = 5 };

typedef struct ProbeInput {
    uint32_t previous[NODE_COUNT];
    uint32_t next[NODE_COUNT];
    uint32_t active, free_head, released, count;
    uint32_t operation, owner, zero_owner, zero_previous, zero_next;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t previous[NODE_COUNT];
    uint32_t next[NODE_COUNT];
    uint32_t owner[NODE_COUNT];
    uint32_t active, free_head, count, result;
    uint32_t zero_owner, zero_previous, zero_next, assertions;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 19 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 23 * sizeof(uint32_t), "probe output layout");
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(offsetof(SequencePlaybackNode, previous_04) == 4, "previous link offset");
_Static_assert(offsetof(SequencePlaybackNode, next_08) == 8, "next link offset");
_Static_assert(sizeof(SequencePlaybackNode) == 12, "node size");
#endif

SequencePlaybackNode *g_sequence_playback_active;
SequencePlaybackNode *g_sequence_playback_free;
SequencePlaybackNode g_sequence_playback_zero_address;
uint16_t g_sequence_playback_count;
const char g_sequence_playback_free_condition[] = "free node";
const char g_sequence_playback_source_path[] = "sequence source";
static uint32_t assertion_calls;

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 1 && condition == g_sequence_playback_free_condition);
    assert(source == g_sequence_playback_source_path && line == 0xC68);
    ++assertion_calls;
}

static SequencePlaybackNode *node_pointer(SequencePlaybackNode *nodes, uint32_t index) {
    assert(index <= NODE_COUNT);
    return index == 0 ? 0 : &nodes[index - 1];
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

static void *owner_pointer(uint32_t *owners, uint32_t index) {
    assert(index <= NODE_COUNT);
    return index == 0 ? 0 : &owners[index - 1];
}

static uint32_t owner_index(const uint32_t *owners, const void *owner) {
    uint32_t index;
    if (owner == 0) {
        return 0;
    }
    for (index = 0; index < NODE_COUNT; ++index) {
        if (owner == &owners[index]) {
            return index + 1;
        }
    }
    assert(0);
    return 0;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    SequencePlaybackNode nodes[NODE_COUNT];
    uint32_t owners[NODE_COUNT];
    ProbeOutput output;
    uint32_t index;

    assert(input->operation <= 1);
    for (index = 0; index < NODE_COUNT; ++index) {
        owners[index] = index + 1;
        nodes[index].owner_00 = &owners[index];
        nodes[index].previous_04 = node_pointer(nodes, input->previous[index]);
        nodes[index].next_08 = node_pointer(nodes, input->next[index]);
    }
    g_sequence_playback_zero_address.owner_00 = owner_pointer(owners, input->zero_owner);
    g_sequence_playback_zero_address.previous_04 = node_pointer(nodes, input->zero_previous);
    g_sequence_playback_zero_address.next_08 = node_pointer(nodes, input->zero_next);
    g_sequence_playback_active = node_pointer(nodes, input->active);
    g_sequence_playback_free = node_pointer(nodes, input->free_head);
    g_sequence_playback_count = (uint16_t)input->count;
    assertion_calls = 0;
    if (input->operation == 0) {
        output.result = (uint32_t)func_80053180(node_pointer(nodes, input->released));
    } else {
        output.result = node_index(nodes, func_800530FC(owner_pointer(owners, input->owner)));
    }

    for (index = 0; index < NODE_COUNT; ++index) {
        output.previous[index] = node_index(nodes, nodes[index].previous_04);
        output.next[index] = node_index(nodes, nodes[index].next_08);
        output.owner[index] = owner_index(owners, nodes[index].owner_00);
    }
    output.active = node_index(nodes, g_sequence_playback_active);
    output.free_head = node_index(nodes, g_sequence_playback_free);
    output.count = g_sequence_playback_count;
    output.zero_owner = owner_index(owners, g_sequence_playback_zero_address.owner_00);
    output.zero_previous = node_index(nodes, g_sequence_playback_zero_address.previous_04);
    output.zero_next = node_index(nodes, g_sequence_playback_zero_address.next_08);
    output.assertions = assertion_calls;
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.released = 1;
    input.active = 1;
    input.free_head = 3;
    input.next[0] = 2;
    input.previous[1] = 1;
    input.count = 0;
    output = run_probe(&input);
    assert(output.active == 2 && output.free_head == 1);
    assert(output.previous[0] == 0 && output.next[0] == 3);
    assert(output.previous[1] == 0 && output.previous[2] == 1);
    assert(output.count == 0xFFFF && output.result == 0xFFFFFFFFU);

    memset(&input, 0, sizeof(input));
    input.released = input.free_head = 1;
    input.previous[0] = 1;
    input.next[0] = 1;
    input.count = 2;
    output = run_probe(&input);
    assert(output.previous[0] == 0 && output.next[0] == 1);
    assert(output.free_head == 1 && output.count == 1 && output.result == 1);

    memset(&input, 0, sizeof(input));
    input.operation = 1;
    input.active = 2;
    input.free_head = 1;
    input.next[0] = 3;
    input.owner = 5;
    input.count = 0xFFFF;
    output = run_probe(&input);
    assert(output.active == 1 && output.free_head == 3 && output.result == 1);
    assert(output.owner[0] == 5 && output.previous[1] == 1 && output.previous[2] == 0);
    assert(output.previous[0] == 0 && output.next[0] == 2 && output.count == 0);

    memset(&input, 0, sizeof(input));
    input.operation = 1;
    input.free_head = 1;
    input.next[0] = 3;
    input.owner = 5;
    output = run_probe(&input);
    assert(output.zero_previous == 1 && output.active == 1);
    assert(output.free_head == 3 && output.assertions == 0);

    memset(&input, 0, sizeof(input));
    input.operation = 1;
    input.active = 3;
    input.zero_next = 2;
    input.owner = 5;
    output = run_probe(&input);
    assert(output.assertions == 1 && output.result == 0);
    assert(output.active == 0 && output.free_head == 2 && output.count == 1);
    assert(output.zero_owner == 5 && output.zero_previous == 0 && output.zero_next == 3);

    memset(&input, 0, sizeof(input));
    input.released = input.active = 1;
    input.next[0] = 2;
    input.count = 1;
    output = run_probe(&input);
    assert(output.active == 2 && output.free_head == 1 && output.zero_previous == 1);
    assert(output.count == 0 && output.result == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("sequence playback list tests passed");
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
