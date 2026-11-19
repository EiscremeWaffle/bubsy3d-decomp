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
    uint32_t operation, index, first, second, third, aliases, mutation;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, globals[3], outputs[3], positions[9], rotations[3], channels[3];
    uint32_t call_count, call_kind, call_first, call_second, call_third;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 7 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 27 * sizeof(uint32_t), "probe output layout");

int32_t g_unknown_801D89C0[3];
uint8_t g_unknown_800BE5A4[3];
CameraTransformRecord g_camera_transforms[3];
uint8_t g_camera_slot_status[3];
uint8_t g_camera_mode;

static uint32_t mutation;
static uint32_t call_count, call_kind, call_first, call_second, call_third;

int32_t func_80072798(uint32_t index, CameraTransformRecord *record) {
    assert(index < 3 && record == &g_camera_transforms[index]);
    ++call_count;
    call_kind = 1;
    call_first = index;
    call_second = (uint32_t)record->position[1];
    call_third = record->rotation[1];
    if (mutation != 0) {
        record->position[1] = -9;
        record->rotation[1] = 0xA5;
    }
    return -7;
}

#ifdef BUBSY3D_TEST_REAL_CAMERA_CHANNEL
void func_80077874(int32_t first, int32_t second, int32_t third) {
#else
void func_80072DF0(int32_t first, int32_t second, int32_t third) {
#endif
    ++call_count;
    call_kind = 2;
    call_first = (uint32_t)first;
    call_second = (uint32_t)second;
    call_third = (uint32_t)third;
    if (mutation != 0) {
        memset(g_unknown_800BE5A4, 0xEE, sizeof(g_unknown_800BE5A4));
    }
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeOutput output;
    int32_t locals[3] = {0x11111111, 0x22222222, 0x33333333};
    int32_t values[3];
    int32_t *pointers[6];
    unsigned index, component;

    memset(&output, 0, sizeof(output));
    memcpy(&g_unknown_801D89C0[0], &input->first, sizeof(input->first));
    memcpy(&g_unknown_801D89C0[1], &input->second, sizeof(input->second));
    memcpy(&g_unknown_801D89C0[2], &input->third, sizeof(input->third));
    memcpy(&values[0], &input->first, sizeof(input->first));
    memcpy(&values[1], &input->second, sizeof(input->second));
    memcpy(&values[2], &input->third, sizeof(input->third));
    for (index = 0; index < 3; ++index) {
        uint32_t rotation = 0x44332211U + index;
        for (component = 0; component < 3; ++component) {
            g_camera_transforms[index].position[component] = 0x1000 + (int32_t)(index * 0x10 + component);
        }
        g_camera_transforms[index].rotation[0] = (uint8_t)rotation;
        g_camera_transforms[index].rotation[1] = (uint8_t)(rotation >> 8);
        g_camera_transforms[index].rotation[2] = (uint8_t)(rotation >> 16);
        g_camera_transforms[index].padding = (uint8_t)(rotation >> 24);
        g_unknown_800BE5A4[index] = (uint8_t)(0x34 + index * 0x22);
        pointers[index] = &g_unknown_801D89C0[index];
        pointers[index + 3] = &locals[index];
    }
    mutation = input->mutation;
    call_count = call_kind = call_first = call_second = call_third = 0;
    output.result = 0xFFFFFFFFU;

    if (input->operation == 0) {
        uint32_t first = input->aliases & 7U;
        uint32_t second = (input->aliases >> 3) & 7U;
        uint32_t third = (input->aliases >> 6) & 7U;
        assert(first < 6 && second < 6 && third < 6);
        func_80028100(pointers[first], pointers[second], pointers[third]);
    } else if (input->operation == 1) {
        const int32_t *position = values;
        if (input->aliases != 0) {
            assert(input->aliases <= 3);
            position = g_camera_transforms[input->aliases - 1].position;
        }
        output.result = (uint32_t)func_8004EDEC(input->index, position);
    } else if (input->operation == 2) {
        output.result = (uint32_t)func_8004EF10(values[0], values[1], values[2]);
    } else {
        assert(input->operation == 3);
        func_80072DF0(values[0], values[1], values[2]);
    }

    memcpy(output.globals, g_unknown_801D89C0, sizeof(output.globals));
    memcpy(output.outputs, locals, sizeof(output.outputs));
    for (index = 0; index < 3; ++index) {
        for (component = 0; component < 3; ++component) {
            output.positions[index * 3 + component] = (uint32_t)g_camera_transforms[index].position[component];
        }
        output.rotations[index] = (uint32_t)g_camera_transforms[index].rotation[0] |
                                  ((uint32_t)g_camera_transforms[index].rotation[1] << 8) |
                                  ((uint32_t)g_camera_transforms[index].rotation[2] << 16) |
                                  ((uint32_t)g_camera_transforms[index].padding << 24);
        output.channels[index] = g_unknown_800BE5A4[index];
    }
    output.call_count = call_count;
    output.call_kind = call_kind;
    output.call_first = call_first;
    output.call_second = call_second;
    output.call_third = call_third;
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.first = 10;
    input.second = 20;
    input.third = 30;
    input.aliases = 1U | (2U << 3) | (3U << 6);
    output = run_probe(&input);
    assert(output.globals[1] == 10 && output.globals[2] == 10 && output.outputs[0] == 10);

    input.operation = 1;
    input.index = 2;
    input.aliases = 0;
    input.mutation = 1;
    output = run_probe(&input);
    assert(output.result == 0 && output.call_count == 1 && output.positions[6] == 10);
    assert(output.positions[7] == 0xFFFFFFF7U && output.rotations[2] == 0x4433A513U);
    input.index = 3;
    output = run_probe(&input);
    assert(output.result == 1 && output.call_count == 0);

    input.operation = 2;
    input.first = 0;
    input.second = 0x10;
    input.third = 0xFFF;
    output = run_probe(&input);
    assert(output.result == 0 && output.channels[0] == 0 && output.channels[1] == 1);
    assert(output.channels[2] == 0xFF && output.call_count == 1);
    input.third = 0x1000;
    output = run_probe(&input);
    assert(output.result == 1 && output.call_count == 0 && output.channels[2] == 0x78);
#ifdef BUBSY3D_TEST_REAL_CAMERA_CHANNEL
    input.operation = 3;
    input.first = 0x80000000U;
    input.second = 0xFFFFFFFFU;
    input.third = 0x7FFFFFFFU;
    output = run_probe(&input);
    assert(output.call_first == 0xF8000000U && output.call_second == 0xFFFFFFFFU);
    assert(output.call_third == 0x07FFFFFFU);
#endif
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("mode-camera helper tests passed");
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
