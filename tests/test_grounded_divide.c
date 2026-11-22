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

extern int32_t func_80053350(int32_t value, int32_t scale, int32_t *output);
extern int32_t func_80053B70(const int32_t *input, int32_t scale, int32_t *output);

typedef struct ProbeInput {
    uint32_t mode, alias, scale, values[3], output[3];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, values[3], output[3];
} ProbeOutput;

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t values[3], output[3], scale, *target;
    ProbeOutput result;
    memcpy(values, input->values, sizeof(values));
    memcpy(output, input->output, sizeof(output));
    memcpy(&scale, &input->scale, sizeof(scale));
    target = input->alias ? values : output;
    if (input->mode == 0) {
        result.status = (uint32_t)func_80053350(values[0], scale, target);
    } else {
        assert(input->mode == 1);
        result.status = (uint32_t)func_80053B70(values, scale, target);
    }
    memcpy(result.values, values, sizeof(values));
    memcpy(result.output, output, sizeof(output));
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
        static const struct {
            uint32_t value, scale, result, status;
        } boundaries[] = {
            {0x0007ffffu, 0x00080000u, 4095, 0},
            {0x00080000u, 0x00080001u, 4096, 0},
            {0x007fffffu, 0x00800000u, 4095, 0},
            {0x00800000u, 0x00800001u, 4096, 0},
            {0x07ffffffu, 0x08000000u, 4095, 0},
            {0x08000000u, 0x08000001u, 4096, 0},
            {0x0007ffffu, 1, 0x7ffff000u, 0},
            {0x00080000u, 1, 0x7fffffffu, 14},
            {0x00080000u, 3, 0x2aaaa000u, 0},
        };
        size_t index;
        for (index = 0; index < sizeof(boundaries) / sizeof(boundaries[0]); ++index) {
            input.values[0] = boundaries[index].value;
            input.scale = boundaries[index].scale;
            actual = run_probe(&input);
            assert(actual.status == boundaries[index].status && actual.output[0] == boundaries[index].result);
        }
        input.values[0] = 0x7fffffffu;
        input.scale = 0;
        input.output[0] = 0x12345678u;
        actual = run_probe(&input);
        assert(actual.status == 7 && actual.output[0] == 0x12345678u);
        input.scale = 1;
        actual = run_probe(&input);
        assert(actual.status == 14 && actual.output[0] == 0x7fffffffu);
        input.scale = (uint32_t)-1;
        actual = run_probe(&input);
        assert(actual.status == 14 && actual.output[0] == 0x80000001u);
        input.values[0] = 0x80000000u;
        actual = run_probe(&input);
        assert(actual.status == 0 && actual.output[0] == 0);
        input.mode = 1;
        input.alias = 1;
        input.scale = 0;
        input.values[0] = 3;
        input.values[1] = 4;
        input.values[2] = 5;
        actual = run_probe(&input);
        assert(actual.status == 0 && actual.values[0] == 3 && actual.values[1] == 4 && actual.values[2] == 5);
        input.scale = 2;
        actual = run_probe(&input);
        assert(actual.status == 0 && actual.values[0] == 6144 && actual.values[1] == 8192);
        assert(actual.values[2] == 10240);
    }
    puts("grounded fixed-point division tests passed");
    return 0;
}
