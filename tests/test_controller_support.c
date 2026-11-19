#include "controller_support.h"

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
    uint32_t operation, null_position, selection, origin_x, origin_y, x, y, marker;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, marker, x, y, origin_x, origin_y;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 8 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 6 * sizeof(uint32_t), "probe output layout");

ControllerSelectionSource *g_controller_selection_source;
ControllerScreenOrigin g_controller_screen_origin;

static ProbeOutput run_probe(const ProbeInput *input) {
    ControllerSelectionSource source = {{0}, input->selection};
    ControllerScreenPosition position;
    ProbeOutput output;

    memcpy(position.unknown_00, &input->marker, sizeof(input->marker));
    position.x_04 = (uint16_t)input->marker;
    position.y_06 = (uint16_t)(input->marker >> 16);
    g_controller_selection_source = &source;
    g_controller_screen_origin.x = (uint16_t)input->origin_x;
    g_controller_screen_origin.y = (uint16_t)input->origin_y;
    if (input->operation == 0) {
        output.result = func_8004753C();
    } else {
        assert(input->operation == 1);
        output.result = (uint32_t)func_80055D94(
            input->null_position ? 0 : &position, input->x, input->y);
    }
    memcpy(&output.marker, position.unknown_00, sizeof(output.marker));
    output.x = position.x_04;
    output.y = position.y_06;
    output.origin_x = g_controller_screen_origin.x;
    output.origin_y = g_controller_screen_origin.y;
    return output;
}

static void self_test(void) {
    ProbeInput input = {0, 0, 0xFFFFFFFFU, 5, 0xFFFF, 0, 0, 0x12345678};
    ProbeOutput output = run_probe(&input);

    assert(output.result == 0xFFFFFFFFU);
    assert(output.marker == input.marker && output.x == 0x5678 && output.y == 0x1234);

    input.operation = 1;
    output = run_probe(&input);
    assert(output.result == 0 && output.x == 0xFFFB && output.y == 1);
    assert(output.marker == input.marker && output.origin_x == 5 && output.origin_y == 0xFFFF);

    input.null_position = 1;
    output = run_probe(&input);
    assert(output.result == 22 && output.x == 0x5678 && output.y == 0x1234);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("controller support tests passed");
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
