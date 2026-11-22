#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

extern int32_t func_80053B70(const int32_t *input, int32_t scale, int32_t *output);
extern int32_t func_80053D00(const int32_t *input, int32_t scale, int32_t *output);

typedef struct ProbeInput {
    uint32_t mode, alias, mutation, input[4], output[4], scale, length;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, count, trace[20], input[4], output[4];
} ProbeOutput;

static ProbeInput current;
static ProbeOutput result;
static int32_t input_vector[4], output_vector[4], *length_pointer;
static unsigned divide_calls;

static void record(uint32_t value) {
    assert(result.count < 20);
    result.trace[result.count++] = value;
}

static uint32_t read_bits(const int32_t *input) {
    uint32_t value;
    memcpy(&value, input, sizeof(value));
    return value;
}

static void write_bits(int32_t *output, uint32_t value) {
    memcpy(output, &value, sizeof(value));
}

int32_t func_8005385C(const int32_t *input, int32_t *output) {
    record(1);
    record(read_bits(input));
    record(read_bits(input + 1));
    record(read_bits(input + 2));
    write_bits(output, current.length);
    length_pointer = output;
    if (current.mutation & 1u) write_bits(input_vector + 1, read_bits(input_vector + 1) ^ 0x12345678u);
    return -1;
}

int32_t func_80053A08(const int32_t *input, int32_t scale, int32_t *output) {
    uint32_t values[3];
    unsigned index;
    for (index = 0; index < 3; index++) values[index] = read_bits(input + index);
    record(2);
    record((uint32_t)scale);
    for (index = 0; index < 3; index++) record(values[index]);
    for (index = 0; index < 3; index++) write_bits(output + index, values[index] ^ (uint32_t)scale);
    if (current.mutation & 2u) write_bits(length_pointer, read_bits(length_pointer) ^ 0x87654321u);
    return -2;
}

int32_t func_80053350(int32_t value, int32_t scale, int32_t *output) {
    record(3);
    record((uint32_t)value);
    record((uint32_t)scale);
    write_bits(output, (uint32_t)value + (uint32_t)scale);
    if (current.mode == 0 && (current.mutation & 4u) && divide_calls == 0) {
        write_bits(input_vector + 1, read_bits(input_vector + 1) ^ 0xabcdef01u);
    }
    divide_calls++;
    return -3;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t *target;
    current = *input;
    memset(&result, 0, sizeof(result));
    memcpy(input_vector, input->input, sizeof(input_vector));
    memcpy(output_vector, input->output, sizeof(output_vector));
    divide_calls = 0;
    target = input->alias ? input_vector : output_vector;
    if (input->mode == 0) {
        result.status = (uint32_t)func_80053B70(input_vector, (int32_t)input->scale, target);
    } else {
        assert(input->mode == 1);
        result.status = (uint32_t)func_80053D00(input_vector, (int32_t)input->scale, target);
    }
    memcpy(result.input, input_vector, sizeof(result.input));
    memcpy(result.output, output_vector, sizeof(result.output));
    return result;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--binary") == 0) {
        ProbeInput input;
        size_t bytes_read;
#ifdef _WIN32
        if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
            perror("set binary mode");
            return 2;
        }
#endif
        while ((bytes_read = fread(&input, 1, sizeof(input), stdin)) != 0) {
            ProbeOutput output;
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
            perror("read probe input");
            return 2;
        }
        return 0;
    }
    if (argc != 1) return 2;
    {
        ProbeInput input = {0};
        ProbeOutput actual;
        input.input[0] = 0xffffffffu;
        input.input[1] = 2;
        input.input[2] = 3;
        input.output[3] = 0x12345678u;
        input.scale = 1;
        input.mutation = 4;
        actual = run_probe(&input);
        assert(actual.status == 0 && actual.count == 9);
        assert(actual.output[0] == 0 && actual.output[1] == ((2u ^ 0xabcdef01u) + 1u));
        assert(actual.output[2] == 4 && actual.output[3] == 0x12345678u);
        input.mode = 1;
        input.mutation = 3;
        input.length = 5;
        actual = run_probe(&input);
        assert(actual.status == 0 && actual.count == 18);
        assert(actual.trace[11] == (5u ^ 0x87654321u));
        assert(actual.output[0] == ((0xffffffffu ^ 1u) + (5u ^ 0x87654321u)));
    }
    puts("grounded vector scale tests passed");
    return 0;
}
