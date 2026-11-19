#include "hierarchy_transform.h"
#include "transform_composition.h"

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

#ifndef HIERARCHY_TEST_MAX_CHAIN
#define HIERARCHY_TEST_MAX_CHAIN 8
#endif
#define MAX_CHAIN HIERARCHY_TEST_MAX_CHAIN
#define NODE_COUNT (MAX_CHAIN + 1)

typedef struct ProbeInput {
    uint32_t count, generation, mutations;
    uint32_t versions[NODE_COUNT];
    uint32_t local[NODE_COUNT][8], cached[NODE_COUNT][8];
    uint32_t output[8];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t calls, call_nodes[MAX_CHAIN], generation;
    uint32_t versions[NODE_COUNT], cached[NODE_COUNT][8];
    uint32_t chain[MAX_CHAIN], output[8];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == (11 + NODE_COUNT * 17) * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == (10 + 2 * MAX_CHAIN + NODE_COUNT * 9) * sizeof(uint32_t),
               "probe output layout");

uint32_t g_hierarchy_generation;
HierarchyTransformNode *g_hierarchy_chain[MAX_CHAIN];
static HierarchyTransformNode nodes[NODE_COUNT];
static TransformResetMatrix result_matrix;
static ProbeInput current;
static ProbeOutput output;

void func_800724B0(ComponentRotationMatrix *first, const ComponentRotationMatrix *second) {
    unsigned index, slot;

    assert(first == &result_matrix.rotation);
    for (index = 0; index < current.count; ++index) {
        if (second == &nodes[index].local_04.rotation) {
            break;
        }
    }
    assert(index < current.count && output.calls < MAX_CHAIN);
    output.call_nodes[output.calls++] = index;
    for (slot = 0; slot < MAX_CHAIN; ++slot) {
        if (g_hierarchy_chain[slot] == &nodes[index]) {
            break;
        }
    }
    assert(slot < MAX_CHAIN);
    for (index = 0; index < 8; ++index) {
        result_matrix.words[index] += nodes[output.call_nodes[output.calls - 1]].local_04.words[index]
            + output.calls * 0x01010101U;
    }
    if (current.mutations & 1U) {
        g_hierarchy_generation ^= 0xA5A50000U ^ output.calls;
    }
    if (current.mutations & 2U) {
        g_hierarchy_chain[slot] = &nodes[NODE_COUNT - 1];
    }
}

static ProbeOutput run_probe(const ProbeInput *input) {
    unsigned index;

    assert(input->count >= 1 && input->count <= MAX_CHAIN && input->mutations <= 3);
    current = *input;
    memset(&output, 0, sizeof(output));
    memset(output.call_nodes, 0xFF, sizeof(output.call_nodes));
    memset(g_hierarchy_chain, 0, sizeof(g_hierarchy_chain));
    g_hierarchy_generation = input->generation;
    memcpy(&result_matrix, input->output, sizeof(input->output));
    for (index = 0; index < NODE_COUNT; ++index) {
        nodes[index].generation_00 = input->versions[index];
        memcpy(&nodes[index].local_04, input->local[index], sizeof(input->local[index]));
        memcpy(&nodes[index].cached_24, input->cached[index], sizeof(input->cached[index]));
        nodes[index].unknown_44 = 0;
        nodes[index].parent_48 = index + 1 < input->count ? &nodes[index + 1] : 0;
    }
    func_80073324(&nodes[0], &result_matrix);
    output.generation = g_hierarchy_generation;
    for (index = 0; index < NODE_COUNT; ++index) {
        output.versions[index] = nodes[index].generation_00;
        memcpy(output.cached[index], &nodes[index].cached_24, sizeof(output.cached[index]));
    }
    for (index = 0; index < MAX_CHAIN; ++index) {
        if (g_hierarchy_chain[index] == 0) {
            output.chain[index] = UINT32_MAX;
        } else {
            assert(g_hierarchy_chain[index] >= nodes && g_hierarchy_chain[index] < nodes + NODE_COUNT);
            output.chain[index] = (uint32_t)(g_hierarchy_chain[index] - nodes);
        }
    }
    memcpy(output.output, &result_matrix, sizeof(output.output));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;
    unsigned index;

    input.count = 2;
    input.generation = 7;
    for (index = 0; index < 8; ++index) {
        input.local[0][index] = 10 + index;
        input.local[1][index] = 1 + index;
    }
    result = run_probe(&input);
    assert(result.calls == 1 && result.call_nodes[0] == 0);
    assert(result.versions[0] == 7 && result.versions[1] == 7);
    for (index = 0; index < 8; ++index) {
        assert(result.cached[1][index] == input.local[1][index]);
        assert(result.output[index] == input.local[0][index] + input.local[1][index] + 0x01010101U);
        assert(result.cached[0][index] == result.output[index]);
    }
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("hierarchy transform tests passed");
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
        if (bytes_read != sizeof(input)) {
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
