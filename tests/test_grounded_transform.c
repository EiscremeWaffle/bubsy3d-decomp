#include "component_rotation.h"
#include "grounded_transform.h"
#include "queue_update.h"

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
    uint32_t mode, gate, alias, mutation;
    uint32_t direction[4], output[4], translation[3], distance, parameter;
#ifdef GROUNDED_TRANSFORM_REAL_SQRT
    uint16_t lookup[512];
#endif
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t count, trace[8], direction[4], output[4];
} ProbeOutput;

typedef struct ProbeVector {
    GroundedTransformVector vector;
    uint32_t canary;
} ProbeVector;

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

static ProbeInput current;
static ProbeOutput result;
static ProbeVector direction, output;
static ComponentRotationMatrix component_a, component_b, component_c;
void *g_unknown_800BE528;
uint8_t g_unknown_800BE024;

static void record(uint32_t value) {
    assert(result.count < 8);
    result.trace[result.count++] = value;
}

int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t *target
) {
    assert((const void *)component == &component_a && basis == NULL);
    record(1);
    memcpy(target, current.translation, sizeof(current.translation));
    if (current.mutation) direction.vector.coordinates.y ^= 0x12345678u;
    return -1;
}

#if !defined(GROUNDED_TRANSFORM_REAL_SCALE) && !defined(GROUNDED_TRANSFORM_REAL_ARITHMETIC)
int32_t func_80053D00(const int32_t *input, int32_t distance, int32_t *target) {
    uint32_t values[3];
    memcpy(values, input, sizeof(values));
    record(2);
    record((uint32_t)distance);
    record(values[0]);
    record(values[1]);
    record(values[2]);
    values[0] ^= (uint32_t)distance;
    values[1] += 0x23456789u;
    values[2] ^= 0xabcdef01u;
    memcpy(target, values, sizeof(values));
    return -2;
}
#elif defined(GROUNDED_TRANSFORM_REAL_ARITHMETIC)
#ifdef GROUNDED_TRANSFORM_REAL_SQRT
int16_t g_sqrt_lookup_domain[512];
#else
int32_t func_80076554(int32_t value) {
    int32_t square_root;
    record(2);
    record((uint32_t)value);
    memcpy(&square_root, &current.translation[1], sizeof(square_root));
    return square_root;
}

int32_t func_800764D0(int32_t value) {
    (void)value;
    assert(0);
    return 0;
}
#endif
#else
int32_t func_8005385C(const int32_t *input, int32_t *target) {
    uint32_t values[3];
    memcpy(values, input, sizeof(values));
    record(2);
    record(values[0]);
    record(values[1]);
    record(values[2]);
    memcpy(target, &current.translation[1], sizeof(*target));
    return -1;
}

int32_t func_80053A08(const int32_t *input, int32_t distance, int32_t *target) {
    uint32_t values[3];
    memcpy(values, input, sizeof(values));
    record(6);
    record((uint32_t)distance);
    values[0] ^= (uint32_t)distance;
    values[1] += 0x23456789u;
    values[2] ^= 0xabcdef01u;
    memcpy(target, values, sizeof(values));
    return -2;
}
#endif

int32_t func_80054C7C(
    ComponentTransformView *component, const ComponentBasisView *basis, const int32_t *input
) {
    uint32_t values[3];
    assert((void *)component == &component_a && basis == NULL);
    memcpy(values, input, sizeof(values));
    record(3);
    record(values[0]);
    record(values[1]);
    record(values[2]);
    if (current.mutation) direction.vector.coordinates.y ^= 0x87654321u;
    return -3;
}

int32_t func_800516F8(void **target) {
    record(4);
    *target = &component_b;
    if (current.mutation) g_unknown_800BE528 = &component_c;
    return -4;
}

int32_t func_800550A4(ComponentRotationMatrix *component, const ComponentRotationMatrix *basis) {
    record(5);
    record(component == &component_b);
    record(basis == &component_c ? 3 : basis == &component_a ? 1 : 0);
    return -5;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    GroundedTransformVector *target;
#ifdef GROUNDED_TRANSFORM_REAL_SQRT
    memcpy(g_sqrt_lookup_domain, input->lookup, sizeof(g_sqrt_lookup_domain));
#endif
    current = *input;
    memset(&result, 0, sizeof(result));
    memcpy(&direction, input->direction, sizeof(input->direction));
    memcpy(&output, input->output, sizeof(input->output));
    g_unknown_800BE528 = &component_a;
    g_unknown_800BE024 = (uint8_t)input->gate;
    if (input->mode == 0) {
        target = input->alias ? &direction.vector : &output.vector;
        func_8002E858(&direction.vector, (int32_t)input->distance, input->parameter, target);
    } else if (input->mode == 1) {
        func_8002E954(&direction.vector, input->parameter);
    } else {
        assert(input->mode == 2);
        func_8002EA80();
    }
    memcpy(result.direction, &direction, sizeof(result.direction));
    memcpy(result.output, &output, sizeof(result.output));
    return result;
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
            ProbeOutput output_value;
            if (bytes_read != sizeof(input)) {
                fputs("incomplete probe input\n", stderr);
                return 2;
            }
            output_value = run_probe(&input);
            if (fwrite(&output_value, sizeof(output_value), 1, stdout) != 1) {
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
#ifdef GROUNDED_TRANSFORM_REAL_ARITHMETIC
    {
        ProbeInput input = {0};
        ProbeOutput actual;
        input.direction[0] = 4096;
        input.output[3] = 0x11223344u;
        input.translation[0] = 10;
        input.translation[1] = 4096;
        input.translation[2] = 20;
        input.distance = 4096;
        input.parameter = 30;
#ifdef GROUNDED_TRANSFORM_REAL_SQRT
        input.lookup[320] = 4096;
#endif
        actual = run_probe(&input);
#ifdef GROUNDED_TRANSFORM_REAL_SQRT
        assert(actual.count == 1 && actual.trace[0] == 1);
#else
        assert(actual.count == 3 && actual.trace[0] == 1 && actual.trace[1] == 2);
        assert(actual.trace[2] == 4096);
#endif
        assert(actual.output[0] == 4106 && actual.output[1] == 30);
        assert(actual.output[2] == 20 && actual.output[3] == 0x11223344u);
    }
#else
    {
        ProbeInput input = {0};
        ProbeOutput actual;
        input.direction[0] = 0xffffffffu;
        input.direction[1] = 0x7fffffffu;
        input.direction[2] = 0x80000000u;
        input.output[3] = 0x11223344u;
        input.translation[0] = 2;
        input.translation[2] = 0x80000000u;
        input.distance = 1;
        input.parameter = 0xffffffffu;
        actual = run_probe(&input);
        assert(actual.count == 6 && actual.trace[0] == 1 && actual.trace[1] == 2);
        assert(actual.output[0] == 0 && actual.output[1] == 0xa3456787u);
        assert(actual.output[2] == 0xabcdef01u && actual.output[3] == 0x11223344u);
        input.mode = 1;
        input.parameter = 2;
        input.mutation = 1;
        actual = run_probe(&input);
        assert(actual.count == 4 && actual.trace[2] == 0x80000001u);
        assert(actual.direction[1] == ((0x80000001u ^ 0x87654321u) - 2u));
        input.mode = 2;
        input.gate = 0;
        actual = run_probe(&input);
        assert(actual.count == 0);
        input.gate = 255;
        actual = run_probe(&input);
        assert(actual.count == 4 && actual.trace[0] == 4 && actual.trace[1] == 5);
        assert(actual.trace[2] == 1 && actual.trace[3] == 3);
    }
#endif
    puts("grounded transform tests passed");
    return 0;
}
