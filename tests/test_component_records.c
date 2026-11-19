#include "component_records.h"

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
    uint32_t operation, index, offset, words[32];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, words[32];
} ProbeOutput;

typedef union RecordStorage {
    ComponentRecord records[4];
    uint32_t words[32];
} RecordStorage;

_Static_assert(sizeof(ProbeInput) == 35 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 33 * sizeof(uint32_t), "probe output layout");

static ProbeOutput run_probe(const ProbeInput *input) {
    RecordStorage storage;
    ComponentRecordTable table;
    ComponentRecordOwner owner = {{0}, &table};
    ProbeOutput output;

    assert(input->index < 4 && input->offset <= 24);
    memcpy(storage.words, input->words, sizeof(storage.words));
    table.records_00 = storage.records;
    if (input->operation == 0) {
        output.result = (uint32_t)func_80052094(&owner, input->index, storage.words + input->offset);
    } else {
        assert(input->operation == 1);
        output.result = (uint32_t)func_800528A4(&owner, input->index, storage.words + input->offset);
    }
    memcpy(output.words, storage.words, sizeof(output.words));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;
    unsigned index;

    for (index = 0; index < 32; ++index) {
        input.words[index] = index;
    }
    input.offset = 1;
    output = run_probe(&input);
    assert(output.result == 0 && output.words[1] == 0 && output.words[4] == 3);
    assert(output.words[5] == 3 && output.words[8] == 7);

    input.operation = 1;
    output = run_probe(&input);
    assert(output.result == 0 && output.words[0] == 1 && output.words[3] == 4);
    assert(output.words[4] == 5 && output.words[7] == 8);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("component record tests passed");
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
