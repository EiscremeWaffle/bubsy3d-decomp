#include <stdint.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef BUBSY3D_COMPARISON_REAL_EXTENT
#include "component_bounds.h"
#endif

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

typedef struct ProbeInput {
    uint32_t first, second, distance, mutations, alias, null_basis, output_alias;
    uint32_t distance_status, first_status, second_status;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, output, separate_output, calls, first, second, sampled_first, sampled_second;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 10 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 8 * sizeof(uint32_t), "probe output layout");

extern int32_t func_8005C3A4(void *first, void *second, void *basis, uint8_t *output);

static ProbeInput current;
static ProbeOutput output;
static union ProbeObject {
#ifdef BUBSY3D_COMPARISON_REAL_EXTENT
    ComponentRecordOwner owner;
#endif
    uint32_t word;
} objects[2];
static uint32_t basis;
static uint32_t *second_object;
#ifdef BUBSY3D_COMPARISON_REAL_DISTANCE
static unsigned position_samples;
#endif

static int32_t signed_bits(uint32_t value) {
    int32_t result;
    memcpy(&result, &value, sizeof(result));
    return result;
}

static int32_t finish_distance(int32_t *distance) {
    assert(output.calls++ == 0);
    *distance = signed_bits(current.distance);
    if (current.mutations & 1U) {
        objects[0].word ^= 0x80000000U;
    }
    if (current.mutations & 2U) {
        *second_object ^= 0x7FFFFFFFU;
    }
    return signed_bits(current.distance_status);
}

#ifdef BUBSY3D_COMPARISON_REAL_DISTANCE
int32_t func_80058E7C(void *object, void *input_basis, int32_t *position) {
    assert(object == (position_samples == 0 ? (void *)&objects[0] : (void *)second_object));
    assert(position_samples < 2);
    assert(input_basis == (current.null_basis ? NULL : &basis));
    memset(position, 0, 3 * sizeof(*position));
    position_samples++;
    return -1;
}

int32_t func_80062D4C(const int32_t *first, const int32_t *second, int32_t *distance) {
    (void)first;
    (void)second;
    assert(position_samples == 2);
    return finish_distance(distance);
}
#else
int32_t func_800590B4(void *first, void *second, void *input_basis, int32_t *distance) {
    assert(first == &objects[0] && second == second_object);
    assert(input_basis == (current.null_basis ? NULL : &basis));
    return finish_distance(distance);
}
#endif

#ifdef BUBSY3D_COMPARISON_REAL_EXTENT
int32_t func_80058C74(ComponentRecordOwner *owner, ComponentBounds *bounds) {
    void *object = owner;
    int32_t extent;
#else
int32_t func_80059274(void *object, int32_t *target) {
    int32_t extent;
#endif
    if (output.calls == 1) {
        assert(object == &objects[0]);
        output.sampled_first = objects[0].word;
        extent = signed_bits(objects[0].word);
        if (current.mutations & 4U) {
            *second_object ^= 0x12345678U;
        }
        output.calls++;
#ifdef BUBSY3D_COMPARISON_REAL_EXTENT
        memset(bounds, 0, sizeof(*bounds));
        bounds->words[4] = (uint32_t)extent;
#else
        *target = extent;
#endif
        return signed_bits(current.first_status);
    }
    assert(output.calls++ == 2 && object == second_object);
    output.sampled_second = *second_object;
    extent = signed_bits(*second_object);
#ifdef BUBSY3D_COMPARISON_REAL_EXTENT
    memset(bounds, 0, sizeof(*bounds));
    bounds->words[4] = (uint32_t)extent;
#else
    *target = extent;
#endif
    return signed_bits(current.second_status);
}

#ifdef BUBSY3D_COMPARISON_REAL_EXTENT
int32_t func_80010000(const int32_t *upper, const int32_t *lower, int32_t *difference) {
    (void)lower;
    difference[0] = upper[0];
    difference[1] = INT32_MIN;
    difference[2] = INT32_MIN;
    return -1;
}
#endif

static ProbeOutput run_probe(const ProbeInput *input) {
    uint8_t separate_output = 0xA5;
    uint8_t *output_pointer;

    current = *input;
    memset(&output, 0, sizeof(output));
#ifdef BUBSY3D_COMPARISON_REAL_DISTANCE
    position_samples = 0;
#endif
    objects[0].word = input->first;
    objects[1].word = input->second;
    second_object = input->alias ? &objects[0].word : &objects[1].word;
    output_pointer = input->output_alias ? (uint8_t *)&objects[0].word : &separate_output;
    output.status = (uint32_t)func_8005C3A4(
        &objects[0], second_object, input->null_basis ? NULL : &basis, output_pointer
    );
    output.output = *output_pointer;
    output.separate_output = separate_output;
    output.first = objects[0].word;
    output.second = *second_object;
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.first = 3;
    input.second = 7;
    input.distance = 5;
    input.distance_status = 1;
    input.first_status = 2;
    input.second_status = 3;
    result = run_probe(&input);
    assert(result.status == 0 && result.output == 1 && result.calls == 3);
    input.distance = 6;
    result = run_probe(&input);
    assert(result.output == 0);
    input.distance = 0x80000000U;
    input.first = 0x7FFFFFFFU;
    input.second = 1;
    result = run_probe(&input);
    assert(result.output == 0);
    input.alias = 1;
    input.output_alias = 1;
    input.mutations = 7;
    result = run_probe(&input);
    assert(result.calls == 3 && result.first == result.second && result.separate_output == 0xA5);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("secondary extent comparison tests passed");
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
