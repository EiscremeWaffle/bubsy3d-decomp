#include <stdint.h>

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
    uint32_t first[4], second[4], result, alias, null_basis, output_alias, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, calls, result, first_sample[4], second_sample[4], first[4], second[4];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 13 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 19 * sizeof(uint32_t), "probe output layout");

extern int32_t func_800590B4(void *first, void *second, void *basis, int32_t *output);

static ProbeInput current;
static ProbeOutput output;
static uint32_t objects[2][4], *second_object, basis;
static int32_t *first_position;

int32_t func_80058E7C(void *object, void *input_basis, int32_t *position) {
    assert(input_basis == (current.null_basis ? NULL : &basis));
    if (output.calls == 0) {
        assert(object == objects[0]);
        memcpy(position, object, 16);
        first_position = position;
        if (current.mutations & 1U) {
            second_object[0] ^= 0xA5A5A5A5U;
        }
    } else {
        assert(output.calls == 1 && object == second_object);
        memcpy(position, object, 16);
        if (current.mutations & 2U) {
            uint32_t changed;
            memcpy(&changed, first_position, sizeof(changed));
            changed ^= 0x12345678U;
            memcpy(first_position, &changed, sizeof(changed));
        }
    }
    output.calls++;
    return -1;
}

int32_t func_80062D4C(const int32_t *first, const int32_t *second, int32_t *result) {
    assert(output.calls++ == 2);
    memcpy(output.first_sample, first, sizeof(output.first_sample));
    memcpy(output.second_sample, second, sizeof(output.second_sample));
    memcpy(result, &current.result, sizeof(current.result));
    return -2;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t separate_result = 0;
    int32_t *result;

    current = *input;
    memset(&output, 0, sizeof(output));
    memcpy(objects[0], input->first, sizeof(input->first));
    memcpy(objects[1], input->second, sizeof(input->second));
    second_object = input->alias ? objects[0] : objects[1];
    result = input->output_alias ? (int32_t *)objects[0] : &separate_result;
    output.status = (uint32_t)func_800590B4(
        objects[0], second_object, input->null_basis ? NULL : &basis, result
    );
    memcpy(&output.result, result, sizeof(output.result));
    memcpy(output.first, objects[0], sizeof(output.first));
    memcpy(output.second, second_object, sizeof(output.second));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.first[0] = 3;
    input.second[0] = 7;
    input.result = 0xDEADBEEFU;
    input.mutations = 3;
    result = run_probe(&input);
    assert(result.status == 0 && result.calls == 3 && result.result == input.result);
    assert(result.first_sample[0] == (3U ^ 0x12345678U));
    assert(result.second_sample[0] == (7U ^ 0xA5A5A5A5U));
    input.alias = 1;
    input.output_alias = 1;
    result = run_probe(&input);
    assert(result.first[0] == input.result && result.second[0] == input.result);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("secondary distance tests passed");
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
