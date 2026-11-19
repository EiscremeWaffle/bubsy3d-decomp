#include "secondary_controller_helpers.h"

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

#define EMPTY_VALUE UINT32_MAX

typedef struct ProbeInput {
    uint32_t flag, event, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t flag, event, head, first_next, second_next, busy, freed, frees;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 3 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 8 * sizeof(uint32_t), "probe output layout");

static GroundedEventRecord records[3];
GroundedEventRecord *volatile g_secondary_controller_event_8e4;
volatile uint8_t g_secondary_controller_flag_507;
GroundedEventRecord *g_grounded_event_records;
uint8_t g_grounded_event_busy;
static ProbeInput current;
static ProbeOutput output;

static GroundedEventRecord *record_for(uint32_t value) {
    assert(value <= 3);
    return value == 0 ? 0 : &records[value - 1];
}

static uint32_t record_id(GroundedEventRecord *record) {
    unsigned int index;
    if (record == 0) {
        return 0;
    }
    for (index = 0; index < 3; index++) {
        if (record == &records[index]) {
            return index + 1;
        }
    }
    assert(!"unexpected event record");
    return 0;
}

void func_80082EFC(void *record) {
    assert(g_secondary_controller_flag_507 == 1 && g_grounded_event_busy == 1);
    output.freed = record_id(record);
    output.frees++;
    if (current.mutations & 1U) {
        g_secondary_controller_flag_507 = 0xA5;
    }
    if (current.mutations & 2U) {
        g_secondary_controller_event_8e4 = &records[2];
    }
}

static ProbeOutput run_probe(const ProbeInput *input) {
    current = *input;
    memset(&output, 0, sizeof(output));
    memset(records, 0, sizeof(records));
    records[0].next = &records[1];
    g_grounded_event_records = &records[0];
    g_grounded_event_busy = 9;
    g_secondary_controller_flag_507 = (uint8_t)input->flag;
    g_secondary_controller_event_8e4 = record_for(input->event);
    output.freed = EMPTY_VALUE;
    func_800493CC();
    output.flag = g_secondary_controller_flag_507;
    output.event = record_id(g_secondary_controller_event_8e4);
    output.head = record_id(g_grounded_event_records);
    output.first_next = record_id(records[0].next);
    output.second_next = record_id(records[1].next);
    output.busy = g_grounded_event_busy;
    return output;
}

static void self_test(void) {
    ProbeInput input = {7, 1, 3};
    ProbeOutput result = run_probe(&input);

    assert(result.flag == 0xA5 && result.event == 3 && result.head == 2);
    assert(result.busy == 0 && result.freed == 1 && result.frees == 1);
    input.event = 3;
    result = run_probe(&input);
    assert(result.flag == 1 && result.event == 3 && result.head == 1);
    assert(result.busy == 0 && result.freed == EMPTY_VALUE && result.frees == 0);
    input.event = 0;
    result = run_probe(&input);
    assert(result.flag == 1 && result.busy == 9 && result.frees == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("secondary-controller event removal flow tests passed");
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
