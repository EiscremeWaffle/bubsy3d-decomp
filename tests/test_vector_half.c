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
    uint32_t input_offset, output_offset, words[12];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, words[12];
} ProbeOutput;

typedef union ProbeStorage {
    uint32_t words[12];
    int32_t signed_words[12];
} ProbeStorage;

_Static_assert(sizeof(ProbeInput) == 14 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 13 * sizeof(uint32_t), "probe output layout");

extern int32_t func_80053EA0(const int32_t *input, int32_t *output);

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeStorage storage;
    ProbeOutput output;

    assert(input->input_offset <= 9 && input->output_offset <= 9);
    memcpy(storage.words, input->words, sizeof(storage.words));
    output.status = (uint32_t)func_80053EA0(
        storage.signed_words + input->input_offset, storage.signed_words + input->output_offset
    );
    memcpy(output.words, storage.words, sizeof(output.words));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.words[0] = 0x80000000U;
    input.words[1] = 0xFFFFFFFDU;
    input.words[2] = 7;
    input.output_offset = 3;
    output = run_probe(&input);
    assert(output.status == 0 && output.words[3] == 0x40000000U);
    assert(output.words[4] == 0xFFFFFFFFU && output.words[5] == 3);
    input.output_offset = 1;
    output = run_probe(&input);
    assert(output.words[1] == 0x40000000U && output.words[2] == 0x20000000U);
    assert(output.words[3] == 0x10000000U);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("vector half tests passed");
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
