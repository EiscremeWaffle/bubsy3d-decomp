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
    uint32_t first, second, current[3], restored[3], mutation;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t count, calls[10], snapshots[9], camera_state[6];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 9 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 26 * sizeof(uint32_t), "probe output layout");

static ProbeInput current;
static ProbeOutput output;

static void record(uint32_t call) {
    assert(output.count < 10);
    output.calls[output.count++] = call;
}

static void capture(unsigned slot, const uint8_t *rotation) {
    unsigned index;
    for (index = 0; index < 3; index++) output.snapshots[slot * 3 + index] = rotation[index];
}

int32_t func_8005F374(void) { record(1); return -1; }
int32_t func_8004EF10(int32_t first, int32_t second, int32_t third) {
    record(2);
    assert(first == 0 && second == 0 && third == 0);
    return -1;
}
int32_t func_8004EFD4(void) { record(3); return -1; }
void func_80028E50(void) { record(4); }
int32_t func_8004EE60(uint32_t camera, uint8_t *rotation) {
    unsigned index;
    record(5);
    assert(camera == 0);
    for (index = 0; index < 3; index++) rotation[index] = (uint8_t)current.current[index];
    return -1;
}
#ifndef REAL_CAMERA
int32_t func_8004ED78(uint32_t camera, const uint8_t *rotation) {
    uint8_t *mutable_rotation = (uint8_t *)rotation;
    unsigned index;
    assert(camera < 2);
    record(6 + camera);
    capture(camera, rotation);
    if (current.mutation & (1U << camera)) {
        for (index = 0; index < 3; index++) mutable_rotation[index] ^= (uint8_t)(0x51U + camera + index);
    }
    return -1;
}
#else
CameraTransformRecord g_camera_transforms[3];

int32_t func_80072798(uint32_t camera, CameraTransformRecord *recorded) {
    unsigned index;
    assert(camera < 2 && recorded == &g_camera_transforms[camera]);
    record(6 + camera);
    capture(camera, recorded->rotation);
    if (current.mutation & (1U << camera)) {
        for (index = 0; index < 3; index++) recorded->rotation[index] ^= (uint8_t)(0x51U + camera + index);
    }
    return -1;
}
#endif
int32_t func_80050824(const uint8_t *rotation) { record(8); capture(2, rotation); return -1; }
int32_t func_8005F398(void) { record(9); return -1; }
void func_80028EE0(void) { record(10); }
void func_80028EA8(void) { record(11); }
int32_t func_8004EF98(void) { record(12); return -1; }
void func_80028E20(void) { record(13); }
void func_800280D0(uint8_t *rotation) {
    unsigned index;
    record(14);
    for (index = 0; index < 3; index++) rotation[index] = (uint8_t)current.restored[index];
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t first, second;
    current = *input;
    memset(&output, 0xFF, sizeof(output));
    output.count = 0;
#ifdef REAL_CAMERA
    memset(g_camera_transforms, 0xA5, sizeof(g_camera_transforms));
#endif
    memcpy(&first, &input->first, sizeof(first));
    memcpy(&second, &input->second, sizeof(second));
    func_80027114(first, second);
#ifdef REAL_CAMERA
    {
        unsigned camera, index;
        for (camera = 0; camera < 2; camera++) {
            for (index = 0; index < 3; index++) {
                output.camera_state[camera * 3 + index] = g_camera_transforms[camera].rotation[index];
            }
        }
    }
#endif
    return output;
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t read_count;
    if (argc == 1) {
        memset(&input, 0, sizeof(input));
        input.first = input.second = 1;
        input.current[0] = 255;
        input.current[1] = 8;
        input.current[2] = 7;
        input.mutation = 1;
        result = run_probe(&input);
        assert(result.count == 8 && result.snapshots[0] == 224);
        assert(result.snapshots[1] == 7 && result.snapshots[2] == 7);
#ifdef REAL_CAMERA
        assert(result.snapshots[3] == 224 && result.camera_state[0] == (224 ^ 0x51));
#else
        assert(result.snapshots[3] == (224 ^ 0x51));
#endif
        assert(result.snapshots[6] == 0 && result.snapshots[7] == 0 && result.snapshots[8] == 0);
        puts("controller camera transition tests passed");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) return 2;
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
        perror("set binary mode");
        return 2;
    }
#endif
    while ((read_count = fread(&input, 1, sizeof(input), stdin)) != 0) {
        if (read_count != sizeof(input)) {
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
        fputs("failed to read probe input\n", stderr);
        return 2;
    }
    return 0;
}
