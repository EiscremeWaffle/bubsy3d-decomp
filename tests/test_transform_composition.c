#include "gte_basis_transform.h"
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

typedef struct ProbeInput {
    uint32_t variant, alias, mutations;
    uint32_t transformed[3], first_replacement[3], second_replacement[3];
    uint32_t words[16];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t observed[3], words[16];
} ProbeOutput;

typedef union MatrixStorage {
    ComponentRotationMatrix matrices[2];
    uint32_t words[16];
} MatrixStorage;

_Static_assert(sizeof(ProbeInput) == 28 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 19 * sizeof(uint32_t), "probe output layout");

static ProbeInput current;
static ProbeOutput output;
static ComponentRotationMatrix *first_matrix, *second_matrix;
static int32_t *saved_transformed;
static unsigned call_count;

static void replace_translation(ComponentRotationMatrix *matrix, const uint32_t values[3], uint32_t mask) {
    unsigned index;
    for (index = 0; index < 3; ++index) {
        uint32_t bits = values[index] ^ mask;
        memcpy(&matrix->translation[index], &bits, sizeof(bits));
    }
}

int32_t *func_80077298(const volatile void *basis, const int32_t *input, int32_t *result) {
    assert(call_count++ == 0);
    assert(basis == first_matrix && input == second_matrix->translation);
    memcpy(output.observed, input, sizeof(output.observed));
    if (current.mutations & 1U) {
        replace_translation(first_matrix, current.first_replacement, 0);
    }
    if (current.mutations & 2U) {
        replace_translation(second_matrix, current.second_replacement, 0);
    }
    memcpy(result, current.transformed, sizeof(current.transformed));
    saved_transformed = result;
    return result;
}

static ComponentRotationMatrix *compose_rotation(void) {
    unsigned index;
    ComponentRotationMatrix *destination = current.variant ? first_matrix : second_matrix;

    assert(call_count++ == 1);
    destination->elements[0] = 0xBEEFu;
    if (current.mutations & 4U) {
        replace_translation(first_matrix, current.first_replacement, 0x5A5A5A5AU);
    }
    if (current.mutations & 8U) {
        replace_translation(second_matrix, current.second_replacement, 0xA5A5A5A5U);
    }
    if (current.mutations & 16U) {
        for (index = 0; index < 3; ++index) {
            uint32_t bits;
            memcpy(&bits, &saved_transformed[index], sizeof(bits));
            bits ^= 0xFFFFFFFFU;
            memcpy(&saved_transformed[index], &bits, sizeof(bits));
        }
    }
    return destination;
}

ComponentRotationMatrix *func_8007699C(
    const ComponentRotationMatrix *first, ComponentRotationMatrix *second
) {
    assert(current.variant == 0 && first == first_matrix && second == second_matrix);
    return compose_rotation();
}

ComponentRotationMatrix *func_80076890(
    ComponentRotationMatrix *first, const ComponentRotationMatrix *second
) {
    assert(current.variant == 1 && first == first_matrix && second == second_matrix);
    return compose_rotation();
}

static ProbeOutput run_probe(const ProbeInput *input) {
    MatrixStorage storage;

    assert(input->variant <= 1 && input->alias <= 1);
    current = *input;
    memset(&output, 0, sizeof(output));
    memcpy(storage.words, input->words, sizeof(storage.words));
    first_matrix = &storage.matrices[0];
    second_matrix = &storage.matrices[input->alias ? 0 : 1];
    saved_transformed = 0;
    call_count = 0;
    if (input->variant) {
        func_800724B0(first_matrix, second_matrix);
    } else {
        func_8007242C(first_matrix, second_matrix);
    }
    assert(call_count == 2);
    memcpy(output.words, storage.words, sizeof(output.words));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.words[5] = 0xFFFFFFFFU;
    input.words[6] = 0x80000000U;
    input.words[7] = 12;
    input.words[13] = 8;
    input.words[14] = 9;
    input.words[15] = 10;
    input.transformed[0] = 1;
    input.transformed[1] = 0x80000000U;
    input.transformed[2] = 0xFFFFFFFFU;
    result = run_probe(&input);
    assert(result.observed[0] == 8 && result.observed[1] == 9 && result.observed[2] == 10);
    assert(result.words[13] == 0 && result.words[14] == 0 && result.words[15] == 11);
    assert((result.words[8] & 0xFFFFU) == 0xBEEFU);

    input.alias = 1;
    result = run_probe(&input);
    assert(result.observed[0] == 0xFFFFFFFFU && result.words[5] == 0);
    assert(result.words[6] == 0 && result.words[7] == 11);

    input.variant = 1;
    input.alias = 0;
    result = run_probe(&input);
    assert(result.observed[0] == 8 && result.words[5] == 0);
    assert(result.words[6] == 0 && result.words[7] == 11);
    assert(result.words[13] == 8 && result.words[14] == 9 && result.words[15] == 10);
    assert((result.words[0] & 0xFFFFU) == 0xBEEFU);

    input.alias = 1;
    result = run_probe(&input);
    assert(result.observed[0] == 0xFFFFFFFFU && result.words[5] == 0);
    assert(result.words[6] == 0 && result.words[7] == 11);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("transform composition tests passed");
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
