#include "cycle_movement_setup.h"
#include "queue_update.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

typedef struct Input {
    uint32_t enabled, permit, active, mode, duration;
    uint32_t current[3], destination[3];
    uint32_t mutations;
    uint32_t use_target, target_position[3], override_position[3];
} Input;

typedef struct Output {
    uint32_t step[3], adjustment[3], duration, elapsed;
    uint32_t active, mode, permit, mode_copy, has_callback, calls, target_position[3];
} Output;

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

uint8_t g_unknown_800BE024, g_unknown_800BE020, g_unknown_800BE022;
uint8_t g_unknown_80186108, g_unknown_80186109;
uint32_t g_unknown_80186110, g_unknown_80186114;
uint32_t g_unknown_80186118[3], g_unknown_80186148[3];
void (*g_unknown_80186144)(void);
static Input current_input;
static uint32_t calls;
static int32_t *saved_current;
static GroundedTransformVector override_position;
static int target_component;
void *g_unknown_800BE528 = &target_component;

static void callback(void) {}

int32_t func_800516F8(void **output) {
    calls = calls * 10 + 1;
    *output = &current_input;
    return 0;
}

int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t *output
) {
    if (basis != 0) abort();
    if ((const void *)component == &target_component) {
        calls = calls * 10 + 3;
        memcpy(output, current_input.target_position, sizeof(current_input.target_position));
        return 0;
    }
    if ((const void *)component != &current_input) abort();
    calls = calls * 10 + 2;
    saved_current = output;
    memcpy(output, current_input.current, sizeof(current_input.current));
    return 0;
}

static void mutate_current(void) {
    uint32_t word;
    if (current_input.mutations & 1) {
        memcpy(&word, saved_current, sizeof(word));
        word ^= 0x80000000u;
        memcpy(saved_current, &word, sizeof(word));
    }
    if (current_input.mutations & 2) {
        memcpy(&word, &override_position.values[1], sizeof(word));
        word ^= 0x40000000u;
        memcpy(&override_position.values[1], &word, sizeof(word));
    }
}

#ifndef CYCLE_REAL_TARGET
void func_8002E858(
    const GroundedTransformVector *direction, int32_t distance,
    uint32_t height, GroundedTransformVector *output
) {
    if (direction->coordinates.x != 0x1234 || distance != 321 || height != 654) abort();
    calls = calls * 10 + 3;
    mutate_current();
    memcpy(output->values, current_input.destination, sizeof(current_input.destination));
}
#else
int32_t func_80053D00(const int32_t *direction, int32_t distance, int32_t *output) {
    if ((uint32_t)direction[0] != 0x1234 || distance != 321) abort();
    calls = calls * 10 + 4;
    mutate_current();
    memcpy(output, current_input.destination, sizeof(current_input.destination));
    return 0;
}

int32_t func_800550A4(ComponentRotationMatrix *component, const ComponentRotationMatrix *basis) {
    (void)component;
    (void)basis;
    abort();
}
#endif

int32_t func_80054C7C(
    ComponentTransformView *component, const ComponentBasisView *basis, const int32_t *input
) {
    if ((void *)component != &target_component || basis != 0) abort();
    calls = calls * 10 + 5;
    memcpy(current_input.target_position, input, sizeof(current_input.target_position));
    return 0;
}

static Output run(const Input *input) {
    GroundedTransformVector direction = {.coordinates = {0x1234, 0x5678, 0x9abc}};
    Output result;
    int32_t duration;
    unsigned i;

    current_input = *input;
    calls = 0;
    g_unknown_800BE024 = (uint8_t)input->enabled;
    g_unknown_800BE020 = (uint8_t)input->permit;
    g_unknown_800BE022 = 9;
    g_unknown_80186108 = (uint8_t)input->active;
    g_unknown_80186109 = (uint8_t)input->mode;
    g_unknown_80186110 = 0x11111111;
    g_unknown_80186114 = 0x22222222;
    g_unknown_80186144 = 0;
    for (i = 0; i < 3; i++) {
        g_unknown_80186118[i] = 0x33333333 + i;
        g_unknown_80186148[i] = 0x44444444 + i;
    }
    memcpy(&duration, &input->duration, sizeof(duration));
    memcpy(override_position.values, input->override_position, sizeof(input->override_position));
    cycle_movement_setup_with_target(
        &direction, 321, 654, duration, callback, input->use_target ? &override_position : 0
    );
    memcpy(result.step, g_unknown_80186118, sizeof(result.step));
    memcpy(result.adjustment, g_unknown_80186148, sizeof(result.adjustment));
    result.duration = g_unknown_80186110;
    result.elapsed = g_unknown_80186114;
    result.active = g_unknown_80186108;
    result.mode = g_unknown_80186109;
    result.permit = g_unknown_800BE020;
    result.mode_copy = g_unknown_800BE022;
    result.has_callback = g_unknown_80186144 == callback;
    result.calls = calls;
    memcpy(result.target_position, current_input.target_position, sizeof(result.target_position));
    return result;
}

int main(int argc, char **argv) {
    Input input;
    size_t bytes;

    if (argc == 1) {
        const Input sample = {1, 1, 0, 0, 3, {100, 200, 300}, {101, 190, 300}, 0,
                              0, {7, 8, 9}, {0, 0, 0}};
        const Input disabled = {0, 1, 0, 0, 3, {100, 200, 300}, {101, 190, 300}, 0,
                                0, {7, 8, 9}, {0, 0, 0}};
        const Input target = {1, 1, 0, 0, 3, {100, 200, 300}, {101, 190, 300}, 0,
                              1, {7, 8, 9}, {20, 9, 0}};
        Output result = run(&sample);
#ifdef CYCLE_REAL_TARGET
        if (result.step[0] != 2 || result.step[1] != 214 || result.step[2] != 3 ||
            result.calls != 1234) return 1;
#else
        if (result.step[0] != 1 || result.step[1] != (uint32_t)-3 || result.step[2] != 0 ||
            result.calls != 123) return 1;
#endif
        if (
            result.duration != 3 || result.elapsed != 0 || result.active != 1 || result.mode != 0 ||
            result.has_callback != 1) return 1;
        result = run(&disabled);
        if (result.step[0] != 0x33333333 || result.duration != 0x11111111 || result.calls != 0) return 1;
        result = run(&target);
        if (result.adjustment[0] != 4 || result.adjustment[1] != 1 ||
            result.adjustment[2] != (uint32_t)-3 || result.target_position[0] != 7 ||
            result.target_position[1] != 8 || result.target_position[2] != 9) return 1;
        puts("cycle movement setup tests passed");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) return 2;
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) return 2;
#endif
    while ((bytes = fread(&input, 1, sizeof(input), stdin)) != 0) {
        if (bytes != sizeof(input)) {
            fputs("incomplete probe input\n", stderr);
            return 2;
        }
        Output result = run(&input);
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
