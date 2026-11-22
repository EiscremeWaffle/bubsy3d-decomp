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
    uint32_t enabled, permit, active, mode, duration, elapsed, callback, mutations;
    uint32_t first[3], second[3], step[3], adjustment[3];
} Input;

typedef struct Output {
    uint32_t result, duration, elapsed, active, mode;
    uint32_t step[3], adjustment[3], first[3], second[3], calls;
} Output;

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

uint8_t g_unknown_800BE024, g_unknown_800BE020, g_unknown_800BE022;
uint8_t g_unknown_80186108, g_unknown_80186109;
uint32_t g_unknown_80186110, g_unknown_80186114;
uint32_t g_unknown_80186118[3], g_unknown_80186148[3];
void (*g_unknown_80186144)(void);
static int first_component, second_component, third_component;
void *g_unknown_800BE528 = &second_component;
static Input current;
static Output result;

static void record(uint32_t call) { result.calls = result.calls * 10 + call; }

static void completion(void) {
    record(8);
    if (current.mutations & 4) {
        g_unknown_80186108 = 7;
        g_unknown_80186114 = 0x87654321;
    }
}

int32_t func_800516F8(void **output) {
    record(1);
    *output = &first_component;
    return 0;
}

int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t *output
) {
    if (basis != NULL) abort();
    if ((const void *)component == &first_component) {
        record(2);
        memcpy(output, current.first, sizeof(current.first));
    } else if ((const void *)component == g_unknown_800BE528) {
        record(5);
        memcpy(output, current.second, sizeof(current.second));
    } else abort();
    return 0;
}

int32_t func_80054AF0(const int16_t *component, int32_t *output) {
    static const int32_t direction[] = {7, 8, 9};
    if ((const void *)component != &first_component) abort();
    record(3);
    memcpy(output, direction, sizeof(direction));
    if (current.mutations & 1) g_unknown_80186118[0] ^= 0x80000000u;
    return 0;
}

int32_t func_80054C7C(
    ComponentTransformView *component, const ComponentBasisView *basis, const int32_t *input
) {
    if (basis != NULL) abort();
    if ((void *)component == &first_component) {
        record(4);
        memcpy(result.first, input, sizeof(result.first));
        if (current.mutations & 2) g_unknown_80186148[1] ^= 0x40000000u;
        if (current.mutations & 16) g_unknown_800BE528 = &third_component;
    } else if ((void *)component == g_unknown_800BE528) {
        record(6);
        memcpy(result.second, input, sizeof(result.second));
    } else abort();
    return 0;
}

int32_t func_800550A4(ComponentRotationMatrix *component, const ComponentRotationMatrix *basis) {
    if ((void *)component != &first_component || (const void *)basis != g_unknown_800BE528) abort();
    record(7);
    if (current.mutations & 8) g_unknown_80186114 ^= 0x80000000u;
    return 0;
}

static Output run(const Input *input) {
    current = *input;
    memset(&result, 0, sizeof(result));
    g_unknown_800BE528 = &second_component;
    g_unknown_800BE024 = (uint8_t)input->enabled;
    g_unknown_800BE020 = (uint8_t)input->permit;
    g_unknown_80186108 = (uint8_t)input->active;
    g_unknown_80186109 = (uint8_t)input->mode;
    g_unknown_80186110 = input->duration;
    g_unknown_80186114 = input->elapsed;
    g_unknown_80186144 = input->callback ? completion : NULL;
    memcpy(g_unknown_80186118, input->step, sizeof(input->step));
    memcpy(g_unknown_80186148, input->adjustment, sizeof(input->adjustment));
    result.result = (uint32_t)func_80032A2C(42);
    result.duration = g_unknown_80186110;
    result.elapsed = g_unknown_80186114;
    result.active = g_unknown_80186108;
    result.mode = g_unknown_80186109;
    memcpy(result.step, g_unknown_80186118, sizeof(result.step));
    memcpy(result.adjustment, g_unknown_80186148, sizeof(result.adjustment));
    return result;
}

int main(int argc, char **argv) {
    Input input;
    size_t bytes;
    if (argc == 1) {
        const Input sample = {1, 1, 1, 0, 3, 0, 1, 0,
                              {1, 2, 3}, {4, 5, 6}, {2, 3, 4}, {5, 6, 7}};
        Output output = run(&sample);
        if (output.result != 1 || output.elapsed != 1 || output.first[0] != 3 ||
            output.second[2] != 13 || output.calls != 1234567) return 1;
        puts("cycle movement update tests passed");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) return 2;
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) return 2;
#endif
    while ((bytes = fread(&input, 1, sizeof(input), stdin)) != 0) {
        Output output;
        if (bytes != sizeof(input)) {
            fputs("incomplete probe input\n", stderr);
            return 2;
        }
        output = run(&input);
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
