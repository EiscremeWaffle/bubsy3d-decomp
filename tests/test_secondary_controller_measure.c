#include "secondary_controller_measure.h"
#include "secondary_controller_confirm.h"
#include "secondary_controller_entry.h"
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
    uint32_t target[3], position[3], direction[3], distances[3];
    uint32_t flag, toggle, mode, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t target[3], position[3], direction[3], scaled[3];
    uint32_t distance, adjusted_distance, eligible, branch, flag, toggle, mode, count, calls[18];
} ProbeOutput;

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

ComponentRotationMatrix g_secondary_controller_basis;
uint8_t g_secondary_controller_flag_506;
static ControllerGameContext contexts[2];
static int actor;
static ProbeInput current;
static ProbeOutput output;
static void **saved_context;
static int32_t *saved_direction, *saved_distance, *saved_adjusted_distance;
static unsigned translations, distances, multiplies;

static void log_call(uint32_t call) {
    assert(output.count < 18);
    output.calls[output.count++] = call;
}

static void xor_word(int32_t *value, uint32_t mask) {
    uint32_t bits;
    memcpy(&bits, value, sizeof(bits));
    bits ^= mask;
    memcpy(value, &bits, sizeof(bits));
}

void func_8002CB2C(int32_t index) {
    assert(index == 5);
    log_call(1);
}

int32_t func_8003AA1C(void **result) {
    log_call(2);
    saved_context = result;
    *result = &contexts[0];
    return -1;
}

int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t result[3]
) {
    int is_actor = translations == 0 || translations == 3;
    assert(basis == NULL && translations < 4);
    assert((const void *)component == (is_actor ? (void *)&actor : (void *)&g_secondary_controller_basis));
    log_call(translations < 2 ? 3 + translations : 11 + translations - 2);
    memcpy(result, is_actor ? current.target : current.position, 12);
    if (translations++ == 0 && (current.mutations & 1)) g_secondary_controller_flag_506 ^= 1;
    return -1;
}

int32_t func_80054AF0(const int16_t *component, int32_t *result) {
    assert((const void *)component == (const void *)g_secondary_controller_basis.elements);
    log_call(5);
    memcpy(result, current.direction, 12);
    saved_direction = result;
    return -1;
}

#ifndef SECONDARY_MEASURE_REAL_ARITHMETIC
int32_t func_80062D4C(const int32_t *left, const int32_t *right, int32_t *result) {
    assert(distances < 3);
    log_call(distances < 2 ? 6 + distances * 4 : 13);
    memcpy(result, &current.distances[distances], sizeof(*result));
    if (distances == 0) {
        uint32_t target_y;
        memcpy(&target_y, &right[1], sizeof(target_y));
        assert(target_y == current.target[1] + 80u);
        saved_distance = result;
        if (current.mutations & 2) {
            xor_word((int32_t *)&left[1], 0x01010101u);
            xor_word((int32_t *)&right[0], 0x02020202u);
            xor_word(&saved_direction[2], 0x04040404u);
        }
    } else if (distances == 1) {
        saved_adjusted_distance = result;
        if (current.mutations & 8) {
            g_secondary_controller_flag_506 = 0;
            contexts[0].toggle_13 = 0;
            contexts[0].unknown_00[0x11] = 2;
        }
        if (current.mutations & 16) *saved_context = &contexts[1];
    }
    ++distances;
    return -1;
}

int32_t func_8001007C(int32_t left, int32_t right, int32_t *result) {
    uint32_t bits = (uint32_t)left ^ (uint32_t)right ^ (0xABC00000u + multiplies);
    assert(multiplies < 6);
    if (multiplies >= 3) assert(right == 48);
    log_call(multiplies < 3 ? 7 + multiplies : 14 + multiplies - 3);
    memcpy(result, &bits, sizeof(bits));
    if (multiplies++ == 0 && (current.mutations & 4)) {
        xor_word(&saved_direction[1], 0x08080808u);
        xor_word(saved_distance, 0x10101010u);
    }
    return -1;
}
#else
void func_80010000(const int32_t *left, const int32_t *right, int32_t *result) {
    result[0] = left[0] - right[0];
    result[1] = left[1] - right[1];
    result[2] = left[2] - right[2];
}

int32_t func_800535CC(int32_t squared_length, int32_t *result) {
    assert(distances < 3);
    log_call(distances < 2 ? 6 + distances * 4 : 13);
    log_call((uint32_t)squared_length);
    memcpy(result, &current.distances[distances], sizeof(*result));
    if (distances == 0) {
        saved_distance = result;
    } else if (distances == 1) {
        saved_adjusted_distance = result;
        if (current.mutations & 8) {
            g_secondary_controller_flag_506 = 0;
            contexts[0].toggle_13 = 0;
            contexts[0].unknown_00[0x11] = 2;
        }
        if (current.mutations & 16) *saved_context = &contexts[1];
    }
    ++distances;
    return -1;
}
#endif

void func_8001BC08(void) {
    log_call(17);
    if (current.mutations & 32) xor_word(saved_adjusted_distance, 0x20202020u);
}

int32_t func_8001C6BC(int32_t index, int32_t state, int32_t value) {
    assert(index == 5 && state == 2 && value == 1);
    log_call(18);
    if (current.mutations & 64) xor_word(saved_distance, 0x40404040u);
    return -1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    SecondaryControllerMeasure measure;
    memset(&output, 0, sizeof(output));
    memset(&measure, 0, sizeof(measure));
    memset(contexts, 0, sizeof(contexts));
    current = *input;
    translations = distances = multiplies = 0;
    g_secondary_controller_flag_506 = (uint8_t)input->flag;
    contexts[0].toggle_13 = (uint8_t)input->toggle;
    contexts[0].unknown_00[0x11] = (uint8_t)input->mode;
    output.branch = (uint32_t)secondary_controller_measure(&actor, &measure);
    memcpy(output.target, measure.target, sizeof(output.target));
    memcpy(output.position, measure.position, sizeof(output.position));
    memcpy(output.direction, measure.direction, sizeof(output.direction));
    memcpy(output.scaled, measure.scaled, sizeof(output.scaled));
    memcpy(&output.distance, &measure.distance, sizeof(output.distance));
    memcpy(&output.adjusted_distance, &measure.adjusted_distance, sizeof(output.adjusted_distance));
    output.eligible = (uint32_t)measure.eligible;
    output.flag = g_secondary_controller_flag_506;
    output.toggle = contexts[0].toggle_13;
    output.mode = contexts[0].unknown_00[0x11];
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
    if (argc != 1) return 2;
    {
        ProbeInput input = {{0, 0x7fffffffu, 0}, {0}, {0}, {0, 0, 0}, 0, 0, 0, 0};
#ifdef SECONDARY_MEASURE_REAL_ARITHMETIC
        input.target[1] = 100;
#endif
        ProbeOutput result = run_probe(&input);
#ifdef SECONDARY_MEASURE_REAL_ARITHMETIC
        assert(result.target[1] == 100 && result.eligible == 1 && result.count == 15);
#else
        assert(result.target[1] == 0x7fffffffu && result.eligible == 1 && result.count == 18);
#endif
        input.mutations = 8;
        result = run_probe(&input);
        assert(result.eligible == 0 && result.mode == 2);
        input.mutations = 8 | 16;
        result = run_probe(&input);
        assert(result.eligible == 1);
    }
    puts("secondary controller measurement tests passed");
    return 0;
}
