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
    uint32_t enabled, permit, active, mode, duration, attempts;
    uint32_t first[3], second[3], destination[3];
    uint32_t use_target, target[3];
} Input;

typedef struct Output {
    uint32_t first[3], second[3], duration, elapsed, active, mode;
    uint32_t step[3], adjustment[3], returns, callbacks;
} Output;

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

uint8_t g_unknown_800BE024, g_unknown_800BE020, g_unknown_800BE022;
uint8_t g_unknown_80186108, g_unknown_80186109;
uint32_t g_unknown_80186110, g_unknown_80186114;
uint32_t g_unknown_80186118[3], g_unknown_80186148[3];
void (*g_unknown_80186144)(void);
static int first_component, second_component;
void *g_unknown_800BE528 = &second_component;
static uint32_t first_position[3], second_position[3], destination[3], callbacks;

static void completion(void) { callbacks++; }

int32_t func_800516F8(void **output) {
    *output = &first_component;
    return 0;
}

int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t *output
) {
    if (basis != NULL) abort();
    if ((const void *)component == &first_component) memcpy(output, first_position, sizeof(first_position));
    else if ((const void *)component == &second_component) memcpy(output, second_position, sizeof(second_position));
    else abort();
    return 0;
}

int32_t func_80054AF0(const int16_t *component, int32_t *output) {
    static const int32_t direction[] = {7, 8, 9};
    if ((const void *)component != &first_component) abort();
    memcpy(output, direction, sizeof(direction));
    return 0;
}

int32_t func_80054C7C(
    ComponentTransformView *component, const ComponentBasisView *basis, const int32_t *input
) {
    if (basis != NULL) abort();
    if ((void *)component == &first_component) memcpy(first_position, input, sizeof(first_position));
    else if ((void *)component == &second_component) memcpy(second_position, input, sizeof(second_position));
    else abort();
    return 0;
}

int32_t func_800550A4(ComponentRotationMatrix *component, const ComponentRotationMatrix *basis) {
    if ((void *)component != &first_component || (const void *)basis != &second_component) abort();
    return 0;
}

void func_8002E858(
    const GroundedTransformVector *direction, int32_t distance,
    uint32_t height, GroundedTransformVector *output
) {
    if (direction->coordinates.x != 0x1234 || distance != 321 || height != 654) abort();
    memcpy(output->values, destination, sizeof(destination));
}

static Output run(const Input *input) {
    GroundedTransformVector direction = {.coordinates = {0x1234, 0x5678, 0x9abc}};
    GroundedTransformVector target;
    Output result;
    unsigned i;
    int32_t duration;

    memset(&result, 0, sizeof(result));
    memcpy(first_position, input->first, sizeof(first_position));
    memcpy(second_position, input->second, sizeof(second_position));
    memcpy(destination, input->destination, sizeof(destination));
    memcpy(target.values, input->target, sizeof(input->target));
    callbacks = 0;
    g_unknown_800BE024 = (uint8_t)input->enabled;
    g_unknown_800BE020 = (uint8_t)input->permit;
    g_unknown_800BE022 = 9;
    g_unknown_80186108 = (uint8_t)input->active;
    g_unknown_80186109 = (uint8_t)input->mode;
    g_unknown_80186110 = 0x11111111;
    g_unknown_80186114 = 0x22222222;
    g_unknown_80186144 = NULL;
    for (i = 0; i < 3; i++) {
        g_unknown_80186118[i] = 0x33333333 + i;
        g_unknown_80186148[i] = 0x44444444 + i;
    }
    memcpy(&duration, &input->duration, sizeof(duration));
    cycle_movement_setup_with_target(
        &direction, 321, 654, duration, completion, input->use_target ? &target : NULL
    );
    for (i = 0; i < input->attempts; i++) {
        result.returns |= (uint32_t)func_80032A2C(42) << i;
    }
    memcpy(result.first, first_position, sizeof(result.first));
    memcpy(result.second, second_position, sizeof(result.second));
    result.duration = g_unknown_80186110;
    result.elapsed = g_unknown_80186114;
    result.active = g_unknown_80186108;
    result.mode = g_unknown_80186109;
    memcpy(result.step, g_unknown_80186118, sizeof(result.step));
    memcpy(result.adjustment, g_unknown_80186148, sizeof(result.adjustment));
    result.callbacks = callbacks;
    return result;
}

int main(int argc, char **argv) {
    Input input;
    size_t bytes;
    if (argc == 1) {
        const Input sample = {1, 1, 0, 0, 2, 4, {0, 0, 0}, {1, 2, 3}, {10, 4, 0},
                              0, {0, 0, 0}};
        const Input target = {1, 1, 0, 0, 2, 4, {0, 0, 0}, {1, 2, 3}, {10, 4, 0},
                              1, {11, 8, (uint32_t)-1}};
        Output result = run(&sample);
        if (result.returns != 3 || result.callbacks != 1 || result.first[0] != 10 ||
            result.first[1] != 4 || result.active != 0 || result.mode != 1) return 1;
        result = run(&target);
        if (result.returns != 3 || result.callbacks != 1 || result.second[0] != 11 ||
            result.second[1] != 8 || result.second[2] != (uint32_t)-1) return 1;
        puts("cycle movement flow tests passed");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) return 2;
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) return 2;
#endif
    while ((bytes = fread(&input, 1, sizeof(input), stdin)) != 0) {
        Output result;
        if (bytes != sizeof(input) || input.attempts > 31) {
            fputs("invalid probe input\n", stderr);
            return 2;
        }
        result = run(&input);
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
