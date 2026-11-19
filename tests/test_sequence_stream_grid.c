#include "sequence_stream.h"

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

enum { BUFFER_BYTES = 4096 };

typedef struct ProbeInput {
    uint32_t x, y, stride, offset, table, output;
    uint8_t bytes[BUFFER_BYTES];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result;
    uint8_t bytes[BUFFER_BYTES];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 24 + BUFFER_BYTES, "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 4 + BUFFER_BYTES, "probe output layout");

static ProbeOutput run_probe(const ProbeInput *input) {
    uint32_t memory[BUFFER_BYTES / 4];
    SequenceStreamDescriptor descriptor = {0};
    SequenceStreamArguments arguments = {0};
    ProbeOutput output = {0};

    assert(input->output % 4 == 0 && input->output <= BUFFER_BYTES - 1024);
    assert(input->table + input->offset + 14 * input->stride + 9 < BUFFER_BYTES);
    memcpy(memory, input->bytes, sizeof(memory));
    descriptor.surface_00 = &memory[input->output / 4];
    arguments.descriptor_10 = &descriptor;
    arguments.remainder_scaled_08 = input->x;
    arguments.quotient_scaled_0C = input->y;
    arguments.table_14 = (const uint8_t *)memory + input->table;
    arguments.stride_18 = input->stride;
    arguments.offset_1C = input->offset;
    output.result = (uint32_t)func_80010368(&arguments);
    memcpy(output.bytes, memory, sizeof(memory));
    return output;
}

static uint32_t word(const ProbeOutput *output, unsigned offset) {
    uint32_t result;
    memcpy(&result, output->bytes + offset, sizeof(result));
    return result;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;
    input.x = 0x123;
    input.y = 0x456;
    input.stride = 9;
    input.output = 0x800;
    output = run_probe(&input);
    assert(output.result == 0);
    assert(word(&output, 0x800) == 0x123 && word(&output, 0x804) == 0x456);
    assert(word(&output, 0x800 + 8 * 8) == 0x923);
    assert(word(&output, 0x800 + 9 * 8) == 0x123);
    assert(word(&output, 0x800 + 9 * 8 + 4) == 0x556);
    assert(word(&output, 0x800 + 127 * 8) == 0x223);
    assert(word(&output, 0x800 + 127 * 8 + 4) == 0x1256);
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;
    if (argc == 1) {
        self_test();
        puts("stream-grid probe passed");
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
        ProbeOutput output;
        if (bytes_read != sizeof(input)) {
            fputs("incomplete stream-grid probe input\n", stderr);
            return 2;
        }
        output = run_probe(&input);
        if (fwrite(&output, 1, sizeof(output), stdout) != sizeof(output)) {
            perror("write stream-grid probe output");
            return 2;
        }
    }
    if (ferror(stdin)) {
        perror("read stream-grid probe input");
        return 2;
    }
    return 0;
}
