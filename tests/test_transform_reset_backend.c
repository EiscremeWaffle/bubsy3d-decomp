#include "gte_basis_transform.h"
#include "hierarchy_transform.h"
#include "transform_composition.h"
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
    uint32_t binding, mutations;
    uint32_t descriptor[8], working[8], render[8], hierarchy[8];
    uint32_t transformed[3], composed[8];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, calls, observed_input[3], observed_inverse[3];
    uint32_t working_at_composition[8], working[8], render[8];
    uint16_t basis_before[10], basis_after[10];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 45 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 32 * sizeof(uint32_t) + 20 * sizeof(uint16_t), "probe output layout");

TransformResetMatrix g_transform_working_matrix;
TransformResetMatrix g_transform_render_matrix;
static ProbeInput current;
static ProbeOutput output;
static unsigned call_count;
static const uintptr_t binding_token = 0x13572468U;

void func_80073324(HierarchyTransformNode *binding, TransformResetMatrix *matrix) {
    assert(call_count++ == 0 && (uintptr_t)binding == binding_token);
    memcpy(matrix, current.hierarchy, sizeof(current.hierarchy));
    if (current.mutations & 1U) {
        g_transform_working_matrix.record.second.words[3] ^= 0x11111111U;
        g_transform_render_matrix.record.first.words[0] ^= 0x22222222U;
    }
}

int32_t *func_80077298(const volatile void *basis, const int32_t *input, int32_t *result) {
    const volatile ComponentRotationMatrix *rotation = basis;
    unsigned index;

    assert(call_count++ == 1);
    for (index = 0; index < 9; ++index) {
        output.basis_before[index] = rotation->elements[index];
    }
    memcpy(output.observed_input, input, sizeof(output.observed_input));
    if (current.mutations & 2U) {
        ComponentRotationMatrix *mutable_rotation = (ComponentRotationMatrix *)basis;
        mutable_rotation->elements[0] ^= 1U;
        g_transform_working_matrix.record.first.words[1] ^= 0x33333333U;
    }
    memcpy(result, current.transformed, sizeof(current.transformed));
    return result;
}

void func_8007242C(const ComponentRotationMatrix *first, ComponentRotationMatrix *second) {
    assert(call_count++ == 2 && first == &g_transform_working_matrix.rotation);
    memcpy(output.basis_after, second->elements, 9 * sizeof(uint16_t));
    memcpy(output.observed_inverse, second->translation, sizeof(output.observed_inverse));
    memcpy(output.working_at_composition, &g_transform_working_matrix, sizeof(output.working_at_composition));
    if (current.mutations & 4U) {
        g_transform_working_matrix.record.first.words[0] ^= 0x44444444U;
        g_transform_render_matrix.record.first.words[1] ^= 0x55555555U;
    }
    memcpy(second, current.composed, sizeof(current.composed));
}

static ProbeOutput run_probe(const ProbeInput *input) {
    TransformResetDescriptor descriptor;

    assert(input->binding <= 1);
    current = *input;
    memset(&output, 0, sizeof(output));
    memcpy(&descriptor.matrix_00, input->descriptor, sizeof(input->descriptor));
    descriptor.binding_20 = input->binding ? (HierarchyTransformNode *)binding_token : 0;
    memcpy(&g_transform_working_matrix, input->working, sizeof(input->working));
    memcpy(&g_transform_render_matrix, input->render, sizeof(input->render));
    call_count = 0;
    output.result = (uint32_t)func_800731C0(&descriptor);
    output.calls = call_count;
    memcpy(output.working, &g_transform_working_matrix, sizeof(output.working));
    memcpy(output.render, &g_transform_render_matrix, sizeof(output.render));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;
    unsigned index;

    for (index = 0; index < 8; ++index) {
        input.descriptor[index] = index + 10;
        input.render[index] = index + 40;
        input.composed[index] = index + 80;
    }
    result = run_probe(&input);
    assert(result.result == 0 && result.calls == 0);
    assert(memcmp(result.working, input.descriptor, sizeof(input.descriptor)) == 0);
    assert(memcmp(result.render, input.render, sizeof(input.render)) == 0);

    input.binding = 1;
    input.transformed[0] = 1;
    input.transformed[1] = 0x80000000U;
    input.transformed[2] = 0xFFFFFFFFU;
    result = run_probe(&input);
    assert(result.result == 0 && result.calls == 3);
    assert(result.observed_inverse[0] == 0xFFFFFFFFU);
    assert(result.observed_inverse[1] == 0x80000000U);
    assert(result.observed_inverse[2] == 1);
    assert(memcmp(result.working, input.composed, sizeof(input.composed)) == 0);
    assert(memcmp(result.render, input.composed, sizeof(input.composed)) == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("transform reset backend tests passed");
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
