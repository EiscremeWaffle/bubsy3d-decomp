#include "component_bounds.h"

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

#define EMPTY_VALUE UINT32_MAX

typedef struct ProbeInput {
    uint32_t first_status, second_status, index, mutation, initial[8], record[8];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, calls, index, bounds[8];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 20 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 11 * sizeof(uint32_t), "probe output layout");

static ProbeInput current;
static ProbeOutput output;
static ComponentRecordOwner owner;
static ComponentBounds bounds;

static int32_t signed_bits(uint32_t value) {
    int32_t result;
    memcpy(&result, &value, sizeof(result));
    return result;
}

int32_t func_800529F8(ComponentRecordOwner *object, int32_t *index) {
    assert(output.calls++ == 0 && object == &owner);
    *index = signed_bits(current.index);
    if (current.mutation) {
        bounds.vectors.lower.x ^= 0xA5A5A5A5U;
    }
    return signed_bits(current.first_status);
}

int32_t func_80052094(ComponentRecordOwner *object, uint32_t index, uint32_t *target) {
    assert(output.calls++ == 1 && object == &owner && target == bounds.words);
    output.index = index;
    memcpy(target, current.record, sizeof(current.record));
    return signed_bits(current.second_status);
}

static ProbeOutput run_probe(const ProbeInput *input) {
    current = *input;
    memset(&owner, 0, sizeof(owner));
    memset(&output, 0, sizeof(output));
    memcpy(&bounds, input->initial, sizeof(bounds));
    output.index = EMPTY_VALUE;
    output.status = (uint32_t)func_80058C74(&owner, &bounds);
    memcpy(output.bounds, &bounds, sizeof(bounds));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.first_status = 7;
    input.mutation = 1;
    result = run_probe(&input);
    assert(result.status == 7 && result.calls == 1 && result.bounds[0] == 0xA5A5A5A5U);
    input.first_status = 0;
    input.second_status = 9;
    input.record[1] = 0x80000000U;
    input.record[5] = 3;
    result = run_probe(&input);
    assert(result.status == 9 && result.calls == 2 && result.bounds[1] == 0x80000000U);
    input.second_status = 0;
    result = run_probe(&input);
    assert(result.status == 0 && result.bounds[1] == 0xFFFFFFFDU);
    assert(result.bounds[5] == 0x80000000U);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("component bounds tests passed");
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
