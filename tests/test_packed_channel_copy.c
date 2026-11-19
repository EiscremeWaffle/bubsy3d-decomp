#include "packed_channel_copy.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

enum { BUFFER_SIZE = 192 };

typedef union AlignedBuffer {
    uint32_t words[BUFFER_SIZE / sizeof(uint32_t)];
    uint8_t bytes[BUFFER_SIZE];
} AlignedBuffer;

typedef struct ProbeInput {
    uint32_t kind, count, input_offset, output_offset;
    uint8_t bytes[BUFFER_SIZE];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result;
    uint8_t bytes[BUFFER_SIZE];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 16 + BUFFER_SIZE, "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 4 + BUFFER_SIZE, "probe output layout");

static ProbeOutput run_probe(const ProbeInput *input) {
    AlignedBuffer buffer;
    ProbeOutput output;

    assert(input->kind <= 1 && input->count > 0);
    assert(input->output_offset % 4 == 0);
    assert(input->kind != 0 || input->input_offset % 2 == 0);
    assert(input->input_offset + input->count * 4 <= BUFFER_SIZE);
    assert(input->output_offset + input->count * 8 <= BUFFER_SIZE);
    memcpy(buffer.bytes, input->bytes, BUFFER_SIZE);
    if (input->kind == 0) {
        output.result = (uint32_t)func_80010244(
            buffer.bytes + input->output_offset, buffer.bytes + input->input_offset, input->count);
    } else {
        output.result = (uint32_t)func_80010278(
            buffer.bytes + input->output_offset, buffer.bytes + input->input_offset, input->count);
    }
    memcpy(output.bytes, buffer.bytes, BUFFER_SIZE);
    return output;
}

static int32_t signed_byte(uint8_t value) {
    return value < 0x80 ? value : (int32_t)value - 0x100;
}

static void reference_copy(ProbeInput *input) {
    uint32_t index;
    for (index = 0; index < input->count; ++index) {
        uint8_t *source = input->bytes + input->input_offset + index * 4;
        uint8_t *destination = input->bytes + input->output_offset + index * 8;
        int32_t first = signed_byte(source[0]);
        int32_t second = signed_byte(source[1]);
        int32_t third;
        uint32_t first_bits, third_bits, second_bits;
        if (input->kind == 0) {
            uint32_t value = (uint32_t)source[2] | ((uint32_t)source[3] << 8);
            third = value < 0x8000 ? (int32_t)value : (int32_t)value - 0x10000;
        } else {
            third = signed_byte(source[2]) * 32;
            first *= 32;
            second *= 32;
        }
        first_bits = (uint32_t)first;
        third_bits = (uint32_t)third;
        second_bits = (uint32_t)second;
        destination[0] = (uint8_t)first_bits;
        destination[1] = (uint8_t)(first_bits >> 8);
        destination[2] = (uint8_t)third_bits;
        destination[3] = (uint8_t)(third_bits >> 8);
        destination[4] = (uint8_t)second_bits;
        destination[5] = (uint8_t)(second_bits >> 8);
        destination[6] = (uint8_t)(second_bits >> 16);
        destination[7] = (uint8_t)(second_bits >> 24);
    }
}

static void self_test(void) {
    ProbeInput input;
    uint32_t kind, count, input_offset, output_offset, index;

    for (kind = 0; kind < 2; ++kind) {
        for (count = 1; count <= 8; ++count) {
            for (input_offset = 0; input_offset <= 40; ++input_offset) {
                if (kind == 0 && input_offset % 2 != 0) {
                    continue;
                }
                for (output_offset = 0; output_offset <= 40; output_offset += 4) {
                    ProbeInput expected;
                    ProbeOutput output;
                    input.kind = kind;
                    input.count = count;
                    input.input_offset = input_offset;
                    input.output_offset = output_offset;
                    for (index = 0; index < BUFFER_SIZE; ++index) {
                        input.bytes[index] = (uint8_t)(index * 73 + count * 19 + input_offset);
                    }
                    expected = input;
                    reference_copy(&expected);
                    output = run_probe(&input);
                    assert(output.result == 0);
                    assert(memcmp(output.bytes, expected.bytes, BUFFER_SIZE) == 0);
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("packed channel expansion tests passed");
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
            fputs("incomplete packed-channel probe input\n", stderr);
            return 2;
        }
        output = run_probe(&input);
        if (fwrite(&output, 1, sizeof(output), stdout) != sizeof(output)) {
            perror("write packed-channel probe output");
            return 2;
        }
    }
    if (ferror(stdin)) {
        perror("read packed-channel probe input");
        return 2;
    }
    return 0;
}
