#include "component_bounds.h"

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
    uint32_t bounds[8], difference[3], bounds_status, subtract_status, alias, object;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t status, extent, calls, object, bounds[8];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 15 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 12 * sizeof(uint32_t), "probe output layout");

extern int32_t func_80059274(void *object, int32_t *output);

static ProbeInput current;
static ProbeOutput result;
static union {
    ComponentRecordOwner owner;
    uint32_t word;
} object;

static int32_t signed_bits(uint32_t value) {
    int32_t signed_value;
    memcpy(&signed_value, &value, sizeof(signed_value));
    return signed_value;
}

#ifdef BUBSY3D_EXTENT_REAL_BOUNDS
int32_t func_800529F8(ComponentRecordOwner *owner, int32_t *index) {
    assert(owner == &object.owner && result.calls++ == 0);
    *index = 0;
    return 0;
}

int32_t func_80052094(ComponentRecordOwner *owner, uint32_t index, uint32_t *bounds) {
    assert(owner == &object.owner && index == 0 && result.calls++ == 1);
    object.word ^= 0x12345678U;
    memcpy(bounds, current.bounds, sizeof(current.bounds));
    return 0;
}
#else
int32_t func_80058C74(ComponentRecordOwner *owner, ComponentBounds *bounds) {
    assert(owner == &object.owner && result.calls++ == 0);
    object.word ^= 0x12345678U;
    memcpy(bounds->words, current.bounds, sizeof(current.bounds));
    return signed_bits(current.bounds_status);
}
#endif

int32_t func_80010000(const int32_t *upper, const int32_t *lower, int32_t *difference) {
#ifdef BUBSY3D_EXTENT_REAL_BOUNDS
    unsigned i;
    assert(result.calls++ == 2);
#else
    assert(result.calls++ == 1);
#endif
    memcpy(result.bounds, lower, 4 * sizeof(uint32_t));
    memcpy(result.bounds + 4, upper, 4 * sizeof(uint32_t));
#ifdef BUBSY3D_EXTENT_REAL_BOUNDS
    for (i = 0; i < 3; i++) {
        int64_t value = (int64_t)upper[i] - lower[i];
        assert(value >= INT32_MIN && value <= INT32_MAX);
        difference[i] = (int32_t)value;
    }
#else
    memcpy(difference, current.difference, sizeof(current.difference));
#endif
    return signed_bits(current.subtract_status);
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t separate = 0;
    int32_t *target;
    current = *input;
    memset(&result, 0, sizeof(result));
    object.word = input->object;
    target = input->alias ? (int32_t *)&object.word : &separate;
    result.status = (uint32_t)func_80059274(&object.owner, target);
    memcpy(&result.extent, target, sizeof(result.extent));
    result.object = object.word;
    return result;
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        memset(&input, 0, sizeof(input));
#ifdef BUBSY3D_EXTENT_REAL_BOUNDS
        input.bounds[4] = 3;
        input.bounds[5] = 5;
        input.bounds[6] = 7;
        output = run_probe(&input);
        assert(output.status == 0 && output.calls == 3 && output.extent == 7);
        input.alias = 1;
        output = run_probe(&input);
        assert(output.extent == 7 && output.object == output.extent);
#else
        input.difference[0] = 0x80000000U;
        input.difference[1] = 0xFFFFFFFFU;
        input.difference[2] = 0x7FFFFFFFU;
        input.bounds_status = 1;
        input.subtract_status = 2;
        output = run_probe(&input);
        assert(output.status == 0 && output.calls == 2 && output.extent == 0x7FFFFFFFU);
        input.alias = 1;
        input.difference[2] = 0x80000000U;
        output = run_probe(&input);
        assert(output.extent == 0xFFFFFFFFU && output.object == output.extent);
#endif
        puts("secondary extent tests passed");
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
