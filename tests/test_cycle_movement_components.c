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
    uint32_t enabled, permit, active, mode, duration, elapsed;
    uint32_t first[3], second[3], step[3], adjustment[3];
} Input;

typedef struct Output {
    uint32_t result, duration, elapsed, active, mode;
    uint32_t first[3], second[3], calls;
} Output;

typedef struct ComponentBasisView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentBasisView;

typedef struct ComponentTransformView {
    uint32_t matrix_words[4];
    int16_t matrix_last;
    int16_t padding;
    int32_t translation[3];
} ComponentTransformView;

uint8_t g_unknown_800BE024, g_unknown_800BE020, g_unknown_800BE022;
uint8_t g_unknown_80186108, g_unknown_80186109;
uint32_t g_unknown_80186110, g_unknown_80186114;
uint32_t g_unknown_80186118[3], g_unknown_80186148[3];
void (*g_unknown_80186144)(void);
static ComponentTransformView first, second;
void *g_l0_gp_888 = &first;
void *g_unknown_800BE528 = &second;
const ComponentBasisView g_global_basis = {{0}, 0, 0, {0}};
static uint32_t calls;

int32_t func_8005476C(const ComponentBasisView *basis, ComponentBasisView *output) {
    (void)basis;
    (void)output;
    abort();
}

int32_t func_80064548(const int32_t *input, const ComponentBasisView *basis, int32_t *output) {
    (void)input;
    (void)basis;
    (void)output;
    abort();
}

int32_t func_80064528(const int32_t *input, const ComponentBasisView *basis, int32_t *output) {
    (void)input;
    (void)basis;
    (void)output;
    abort();
}

int32_t func_800550A4(ComponentRotationMatrix *component, const ComponentRotationMatrix *basis) {
    if ((void *)component != &first || (const void *)basis != &second) abort();
    calls++;
    return 0;
}

static void completion(void) { calls += 2; }

static Output run(const Input *input) {
    Output output;
    memset(&first, 0, sizeof(first));
    memset(&second, 0, sizeof(second));
    memcpy(first.translation, input->first, sizeof(input->first));
    memcpy(second.translation, input->second, sizeof(input->second));
    memcpy(g_unknown_80186118, input->step, sizeof(input->step));
    memcpy(g_unknown_80186148, input->adjustment, sizeof(input->adjustment));
    g_unknown_800BE024 = (uint8_t)input->enabled;
    g_unknown_800BE020 = (uint8_t)input->permit;
    g_unknown_80186108 = (uint8_t)input->active;
    g_unknown_80186109 = (uint8_t)input->mode;
    g_unknown_80186110 = input->duration;
    g_unknown_80186114 = input->elapsed;
    g_unknown_80186144 = completion;
    calls = 0;
    output.result = (uint32_t)func_80032A2C(42);
    output.duration = g_unknown_80186110;
    output.elapsed = g_unknown_80186114;
    output.active = g_unknown_80186108;
    output.mode = g_unknown_80186109;
    memcpy(output.first, first.translation, sizeof(output.first));
    memcpy(output.second, second.translation, sizeof(output.second));
    output.calls = calls;
    return output;
}

int main(int argc, char **argv) {
    Input input;
    size_t bytes;
    if (argc == 1) {
        const Input sample = {1, 1, 1, 0, 3, 0,
                              {1, 2, 3}, {4, 5, 6}, {2, 3, 4}, {5, 6, 7}};
        Output output = run(&sample);
        if (output.result != 1 || output.elapsed != 1 || output.first[0] != 3 ||
            output.first[1] != (uint32_t)-1 || output.second[2] != 13 || output.calls != 1) return 1;
        puts("cycle movement component integration tests passed");
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
