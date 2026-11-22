#include "secondary_position.h"

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
    uint32_t statuses[6], mutation, null_basis;
    uint32_t bounds[8], difference[3], half[3], center[3], normalized[8], result[3];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, calls, selected_component, result[3];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 36 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 6 * sizeof(uint32_t), "probe output layout");

static ProbeInput current;
static ProbeOutput output;
static SecondaryPositionOwner owner;
static ComponentRotationMatrix components[2];
static uint32_t basis;
#ifdef BUBSY3D_POSITION_REAL_BOUNDS
static ComponentRecord record;
static ComponentRecordTable table;
#endif

static int32_t signed_bits(uint32_t value) {
    int32_t result;
    memcpy(&result, &value, sizeof(result));
    return result;
}

#ifdef BUBSY3D_POSITION_REAL_BOUNDS
int32_t func_800529F8(ComponentRecordOwner *object, int32_t *index) {
    assert(output.calls++ == 0 && object == &owner.records);
    *index = 0;
    return 0;
}
#else
int32_t func_80058C74(ComponentRecordOwner *object, ComponentBounds *bounds) {
    assert(output.calls++ == 0 && object == &owner.records);
    memcpy(bounds->words, current.bounds, sizeof(current.bounds));
    return signed_bits(current.statuses[0]);
}
#endif

int32_t func_80010000(const int32_t *upper, const int32_t *lower, int32_t *difference) {
    assert(output.calls++ == 1);
    assert(memcmp(upper, current.bounds + 4, 3 * sizeof(uint32_t)) == 0);
    assert(memcmp(lower, current.bounds, 3 * sizeof(uint32_t)) == 0);
    memcpy(difference, current.difference, sizeof(current.difference));
    return signed_bits(current.statuses[1]);
}

#ifndef BUBSY3D_POSITION_REAL_HALF
int32_t func_80053EA0(const int32_t *difference, int32_t *half) {
    assert(output.calls++ == 2);
    assert(memcmp(difference, current.difference, sizeof(current.difference)) == 0);
    memcpy(half, current.half, sizeof(current.half));
    return signed_bits(current.statuses[2]);
}
#endif

int32_t func_80010038(const int32_t *lower, const int32_t *half, int32_t *center) {
#ifdef BUBSY3D_POSITION_REAL_HALF
    output.calls++;
#endif
    assert(output.calls++ == 3);
    assert(memcmp(lower, current.bounds, 3 * sizeof(uint32_t)) == 0);
    assert(memcmp(half, current.half, sizeof(current.half)) == 0);
    memcpy(center, current.center, sizeof(current.center));
    if (current.mutation) {
        owner.position.component_14 = &components[1];
    }
    return signed_bits(current.statuses[3]);
}

int32_t func_8005476C(const ComponentRotationMatrix *component, ComponentRotationMatrix *normalized) {
    assert(output.calls++ == 4);
    assert(component == &components[current.mutation ? 1 : 0]);
    output.selected_component = component == &components[1];
    memcpy(normalized, current.normalized, sizeof(current.normalized));
    return signed_bits(current.statuses[4]);
}

int32_t func_80064528(const int32_t *center, const ComponentRotationMatrix *normalized, int32_t *result) {
    assert(output.calls++ == 5);
    assert(memcmp(center, current.center, sizeof(current.center)) == 0);
    assert(memcmp(normalized, current.normalized, sizeof(current.normalized)) == 0);
    memcpy(result, current.result, sizeof(current.result));
    return signed_bits(current.statuses[5]);
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t result[3];

    current = *input;
    memset(&owner, 0, sizeof(owner));
    memset(&output, 0, sizeof(output));
#ifdef BUBSY3D_POSITION_REAL_BOUNDS
    {
        uint32_t record_words[8];

        memcpy(record_words, current.bounds, sizeof(record_words));
        record_words[1] = 0U - current.bounds[5];
        record_words[5] = 0U - current.bounds[1];
        memcpy(&record, record_words, sizeof(record));
        table.records_00 = &record;
        owner.records.table_18 = &table;
    }
#endif
    owner.position.component_14 = &components[0];
    output.status = (uint32_t)func_80058E7C(&owner, input->null_basis ? NULL : &basis, result);
    memcpy(output.result, result, sizeof(output.result));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.mutation = 1;
    input.statuses[0] = 9;
    input.statuses[5] = 17;
    input.bounds[1] = 0x80000000U;
    input.bounds[5] = 0xFFFFFFFFU;
    input.difference[0] = 0x80000000U;
    input.difference[1] = 0xFFFFFFFDU;
    input.difference[2] = 7;
    input.half[0] = 0x40000000U;
    input.half[1] = 0xFFFFFFFFU;
    input.half[2] = 3;
    input.result[0] = 0x80000000U;
    result = run_probe(&input);
    assert(result.status == 0 && result.calls == 6 && result.selected_component == 1);
    assert(result.result[0] == input.result[0]);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("secondary position tests passed");
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
