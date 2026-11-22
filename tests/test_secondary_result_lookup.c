#include "actor_context.h"
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

typedef struct ProbeSlot {
    int32_t word_00, word_04;
    int8_t byte_08;
    uint8_t padding[3];
    int32_t word_0C, word_10;
} ProbeSlot;

typedef struct ProbeContext {
    uint8_t unknown[0x14];
    int16_t selection;
    uint8_t padding[2];
    ProbeSlot slots[2];
} ProbeContext;

typedef struct ProbeInput {
    uint32_t value;
    uint32_t selection;
    int32_t callback_result;
    int32_t query_result;
    int32_t durations[3];
    uint32_t slot_values[10];
} ProbeInput;

typedef struct ProbeOutput {
    int32_t result;
    uint32_t getter_value;
    uint32_t getter_calls;
    uint32_t callback_kind;
    uint32_t callback_argument;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 17 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 5 * sizeof(uint32_t), "probe output layout");

HeadingTimedRecord g_heading_timed_records[18];
static ProbeContext context;
static ProbeInput current;
static ProbeOutput output;

void *func_80026648(void *actor, void *unused) {
    (void)unused;
    output.getter_calls++;
    output.getter_value = (uint32_t)(uintptr_t)actor;
    return &context;
}

int32_t func_8001AD28(int32_t value) {
    assert(value == 15);
    output.callback_kind = 1;
    output.callback_argument = (uint32_t)value;
    return current.callback_result;
}

int32_t func_80037AFC(void) {
    output.callback_kind = 2;
    return current.query_result;
}

static int32_t value_as_signed(uint32_t bits) {
    int32_t value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    unsigned slot;
    uint32_t field;
    int32_t value = value_as_signed(input->value);

    current = *input;
    memset(&output, 0, sizeof(output));
    memset(&context, 0, sizeof(context));
    memset(g_heading_timed_records, 0, sizeof(g_heading_timed_records));
    context.selection = (int16_t)input->selection;
    for (slot = 0; slot < 2; slot++) {
        ProbeSlot *destination = &context.slots[slot];
        const uint32_t *source = &input->slot_values[slot * 5];
        destination->word_00 = value_as_signed(source[0]);
        destination->word_04 = value_as_signed(source[1]);
        destination->byte_08 = (int8_t)source[2];
        destination->word_0C = value_as_signed(source[3]);
        destination->word_10 = value_as_signed(source[4]);
    }
    g_heading_timed_records[5].duration_30 = input->durations[0];
    g_heading_timed_records[15].duration_30 = input->durations[1];
    g_heading_timed_records[17].duration_30 = input->durations[2];
    output.result = func_8002BD0C(value);
    field = output.getter_calls;
    assert(field == 1);
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.value = 1;
    input.selection = 1;
    input.slot_values[6] = 0x12345678;
    result = run_probe(&input);
    assert(result.result == 0x12345678 && result.getter_value == 1);
    input.value = 12;
    input.callback_result = -77;
    result = run_probe(&input);
    assert(result.result == -77 && result.callback_kind == 1);
    input.value = 17;
    input.durations[2] = 1000;
    result = run_probe(&input);
    assert(result.result == 999 && result.callback_kind == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("secondary result lookup tests passed");
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
