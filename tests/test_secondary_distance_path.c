#include "secondary_position.h"
#ifdef BUBSY3D_DISTANCE_MODELED_GTE
#include "component_transform.h"
#endif
#ifdef BUBSY3D_DISTANCE_REAL_SQRT
#include "sqrt_backend.h"
#endif

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
    uint32_t alias, mutation, null_basis, result, bounds[2][8], translations[2][3];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, calls, first[3], second[3], result, bounds_y[2];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 26 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 11 * sizeof(uint32_t), "probe output layout");

extern int32_t func_800590B4(void *first, void *second, void *basis, int32_t *output);

static ProbeInput current;
static ProbeOutput output;
static SecondaryPositionOwner objects[2];
static ComponentRotationMatrix components[2];
static union {
    ComponentRecord records[2];
    uint32_t words[16];
} bounds[2];
static uint32_t basis;
static unsigned sampling;
#ifdef BUBSY3D_DISTANCE_REAL_ROTATION_DISPATCH
const volatile ComponentRotationMatrix g_gte_global_basis = {
    {0x1000, 0, 0, 0, 0x1000, 0, 0, 0, 0x1000}, 0, {0, 0, 0}
};
#endif
#ifdef BUBSY3D_DISTANCE_REAL_SQRT
int16_t g_sqrt_lookup_domain[512];
#endif
#ifdef BUBSY3D_DISTANCE_REAL_SEQUENCE
#define BOUNDS_RECORD_OFFSET 8
#else
#define BOUNDS_RECORD_OFFSET 0
#endif
#ifdef BUBSY3D_DISTANCE_ORIGINAL_SUBTRACTION
#define BOUNDS_MUTATION 0x20U
#else
#define BOUNDS_MUTATION 0x80000000U
#endif
#ifdef BUBSY3D_DISTANCE_REAL_BOUNDS
#ifdef BUBSY3D_DISTANCE_REAL_SEQUENCE
static union {
    ComponentRecordTable table;
    PlayerActorSequenceData sequence;
} tables[2];
static int32_t sequence_entries[2] = {-2000, 2001};
#else
static ComponentRecordTable tables[2];
#endif
#endif

static unsigned object_index(const ComponentRecordOwner *object) {
    if (object == &objects[0].records) return 0;
    assert(object == &objects[1].records);
    return 1;
}

static void mutate_second_bounds(void) {
    if (current.mutation) {
#ifdef BUBSY3D_DISTANCE_REAL_BOUNDS
        bounds[current.alias ? 0 : 1].words[BOUNDS_RECORD_OFFSET + 5] ^= BOUNDS_MUTATION;
#else
        bounds[current.alias ? 0 : 1].words[1] ^= BOUNDS_MUTATION;
#endif
    }
}

#ifdef BUBSY3D_DISTANCE_REAL_BOUNDS
int32_t func_800529F8(ComponentRecordOwner *object, int32_t *index) {
    unsigned object_number = object_index(object);

    assert(object_number == (sampling == 0 ? 0U : (current.alias ? 0U : 1U)));
#ifdef BUBSY3D_DISTANCE_REAL_SEQUENCE
    output.calls++;
    return player_actor_read_sequence_remainder(&objects[object_number].actor, index);
#else
    *index = 0;
    output.calls++;
    return 0;
#endif
}
#else
int32_t func_80058C74(ComponentRecordOwner *object, ComponentBounds *target) {
    unsigned index = object_index(object);

    assert(index == (sampling == 0 ? 0U : (current.alias ? 0U : 1U)));
    memcpy(target->words, bounds[index].words, sizeof(current.bounds[index]));
    output.calls++;
    return -1;
}
#endif

int32_t func_80010000(const int32_t *upper, const int32_t *lower, int32_t *difference) {
    unsigned i;

#ifdef BUBSY3D_DISTANCE_REAL_LENGTH
    if (sampling == 2) {
        memcpy(output.first, upper, sizeof(output.first));
        memcpy(output.second, lower, sizeof(output.second));
    }
#endif
#ifdef BUBSY3D_DISTANCE_IDENTITY_BASIS
    if (sampling < 2) {
        if (sampling == 0) mutate_second_bounds();
        sampling++;
        output.calls += 3;
    }
#endif
    for (i = 0; i < 3; i++) {
        uint32_t a, b, result;
        memcpy(&a, upper + i, sizeof(a));
        memcpy(&b, lower + i, sizeof(b));
        result = a - b;
        memcpy(difference + i, &result, sizeof(result));
    }
    output.calls++;
    return -2;
}

#ifndef BUBSY3D_DISTANCE_REAL_TRANSFORM
int32_t func_80010038(const int32_t *lower, const int32_t *half, int32_t *center) {
    unsigned i;

    for (i = 0; i < 3; i++) {
        uint32_t a, b, result;
        memcpy(&a, lower + i, sizeof(a));
        memcpy(&b, half + i, sizeof(b));
        result = a + b;
        memcpy(center + i, &result, sizeof(result));
    }
    output.calls++;
    return -3;
}
#endif

#ifndef BUBSY3D_DISTANCE_REAL_BASIS
int32_t func_8005476C(const ComponentRotationMatrix *component, ComponentRotationMatrix *normalized) {
    assert(component == &components[sampling == 0 ? 0 : (current.alias ? 0 : 1)]);
    *normalized = *component;
    output.calls++;
    return -4;
}
#endif

#ifdef BUBSY3D_DISTANCE_REAL_ROTATION_DISPATCH
int32_t *func_80077298(const volatile void *basis_value, const int32_t *center, int32_t *result) {
    const volatile ComponentRotationMatrix *normalized = basis_value;
#elif defined(BUBSY3D_DISTANCE_REAL_TRANSFORM)
int32_t func_80053FA4(const int32_t *center, const ComponentRotationMatrix *normalized, int32_t *result) {
#elif defined(BUBSY3D_DISTANCE_REAL_WRAPPER)
int32_t func_800540BC(const int32_t *center, const ComponentRotationMatrix *normalized, int32_t *result) {
#else
int32_t func_80064528(const int32_t *center, const ComponentRotationMatrix *normalized, int32_t *result) {
#endif
    unsigned i;

#ifdef BUBSY3D_DISTANCE_IDENTITY_BASIS
    assert(0 && "identity basis should bypass rotation");
#endif
#ifdef BUBSY3D_DISTANCE_REAL_BASIS
    {
        unsigned index = sampling == 0 ? 0U : (current.alias ? 0U : 1U);
        uint32_t y;

        y = (uint32_t)normalized->translation[1];
        assert(y == 0U - current.translations[index][1]);
#ifdef BUBSY3D_DISTANCE_REAL_TRANSFORM
        output.calls += 2;
#else
        output.calls++;
#endif
    }
#endif
#ifdef BUBSY3D_DISTANCE_MODELED_GTE
    {
        GameComponentBasisView model_basis;
        int32_t model_input[4], model_output[4];

        memset(&model_basis, 0, sizeof(model_basis));
        for (i = 0; i < 9; i++) model_basis.matrix_halfwords[i] = normalized->elements[i];
        memcpy(model_input, center, 3 * sizeof(int32_t));
        model_input[3] = 0;
        assert(game_component_basis_transform_point_q12(&model_basis, model_input, model_output) == 0);
        memcpy(result, model_output, 3 * sizeof(int32_t));
    }
#else
    for (i = 0; i < 3; i++) {
        uint32_t a, b, value;
        memcpy(&a, center + i, sizeof(a));
#ifdef BUBSY3D_DISTANCE_REAL_TRANSFORM
        b = 0;
#else
        memcpy(&b, &normalized->translation[i], sizeof(b));
#endif
        value = a + b;
        memcpy(result + i, &value, sizeof(value));
    }
#endif
#if defined(BUBSY3D_DISTANCE_REAL_TRANSFORM) && !defined(BUBSY3D_DISTANCE_REAL_ROTATION_DISPATCH)
    result[3] = 0;
#endif
    if (sampling == 0) mutate_second_bounds();
    sampling++;
    output.calls++;
#ifdef BUBSY3D_DISTANCE_REAL_ROTATION_DISPATCH
    return result;
#else
    return -5;
#endif
}

#ifdef BUBSY3D_DISTANCE_REAL_LENGTH
#ifndef BUBSY3D_DISTANCE_REAL_SQRT
int32_t func_800535CC(int32_t squared_length, int32_t *result) {
    uint32_t expected = 0;
    unsigned i;

    assert(sampling == 2);
    for (i = 0; i < 3; i++) {
        uint32_t difference = output.first[i] - output.second[i];
        expected += difference * difference;
    }
    assert((uint32_t)squared_length == expected);
    memcpy(result, &current.result, sizeof(current.result));
    return -6;
}
#endif
#else
int32_t func_80062D4C(const int32_t *first, const int32_t *second, int32_t *result) {
    assert(sampling == 2);
    memcpy(output.first, first, sizeof(output.first));
    memcpy(output.second, second, sizeof(output.second));
    memcpy(result, &current.result, sizeof(current.result));
    output.calls++;
    return -6;
}
#endif

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t result;
    unsigned i;

    current = *input;
    memset(&output, 0, sizeof(output));
    memset(objects, 0, sizeof(objects));
    memset(components, 0, sizeof(components));
    sampling = 0;
#ifdef BUBSY3D_DISTANCE_REAL_SQRT
    for (i = 0; i < 512; i++) {
        g_sqrt_lookup_domain[i] = (int16_t)((i * 97U + 13U) & 0x7FFFU);
    }
#endif
    for (i = 0; i < 2; i++) {
        memset(&bounds[i], 0, sizeof(bounds[i]));
        memcpy(bounds[i].words + BOUNDS_RECORD_OFFSET, current.bounds[i], sizeof(current.bounds[i]));
#ifdef BUBSY3D_DISTANCE_REAL_BOUNDS
        bounds[i].words[BOUNDS_RECORD_OFFSET + 1] = 0U - current.bounds[i][5];
        bounds[i].words[BOUNDS_RECORD_OFFSET + 5] = 0U - current.bounds[i][1];
#ifdef BUBSY3D_DISTANCE_REAL_SEQUENCE
        memset(&tables[i], 0, sizeof(tables[i]));
        tables[i].table.records_00 = bounds[i].records;
        tables[i].sequence.entries = sequence_entries;
        tables[i].sequence.flags_2C = current.null_basis ? 0 : 4;
        objects[i].records.table_18 = &tables[i].table;
        objects[i].actor.cursor_08 = 1;
#else
        tables[i].records_00 = bounds[i].records;
        objects[i].records.table_18 = &tables[i];
#endif
#endif
        objects[i].position.component_14 = &components[i];
#ifdef BUBSY3D_DISTANCE_MODELED_GTE
        {
#ifdef BUBSY3D_DISTANCE_IDENTITY_BASIS
            static const uint16_t rotation[9] = {0x1000, 0, 0, 0, 0x1000, 0, 0, 0, 0x1000};
#else
            static const uint16_t rotation[9] = {0, 0x1000, 0, 0xF000, 0, 0, 0, 0, 0x1000};
#endif
            memcpy(components[i].elements, rotation, sizeof(rotation));
        }
#endif
        memcpy(components[i].translation, current.translations[i], sizeof(current.translations[i]));
    }
    output.status = (uint32_t)func_800590B4(
        &objects[0], &objects[current.alias ? 0 : 1], current.null_basis ? NULL : &basis, &result
    );
    memcpy(&output.result, &result, sizeof(result));
    for (i = 0; i < 2; i++) {
#ifdef BUBSY3D_DISTANCE_REAL_BOUNDS
        output.bounds_y[i] = 0U - bounds[i].words[BOUNDS_RECORD_OFFSET + 5];
#else
        output.bounds_y[i] = bounds[i].words[1];
#endif
    }
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.alias = 1;
    input.mutation = 1;
    input.result = 0xABCDEF12U;
    input.bounds[0][5] = 8;
    input.translations[0][1] = 4;
    result = run_probe(&input);
#ifdef BUBSY3D_DISTANCE_REAL_SQRT
    {
        uint32_t squared_length = 0;
        unsigned i;

        for (i = 0; i < 3; i++) {
            uint32_t difference = result.first[i] - result.second[i];
            squared_length += difference * difference;
        }
        assert(result.result == (uint32_t)func_800764D0((int32_t)squared_length));
    }
    assert(result.status == 0 && result.calls == 11);
#else
    assert(result.status == 0 && result.calls == 11 && result.result == input.result);
#endif
#if defined(BUBSY3D_DISTANCE_REAL_BOUNDS) && defined(BUBSY3D_DISTANCE_ORIGINAL_SUBTRACTION)
    assert(result.bounds_y[0] == 0U - BOUNDS_MUTATION);
#else
    assert(result.bounds_y[0] == BOUNDS_MUTATION);
#endif
#ifdef BUBSY3D_DISTANCE_REAL_BASIS
#ifdef BUBSY3D_DISTANCE_MODELED_GTE
#ifdef BUBSY3D_DISTANCE_IDENTITY_BASIS
    assert(result.first[1] == 0 && result.second[1] == 0xFFFFFFF0U);
#else
    assert(result.first[1] == 0xFFFFFFFCU && result.second[1] == 0xFFFFFFFCU);
#endif
#else
    assert(result.first[1] == 0 && result.second[1] == 0x40000000U);
#endif
#else
    assert(result.first[1] == 8 && result.second[1] == 0x40000008U);
#endif
#ifdef BUBSY3D_DISTANCE_REAL_SEQUENCE
    input.null_basis = 1;
    result = run_probe(&input);
#ifdef BUBSY3D_DISTANCE_REAL_SQRT
    assert(result.status == 0 && result.calls == 11);
#else
    assert(result.status == 0 && result.calls == 11 && result.result == input.result);
#endif
#ifdef BUBSY3D_DISTANCE_MODELED_GTE
#ifdef BUBSY3D_DISTANCE_IDENTITY_BASIS
    assert(result.first[1] == 0 && result.second[1] == 0xFFFFFFF0U);
#else
    assert(result.first[1] == 0xFFFFFFFCU && result.second[1] == 0xFFFFFFFCU);
#endif
#else
    assert(result.first[1] == 0 && result.second[1] == 0x40000000U);
#endif
#endif
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("secondary distance path tests passed");
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
