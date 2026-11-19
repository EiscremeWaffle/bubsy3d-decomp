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

enum { BUFFER_BYTES = 8192, RECORD_COUNT = 5 };

typedef struct ProbeInput {
    uint32_t count, input, output, metadata, lookup;
    uint32_t records[RECORD_COUNT];
    uint8_t bytes[BUFFER_BYTES];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result;
    uint8_t bytes[BUFFER_BYTES];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 40 + BUFFER_BYTES, "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 4 + BUFFER_BYTES, "probe output layout");
_Static_assert(sizeof(SequenceStreamLookupRecord) == 0x34, "lookup record layout");

int16_t g_sequence_stream_lookup_count;
SequenceStreamLookupRecord g_sequence_stream_lookup_records[RECORD_COUNT];

static ProbeOutput run_probe(const ProbeInput *input) {
    uint32_t memory[BUFFER_BYTES / 4];
    uint8_t *bytes = (uint8_t *)memory;
    ProbeOutput output = {0};
    unsigned index;
    assert(input->input <= BUFFER_BYTES - 512);
    assert(input->output % 4 == 0 && input->output <= BUFFER_BYTES - 1024);
    assert(input->metadata % 4 == 0 && input->metadata <= BUFFER_BYTES - 2048);
    assert(input->lookup % 4 == 0 && input->lookup <= BUFFER_BYTES - 512);
    memcpy(memory, input->bytes, sizeof(memory));
    g_sequence_stream_lookup_count = (int16_t)input->count;
    for (index = 0; index < RECORD_COUNT; ++index) {
        g_sequence_stream_lookup_records[index].value_10 = input->records[index];
    }
    output.result = (uint32_t)func_800102BC(
        bytes + input->output, bytes + input->input, bytes + input->metadata,
        (const uint32_t *)(bytes + input->lookup));
    memcpy(output.bytes, memory, sizeof(memory));
    return output;
}

static uint32_t word(const ProbeOutput *output, unsigned offset) {
    return (uint32_t)output->bytes[offset]
         | ((uint32_t)output->bytes[offset + 1] << 8)
         | ((uint32_t)output->bytes[offset + 2] << 16)
         | ((uint32_t)output->bytes[offset + 3] << 24);
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;
    unsigned index;
    input.count = 2;
    input.input = 0;
    input.lookup = 512;
    input.output = 1024;
    input.metadata = 2048;
    input.records[0] = 0x11234567;
    for (index = 0; index < 128; ++index) {
        input.bytes[index * 4] = 0x80;
        input.bytes[index * 4 + 1] = 0xFF;
        input.bytes[index * 4 + 2] = 0x7F;
        input.bytes[512 + index * 4] = 0x67;
        input.bytes[512 + index * 4 + 1] = 0x45;
        input.bytes[512 + index * 4 + 2] = 0x23;
        input.bytes[512 + index * 4 + 3] = 0x13;
    }
    output = run_probe(&input);
    assert(output.result == 0);
    assert(word(&output, 1024) == 0x0FE0F000u);
    assert(word(&output, 1028) == 0xFFFFFFE0u);
    assert(word(&output, 2052) == 0x801D7A34u);
    input.count = 1;
    output = run_probe(&input);
    assert(word(&output, 2052) == 0x11234567u);
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;
    if (argc == 1) {
        self_test();
        puts("stream-transfer probe passed");
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
            fputs("incomplete stream-transfer probe input\n", stderr);
            return 2;
        }
        output = run_probe(&input);
        if (fwrite(&output, 1, sizeof(output), stdout) != sizeof(output)) {
            perror("write stream-transfer probe output");
            return 2;
        }
    }
    if (ferror(stdin)) {
        perror("read stream-transfer probe input");
        return 2;
    }
    return 0;
}
