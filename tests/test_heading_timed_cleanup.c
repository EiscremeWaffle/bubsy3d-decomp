#include "heading_timed_records.h"

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

enum {
    SHORTEN_COUNT = 1 << 0,
    REPLACE_ELEMENTS = 1 << 1,
    EXTEND_COUNT = 1 << 2,
    MUTATE_ACTIVE = 1 << 3,
    NEGATE_COUNT = 1 << 4
};

typedef struct ProbeInput {
    uint32_t index, active, count, base_present, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t active, count, base, call_count, calls[4];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 5 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 8 * sizeof(uint32_t), "probe output layout");

HeadingTimedRecord g_heading_timed_records[18];
static uint8_t elements[2][4 * 0x28];
static HeadingTimedRecord *observed;
static uint32_t mutations, calls[4], call_count;

static uint32_t element_code(const void *payload) {
    unsigned base, index;
    for (base = 0; base < 2; ++base) {
        for (index = 0; index < 4; ++index) {
            if (payload == &elements[base][index * 0x28]) {
                return base * 10U + index + 1U;
            }
        }
    }
    assert(0);
    return 0;
}

int32_t func_80050650(void *payload) {
    assert(call_count < 4);
    calls[call_count++] = element_code(payload);
    if (call_count == 1) {
        if (mutations & SHORTEN_COUNT) {
            observed->element_count_1E = 1;
        }
        if (mutations & REPLACE_ELEMENTS) {
            observed->elements_20 = &elements[1][0];
        }
        if (mutations & EXTEND_COUNT) {
            observed->element_count_1E = 3;
        }
        if (mutations & MUTATE_ACTIVE) {
            observed->active_1C = 0x7F;
        }
        if (mutations & NEGATE_COUNT) {
            observed->element_count_1E = -1;
        }
    }
    return 1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeOutput output;
    uint16_t count_bits = (uint16_t)input->count;

    assert(input->index == 5 || input->index == 15);
    memset(g_heading_timed_records, 0, sizeof(g_heading_timed_records));
    memset(&output, 0, sizeof(output));
    memset(calls, 0, sizeof(calls));
    observed = &g_heading_timed_records[input->index];
    observed->active_1C = (uint8_t)input->active;
    memcpy(&observed->element_count_1E, &count_bits, sizeof(count_bits));
    observed->elements_20 = input->base_present ? &elements[0][0] : 0;
    mutations = input->mutations;
    call_count = 0;

    func_8002C708((int16_t)input->index);
    output.active = observed->active_1C;
    output.count = (uint16_t)observed->element_count_1E;
    output.base = observed->elements_20 == 0 ? 0 :
                  observed->elements_20 == &elements[0][0] ? 1 : 2;
    output.call_count = call_count;
    memcpy(output.calls, calls, sizeof(calls));
    return output;
}

static void self_test(void) {
    ProbeInput input = {5, 1, 2, 1, REPLACE_ELEMENTS | EXTEND_COUNT};
    ProbeOutput output = run_probe(&input);

    assert(output.active == 0 && output.count == 0 && output.base == 2);
    assert(output.call_count == 3 && output.calls[0] == 1);
    assert(output.calls[1] == 12 && output.calls[2] == 13);
    input.base_present = 0;
    output = run_probe(&input);
    assert(output.active == 1 && output.count == 2 && output.call_count == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("heading timed-cleanup tests passed");
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
