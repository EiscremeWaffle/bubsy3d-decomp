#include "gte_basis_transform.h"
#include "hierarchy_transform.h"
#include "transform_reset_backend.h"

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
    uint32_t binding;
    uint32_t descriptor[8], hierarchy[8], working[8], render[8];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, hierarchy_calls, transform_calls, working[8], render[8];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 33 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 19 * sizeof(uint32_t), "probe output layout");

TransformResetMatrix g_transform_working_matrix;
TransformResetMatrix g_transform_render_matrix;
static ProbeInput current;
static unsigned hierarchy_calls, transform_calls;
static const uintptr_t binding_token = 0x13572468U;

void func_80073324(HierarchyTransformNode *binding, TransformResetMatrix *output) {
    assert((uintptr_t)binding == binding_token);
    ++hierarchy_calls;
    memcpy(output, current.hierarchy, sizeof(current.hierarchy));
}

int32_t *func_80077298(const volatile void *basis, const int32_t *input, int32_t *output) {
    const volatile ComponentRotationMatrix *matrix = basis;
    unsigned row, column;

    ++transform_calls;
    for (row = 0; row < 3; ++row) {
        int64_t sum = 0;
        int64_t shifted;
        for (column = 0; column < 3; ++column) {
            uint16_t bits = matrix->elements[row * 3 + column];
            int32_t coefficient = bits < 0x8000U ? (int32_t)bits : (int32_t)bits - 0x10000;
            assert(input[column] >= -1000000 && input[column] <= 1000000);
            sum += (int64_t)coefficient * input[column];
        }
        shifted = sum >= 0 ? sum / 4096 : -((-sum + 4095) / 4096);
        assert(shifted >= INT32_MIN && shifted <= INT32_MAX);
        output[row] = (int32_t)shifted;
    }
    return output;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    TransformResetDescriptor descriptor;
    ProbeOutput output;

    assert(input->binding <= 1);
    current = *input;
    memcpy(&descriptor.matrix_00, input->descriptor, sizeof(input->descriptor));
    descriptor.binding_20 = input->binding ? (HierarchyTransformNode *)binding_token : 0;
    memcpy(&g_transform_working_matrix, input->working, sizeof(input->working));
    memcpy(&g_transform_render_matrix, input->render, sizeof(input->render));
    hierarchy_calls = 0;
    transform_calls = 0;
    output.result = (uint32_t)func_800731C0(&descriptor);
    output.hierarchy_calls = hierarchy_calls;
    output.transform_calls = transform_calls;
    memcpy(output.working, &g_transform_working_matrix, sizeof(output.working));
    memcpy(output.render, &g_transform_render_matrix, sizeof(output.render));
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
    identity.rotation.translation[1] = -12;
    identity.rotation.translation[2] = 13;
    memcpy(input.descriptor, &identity, sizeof(input.descriptor));
    identity.rotation.translation[0] = 2;
    identity.rotation.translation[1] = 3;
    identity.rotation.translation[2] = 4;
    memcpy(input.hierarchy, &identity, sizeof(input.hierarchy));
    input.binding = 1;
    result = run_probe(&input);
    assert(result.result == 0 && result.hierarchy_calls == 1 && result.transform_calls == 2);
    assert(result.working[5] == 9 && result.working[6] == (uint32_t)-15 && result.working[7] == 9);
    assert(memcmp(result.working, result.render, sizeof(result.working)) == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("transform reset with real rotation composition tests passed");
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
