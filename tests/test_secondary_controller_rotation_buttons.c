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
    uint32_t buttons, direction_y, rotation[3], basis[8];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t call_count, calls[8], rotation_count, rotations[12], rotation[3], basis[8];
} ProbeOutput;

static ProbeOutput output;

uint32_t func_80056678(uint32_t buttons, uint32_t selector) {
    assert(output.call_count < 8);
    output.calls[output.call_count++] = selector;
    return (buttons >> selector) & 1;
}

static int32_t rotate(ComponentRotationMatrix *component, const int32_t rotation[3], uint32_t call) {
    uint32_t first;
    assert(output.call_count < 8 && output.rotation_count < 4);
    output.calls[output.call_count++] = call;
    memcpy(output.rotations + 3 * output.rotation_count, rotation, 3 * sizeof(int32_t));
    ++output.rotation_count;
    memcpy(&first, component, sizeof(first));
    first ^= call;
    memcpy(component, &first, sizeof(first));
    return -1;
}

int32_t func_80048FA8(ComponentRotationMatrix *component, const int32_t rotation[3]) {
    return rotate(component, rotation, 0x100);
}
int32_t func_80049080(ComponentRotationMatrix *component, const int32_t rotation[3]) {
    return rotate(component, rotation, 0x101);
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ComponentRotationMatrix basis;
    int32_t direction_y, rotation[3];
    memset(&output, 0, sizeof(output));
    memcpy(&basis, input->basis, sizeof(basis));
    memcpy(&direction_y, &input->direction_y, sizeof(direction_y));
    memcpy(rotation, input->rotation, sizeof(rotation));
    secondary_controller_rotation_buttons(input->buttons, direction_y, &basis, rotation);
    memcpy(output.rotation, rotation, sizeof(rotation));
    memcpy(output.basis, &basis, sizeof(basis));
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
            ProbeOutput result;
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
        ProbeInput input = {0};
        ProbeOutput result;
        input.buttons = 1u << 13;
        input.direction_y = 360;
        result = run_probe(&input);
        assert(result.rotation_count == 1 && result.rotation[1] == 12);
    }
    puts("secondary rotation button tests passed");
    return 0;
}
