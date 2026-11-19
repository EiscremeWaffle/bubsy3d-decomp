#include "heading_timed_records.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

enum {
    MUTATE_LOOKUP = 1 << 0,
    MUTATE_COUNT = 1 << 1,
    MUTATE_POSITION = 1 << 2,
    MUTATE_DRAW = 1 << 3
};

typedef struct ProbeInput {
    uint32_t index, active, base_present, maximum, flag, count, first, second, value, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t active, count, maximum, flag, base_present;
    uint32_t draw_value, draw_first, draw_second, call_count, calls[5];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 10 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 14 * sizeof(uint32_t), "probe output layout");

HeadingTimedRecord g_heading_timed_records[18];
const char g_heading_timed_format[] = "%ld";
static uint8_t elements[0x28];
static HeadingTimedRecord *observed;
static uint32_t mutations, calls[5], call_count, observed_index;
static uint32_t value_bits, count_bits, first_bits, second_bits;
static uint32_t draw_value, draw_first, draw_second;

static void record_call(uint32_t call) {
    assert(call_count < 5);
    calls[call_count++] = call;
}

int32_t func_8002BD0C(int32_t index) {
    int32_t value;
    assert((uint32_t)index == observed_index);
    record_call(1);
    if (mutations & MUTATE_LOOKUP) {
        observed->maximum_34 = 3;
        observed->payload_24 = (void *)(uintptr_t)1;
        observed->elements_20 = 0;
    }
    memcpy(&value, &value_bits, sizeof(value));
    return value;
}

int32_t func_80082F1C(char *output, const char *format, ...) {
    va_list arguments;
    int32_t value;
    assert(format == g_heading_timed_format);
    record_call(2);
    va_start(arguments, format);
    value = va_arg(arguments, int32_t);
    va_end(arguments);
    assert((uint32_t)value == value_bits);
    output[0] = '0';
    output[1] = '\0';
    return 1;
}

int32_t func_8002BAA0(int32_t value) {
    int32_t result;
    assert((uint32_t)value == value_bits);
    record_call(3);
    if (mutations & MUTATE_COUNT) {
        observed->maximum_34 = 2;
    }
    memcpy(&result, &count_bits, sizeof(result));
    return result;
}

void func_8002BE68(int16_t index, int16_t *first, int16_t *second) {
    uint16_t first_value = (uint16_t)first_bits;
    uint16_t second_value = (uint16_t)second_bits;
    assert((uint32_t)(int32_t)index == observed_index);
    record_call(4);
    memcpy(first, &first_value, sizeof(first_value));
    memcpy(second, &second_value, sizeof(second_value));
    if (mutations & MUTATE_POSITION) {
        observed->payload_24 = (void *)(uintptr_t)1;
        observed->element_count_1E = -7;
    }
}

void func_8002C4C8(int16_t index, int32_t value, int16_t first, int16_t second) {
    assert((uint32_t)(int32_t)index == observed_index);
    record_call(5);
    draw_value = (uint32_t)value;
    draw_first = (uint32_t)(int32_t)first;
    draw_second = (uint32_t)(int32_t)second;
    if (mutations & MUTATE_DRAW) {
        observed->active_1C = 2;
    }
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeOutput output;

    assert(input->index == 5 || input->index == 15 || input->index == 17);
    memset(g_heading_timed_records, 0, sizeof(g_heading_timed_records));
    memset(&output, 0, sizeof(output));
    memset(calls, 0, sizeof(calls));
    observed = &g_heading_timed_records[input->index];
    observed->active_1C = (uint8_t)input->active;
    observed->maximum_34 = (uint8_t)input->maximum;
    observed->payload_24 = (void *)(uintptr_t)input->flag;
    observed->elements_20 = input->base_present ? elements : 0;
    observed->element_count_1E = -9;
    observed_index = input->index;
    value_bits = input->value;
    count_bits = input->count;
    first_bits = input->first;
    second_bits = input->second;
    mutations = input->mutations;
    call_count = draw_value = draw_first = draw_second = 0;

    func_8002C604((int16_t)input->index);
    output.active = observed->active_1C;
    output.count = (uint16_t)observed->element_count_1E;
    output.maximum = observed->maximum_34;
    output.flag = (uint32_t)(uintptr_t)observed->payload_24;
    output.base_present = observed->elements_20 != 0;
    output.draw_value = draw_value;
    output.draw_first = draw_first;
    output.draw_second = draw_second;
    output.call_count = call_count;
    memcpy(output.calls, calls, sizeof(calls));
    return output;
}

static void self_test(void) {
    ProbeInput input = {5, 0, 1, 4, 1, 9, 0x7FFF, 0xFFFF, 123, MUTATE_DRAW};
    ProbeOutput output = run_probe(&input);

    assert(output.active == 1 && output.count == 4 && output.call_count == 5);
    assert(output.draw_value == 123 && output.draw_first == 0xFFFF8022U);
    assert(output.draw_second == 0xFFFFFFFFU);
    input.active = 1;
    output = run_probe(&input);
    assert(output.active == 1 && output.count == 0xFFF7 && output.call_count == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("heading timed-start tests passed");
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
