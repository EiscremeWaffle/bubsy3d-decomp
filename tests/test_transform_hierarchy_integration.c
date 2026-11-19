#include "component_transform.h"
#include "gte_basis_transform.h"
#include "hierarchy_transform.h"

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

#define NODE_COUNT 4

typedef struct ProbeInput {
    uint32_t binding, count, generation, route, source_alias, status, versions[NODE_COUNT];
    uint32_t local[NODE_COUNT][8], cached[NODE_COUNT][8];
    uint32_t descriptor[8], working[8], render[8], source[8], snapshot[8];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, status, generation, basis_calls, versions[NODE_COUNT];
    uint32_t cached[NODE_COUNT][8], working[8], render[8], chain[NODE_COUNT], snapshot[8];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 114 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 68 * sizeof(uint32_t), "probe output layout");

uint32_t g_hierarchy_generation;
HierarchyTransformNode *g_hierarchy_chain[NODE_COUNT];
TransformResetMatrix g_transform_working_matrix;
TransformResetMatrix g_transform_render_matrix;
void *g_l0_gp_888;
int32_t g_transform_reset_status;
uint32_t g_transform_reset_snapshot[8];
TransformResetDescriptor g_transform_reset_descriptor;
static HierarchyTransformNode nodes[NODE_COUNT];
static unsigned basis_calls;

int32_t func_80051678(void);

int32_t *func_80077298(const volatile void *basis, const int32_t *input, int32_t *output) {
    const volatile ComponentRotationMatrix *matrix = basis;
    GameComponentBasisView model = {0};
    int32_t model_input[4] = {input[0], input[1], input[2], 0};
    int32_t model_output[4] = {0};
    unsigned index;

    ++basis_calls;
    for (index = 0; index < 9; ++index) {
        model.matrix_halfwords[index] = matrix->elements[index];
    }
    assert(game_component_basis_transform_point_q12(&model, model_input, model_output) == 0);
    memcpy(output, model_output, 3 * sizeof(int32_t));
    return output;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    uint32_t source[8];
    ProbeOutput output;
    unsigned index;

    assert(input->binding <= 1 && input->count >= 1 && input->count <= NODE_COUNT
           && input->route <= 1 && input->source_alias <= 1);
    memset(&output, 0, sizeof(output));
    memset(g_hierarchy_chain, 0, sizeof(g_hierarchy_chain));
    g_hierarchy_generation = input->generation;
    for (index = 0; index < NODE_COUNT; ++index) {
        nodes[index].generation_00 = input->versions[index];
        memcpy(&nodes[index].local_04, input->local[index], sizeof(input->local[index]));
        memcpy(&nodes[index].cached_24, input->cached[index], sizeof(input->cached[index]));
        nodes[index].unknown_44 = 0;
        nodes[index].parent_48 = index + 1 < input->count ? &nodes[index + 1] : 0;
    }
    memcpy(&g_transform_reset_descriptor.matrix_00, input->descriptor, sizeof(input->descriptor));
    g_transform_reset_descriptor.binding_20 = input->binding ? &nodes[0] : 0;
    memcpy(&g_transform_working_matrix, input->working, sizeof(input->working));
    memcpy(&g_transform_render_matrix, input->render, sizeof(input->render));
    memcpy(source, input->source, sizeof(source));
    memcpy(g_transform_reset_snapshot, input->snapshot, sizeof(input->snapshot));
    memcpy(&g_transform_reset_status, &input->status, sizeof(input->status));
    g_l0_gp_888 = input->source_alias ? g_transform_reset_snapshot : source;
    basis_calls = 0;
    output.result = (uint32_t)(input->route
        ? func_80051678() : func_800731C0(&g_transform_reset_descriptor));
    memcpy(&output.status, &g_transform_reset_status, sizeof(output.status));
    output.generation = g_hierarchy_generation;
    output.basis_calls = basis_calls;
    for (index = 0; index < NODE_COUNT; ++index) {
        output.versions[index] = nodes[index].generation_00;
        memcpy(output.cached[index], &nodes[index].cached_24, sizeof(output.cached[index]));
        if (g_hierarchy_chain[index] == 0) {
            output.chain[index] = UINT32_MAX;
        } else {
            assert(g_hierarchy_chain[index] >= nodes && g_hierarchy_chain[index] < nodes + NODE_COUNT);
            output.chain[index] = (uint32_t)(g_hierarchy_chain[index] - nodes);
        }
    }
    memcpy(output.working, &g_transform_working_matrix, sizeof(output.working));
    memcpy(output.render, &g_transform_render_matrix, sizeof(output.render));
    memcpy(output.snapshot, g_transform_reset_snapshot, sizeof(output.snapshot));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;
    TransformResetMatrix identity = {0};

    identity.rotation.elements[0] = 4096;
    identity.rotation.elements[4] = 4096;
    identity.rotation.elements[8] = 4096;
    identity.rotation.translation[0] = 11;
    memcpy(input.descriptor, &identity, sizeof(input.descriptor));
    identity.rotation.translation[0] = 2;
    memcpy(input.local[0], &identity, sizeof(input.local[0]));
    identity.rotation.translation[0] = 3;
    memcpy(input.local[1], &identity, sizeof(input.local[1]));
    input.count = 2;
    input.generation = 1;
    input.binding = 1;
    result = run_probe(&input);
    assert(result.result == 0 && result.basis_calls == 3);
    assert(result.versions[0] == 1 && result.versions[1] == 1);
    assert(result.working[5] == 6);
    assert(memcmp(result.working, result.render, sizeof(result.working)) == 0);

    input.route = 1;
    input.status = 0xFFFFFFFFU;
    input.source[0] = 0x12345678U;
    result = run_probe(&input);
    assert(result.status == 0 && result.snapshot[0] == input.source[0]);
    input.source_alias = 1;
    input.snapshot[0] = 0xABCDEF01U;
    result = run_probe(&input);
    assert(result.status == 0 && result.snapshot[0] == input.snapshot[0]);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("transform hierarchy integration tests passed");
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
