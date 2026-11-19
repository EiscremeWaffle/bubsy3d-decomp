#include "matrix_transpose.h"

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
    uint32_t source, destination;
    uint16_t words[64];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result;
    uint16_t words[64];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 136, "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 132, "probe output layout");

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeOutput output;
    uint16_t *result;

    assert(input->source <= 55 && input->destination <= 55);
    memcpy(output.words, input->words, sizeof(output.words));
    result = func_80077C00(output.words + input->source, output.words + input->destination);
    output.result = (uint32_t)(result - output.words);
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;
    unsigned index;
    static const uint16_t in_place[] = {8, 11, 14, 11, 12, 15, 14, 15, 16};

    for (index = 0; index < 64; ++index) {
        input.words[index] = (uint16_t)index;
    }
    input.source = 8;
    input.destination = 8;
    output = run_probe(&input);
    assert(output.result == 8);
    assert(memcmp(output.words + 8, in_place, sizeof(in_place)) == 0);

    input.destination = 32;
    output = run_probe(&input);
    assert(output.result == 32);
    assert(output.words[32] == 8 && output.words[33] == 11 && output.words[34] == 14);
    assert(output.words[35] == 9 && output.words[36] == 12 && output.words[37] == 15);
    assert(output.words[38] == 10 && output.words[39] == 13 && output.words[40] == 16);
    assert(output.words[41] == input.words[41]);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("matrix transpose tests passed");
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
