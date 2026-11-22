#include "secondary_controller_rotation.h"
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

typedef struct ProbeInput {
    uint32_t position[3], direction[3];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t update, position[3];
} ProbeOutput;

uint32_t func_80056678(uint32_t value, uint32_t selector) {
    (void)value;
    (void)selector;
    assert(0);
    return 0;
}
int32_t func_80048FA8(ComponentRotationMatrix *component, const int32_t rotation[3]) {
    (void)component;
    (void)rotation;
    assert(0);
    return 0;
}
int32_t func_80049080(ComponentRotationMatrix *component, const int32_t rotation[3]) {
    (void)component;
    (void)rotation;
    assert(0);
    return 0;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeOutput output;
    int32_t position[3], direction[3];
    memcpy(position, input->position, sizeof(position));
    memcpy(direction, input->direction, sizeof(direction));
    output.update = (uint32_t)secondary_controller_rotation_position(position, direction);
    memcpy(output.position, position, sizeof(position));
    return output;
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
    if (argc != 1) {
        fputs("expected --binary or no arguments\n", stderr);
        return 2;
    }
    {
        static const struct {
            ProbeInput input;
            ProbeOutput expected;
        } cases[] = {
            {{{0, 100, 0}, {0, (uint32_t)-925, 0}}, {1, {0, (uint32_t)-825, 0}}},
            {{{0, 320, 0}, {0, (uint32_t)-925, 0}}, {0, {0, (uint32_t)-605, 0}}},
            {{{0, 319, 0}, {0, (uint32_t)-924, 0}}, {0, {0, (uint32_t)-605, 0}}},
            {{{0, 100, 0}, {0, 951, 0}}, {0, {0, 1050, 0}}},
            {{{0x7fffffffu, 319, 0x80000000u}, {1, (uint32_t)-926, (uint32_t)-1}},
             {1, {0x80000000u, (uint32_t)-606, 0x7fffffffu}}},
        };
        size_t index;
        for (index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            ProbeOutput output = run_probe(&cases[index].input);
            assert(output.update == cases[index].expected.update);
            assert(memcmp(output.position, cases[index].expected.position, sizeof(output.position)) == 0);
        }
    }
    puts("secondary rotation position tests passed");
    return 0;
}
