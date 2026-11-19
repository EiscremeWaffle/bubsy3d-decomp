#include "gte_background_color.h"
#include "mode_camera_helpers.h"

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
    uint32_t pipeline, values[3];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t controls[3];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 4 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 3 * sizeof(uint32_t), "probe output layout");

uint8_t g_unknown_800BE5A4[3];

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t values[3];
    ProbeOutput output;

    memcpy(values, input->values, sizeof(values));
    if (input->pipeline != 0) {
        func_80072DF0(values[0], values[1], values[2]);
    } else {
        func_80077874(values[0], values[1], values[2]);
    }
    memcpy(output.controls, g_gte_background_color, sizeof(output.controls));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0, {0, 0xFFFFFFFFU, 0x80000000U}};
    uint32_t random = 0x77874001U;
    unsigned index, component;

    for (index = 0; index < 50000; ++index) {
        ProbeOutput output;
        input.pipeline = index & 1U;
        if (index != 0) {
            for (component = 0; component < 3; ++component) {
                random = random * 1664525U + 1013904223U;
                input.values[component] = random;
            }
        }
        output = run_probe(&input);
        for (component = 0; component < 3; ++component) {
            uint32_t expected = input.pipeline ? input.values[component] & 0xFFFFFFF0U
                                               : input.values[component] << 4;
            assert(output.controls[component] == expected);
        }
    }

    assert(func_8004EF10(0, 0x10, 0xFFF) == 0);
    assert(g_gte_background_color[0] == 0 && g_gte_background_color[1] == 0x10);
    assert(g_gte_background_color[2] == 0xFF0);
    assert(g_unknown_800BE5A4[0] == 0 && g_unknown_800BE5A4[1] == 1);
    assert(g_unknown_800BE5A4[2] == 0xFF);
    assert(func_8004EF10(-1, 0x10, 0xFFF) == 1);
    assert(g_gte_background_color[2] == 0xFF0 && g_unknown_800BE5A4[2] == 0xFF);
    assert(func_8004EF10(0, 0x1000, 0xFFF) == 1);
    assert(func_8004EF10(0, 0x10, 0x1000) == 1);
    assert(g_gte_background_color[2] == 0xFF0 && g_unknown_800BE5A4[2] == 0xFF);
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("camera GTE background-color tests passed");
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
