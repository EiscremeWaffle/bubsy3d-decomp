#include "component_rotation.h"

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
    uint32_t mode, alias, mutation, component[8], other[8], rotation[3];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, count, component[8], other[8], trace[96];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 22 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 114 * sizeof(uint32_t), "probe output layout");

static ProbeInput current;
static ProbeOutput result;
static ComponentRotationMatrix component, other;
static unsigned condition_calls;

static void record(uint32_t value) {
    assert(result.count < sizeof(result.trace) / sizeof(result.trace[0]));
    result.trace[result.count++] = value;
}

static uint32_t bits(const int32_t *value) {
    uint32_t output;
    memcpy(&output, value, sizeof(output));
    return output;
}

static void mutate_component(void) {
    uint32_t word;
    component.elements[0] ^= 0x1234U;
    component.padding ^= 0xFFFFU;
    memcpy(&word, &component.translation[0], sizeof(word));
    word ^= 0x87654321U;
    memcpy(&component.translation[0], &word, sizeof(word));
}

ComponentRotationMatrix *func_80077C50(const uint16_t angles[3], ComponentRotationMatrix *output) {
    unsigned i;
    record(1);
    for (i = 0; i < 3; i++) record(angles[i]);
    for (i = 0; i < 9; i++) output->elements[i] = (uint16_t)(angles[i % 3] ^ (i * 977U));
    output->padding = 0xABCD;
    output->translation[0] = 123;
    output->translation[1] = -456;
    output->translation[2] = 789;
    if (current.mutation) mutate_component();
    return output;
}

static ComponentRotationMatrix *compose(
    ComponentRotationMatrix *rotation, const ComponentRotationMatrix *basis, uint32_t id) {
    uint32_t words[8];
    unsigned i;
    record(id);
    memcpy(words, basis, sizeof(words));
    for (i = 0; i < 8; i++) record(words[i]);
    rotation->elements[0] ^= 0x9234U;
    return rotation;
}

ComponentRotationMatrix *func_80076890(
    ComponentRotationMatrix *rotation, const ComponentRotationMatrix *basis) {
    return compose(rotation, basis, 2);
}

ComponentRotationMatrix *func_8007699C(
    const ComponentRotationMatrix *basis, ComponentRotationMatrix *rotation) {
    return compose(rotation, basis, 3);
}

int32_t func_80053D68(const int32_t input[3], int32_t output[3]) {
    uint32_t values[3];
    unsigned i;
    record(4);
    record(input == output);
    for (i = 0; i < 3; i++) {
        values[i] = bits(input + i);
        record(values[i]);
    }
    for (i = 0; i < 3; i++) {
        values[i] ^= 0x13579BDFU;
        memcpy(output + i, &values[i], sizeof(values[i]));
    }
    if (current.mode == 0 && current.mutation && condition_calls == 0) mutate_component();
    condition_calls++;
    return -1;
}

int32_t func_80010128(const int32_t left[3], const int32_t right[3], int32_t output[3]) {
    uint32_t a[3], b[3], value;
    unsigned i;
    record(5);
    for (i = 0; i < 3; i++) {
        a[i] = bits(left + i);
        record(a[i]);
    }
    for (i = 0; i < 3; i++) {
        b[i] = bits(right + i);
        record(b[i]);
    }
    for (i = 0; i < 3; i++) {
        value = a[i] ^ (b[i] + i + 1U);
        memcpy(output + i, &value, sizeof(value));
    }
    return -2;
}

int32_t func_8005395C(const int32_t input[3], int32_t output[3]) {
    uint32_t values[3];
    unsigned i;
    record(6);
    for (i = 0; i < 3; i++) {
        values[i] = bits(input + i);
        record(values[i]);
    }
    for (i = 0; i < 3; i++) {
        values[i] ^= 0x2468ACE0U;
        memcpy(output + i, &values[i], sizeof(values[i]));
    }
    return -3;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t rotation[3];
    ComponentRotationMatrix *destination;
    current = *input;
    memset(&result, 0, sizeof(result));
    condition_calls = 0;
    memcpy(&component, input->component, sizeof(component));
    memcpy(&other, input->other, sizeof(other));
    memcpy(rotation, input->rotation, sizeof(rotation));
    if (input->mode == 0) {
        destination = input->alias ? &component : &other;
        result.status = (uint32_t)func_80048E30(&component, destination);
    } else if (input->mode == 1 || input->mode == 2) {
        const int32_t *angles = input->alias ? component.translation : rotation;
        result.status = (uint32_t)(input->mode == 1
            ? func_80048FA8(&component, angles) : func_80049080(&component, angles));
    } else {
        assert(input->mode == 3 || input->mode == 4);
        result.status = (uint32_t)(input->mode == 3
            ? func_80048FA8(NULL, NULL) : func_80049080(NULL, NULL));
    }
    memcpy(result.component, &component, sizeof(component));
    memcpy(result.other, &other, sizeof(other));
    return result;
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t read_count;
    if (argc == 1) {
        memset(&input, 0, sizeof(input));
        output = run_probe(&input);
        assert(output.status == 0 && output.count != 0);
        input.mode = 1;
        output = run_probe(&input);
        assert(output.status == 0 && output.count != 0);
        input.mode = 3;
        output = run_probe(&input);
        assert(output.status == 0x18 && output.count == 0);
        puts("secondary rotation path tests passed");
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
