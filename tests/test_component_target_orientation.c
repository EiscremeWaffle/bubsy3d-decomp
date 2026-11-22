#include "component_rotation.h"

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
    uint32_t null_component, null_translation, null_basis, mutations;
    uint32_t translation[3], first_status, second_status;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, calls, initial_target[8], translation[3], final_target[8], component[8];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 9 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 29 * sizeof(uint32_t), "probe output layout");

const ComponentRotationMatrix g_component_identity_basis = {
    {0x1000, 0, 0, 0, 0x1000, 0, 0, 0, 0x1000}, 0, {0, 0, 0}
};
static ComponentRotationMatrix component, basis;
static ProbeInput current;
static ProbeOutput output;

int32_t func_80054C7C(
    ComponentRotationMatrix *target, const ComponentRotationMatrix *input_basis,
    const int32_t translation[3]
) {
    assert(output.calls++ == 0);
    assert(input_basis == (current.null_basis ? NULL : &basis));
    memcpy(output.initial_target, target, sizeof(*target));
    memcpy(output.translation, translation, sizeof(output.translation));
    memcpy(target->translation, translation, sizeof(output.translation));
    if (current.mutations & 1U) {
        component.elements[0] ^= 0x1111U;
    }
    return (int32_t)current.first_status;
}

int32_t func_800550A4(ComponentRotationMatrix *input_component, const ComponentRotationMatrix *target) {
    assert(output.calls++ == 1 && input_component == &component);
    memcpy(output.final_target, target, sizeof(*target));
    if (current.mutations & 2U) {
        component.elements[1] ^= 0x2222U;
    }
    return (int32_t)current.second_status;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    static const uint32_t initial_component[8] = {
        0x12345678, 0x9ABCDEF0, 0x1234ABCD, 0x12344321,
        0xBEEFCAFE, 1, 2, 3
    };
    int32_t translation[3];

    current = *input;
    memcpy(translation, input->translation, sizeof(translation));
    memset(&output, 0, sizeof(output));
    memset(&basis, 0xA5, sizeof(basis));
    memcpy(&component, initial_component, sizeof(component));
    output.status = (uint32_t)func_8005536C(
        input->null_component ? NULL : &component,
        input->null_basis ? NULL : &basis,
        input->null_translation ? NULL : translation
    );
    memcpy(output.component, &component, sizeof(component));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.null_component = 1;
    input.null_translation = 1;
    result = run_probe(&input);
    assert(result.status == 0x18 && result.calls == 0);
    input.null_component = 0;
    result = run_probe(&input);
    assert(result.status == 1 && result.calls == 0);
    input.null_translation = 0;
    input.mutations = 3;
    input.translation[0] = 0x12345678;
    input.first_status = 1;
    input.second_status = 2;
    result = run_probe(&input);
    assert(result.status == 0 && result.calls == 2);
    assert(result.initial_target[0] == 0x1000 && result.final_target[5] == 0x12345678);
    assert(result.component[0] == (0x12345678U ^ 0x22221111U));
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("component target orientation tests passed");
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
