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

typedef struct ProbeInput {
    uint32_t flag, event, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t flag, event, argument, calls;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 3 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 4 * sizeof(uint32_t), "probe output layout");

static GroundedEventRecord records[2];
GroundedEventRecord *volatile g_secondary_controller_event_8e4;
volatile uint8_t g_secondary_controller_flag_507;
static ProbeInput current;
static ProbeOutput output;

static GroundedEventRecord *record_for(uint32_t value) {
    assert(value <= 2);
    return value == 0 ? 0 : &records[value - 1];
}

static uint32_t record_id(GroundedEventRecord *record) {
    if (record == 0) {
        return 0;
    }
    if (record == &records[0]) {
        return 1;
    }
    assert(record == &records[1]);
    return 2;
}

int32_t func_8004FFEC(GroundedEventRecord *record) {
    assert(g_secondary_controller_flag_507 == 1);
    output.argument = record_id(record);
    output.calls++;
    if (current.mutations & 1U) {
        g_secondary_controller_flag_507 = 0xA5;
    }
    if (current.mutations & 2U) {
        g_secondary_controller_event_8e4 = &records[1];
    }
    return -1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    current = *input;
    memset(&output, 0, sizeof(output));
    g_secondary_controller_flag_507 = (uint8_t)input->flag;
    g_secondary_controller_event_8e4 = record_for(input->event);
    func_800493CC();
    output.flag = g_secondary_controller_flag_507;
    output.event = record_id(g_secondary_controller_event_8e4);
    return output;
}

static void self_test(void) {
    ProbeInput input = {255, 1, 3};
    ProbeOutput result = run_probe(&input);

    assert(result.flag == 0xA5 && result.event == 2);
    assert(result.argument == 1 && result.calls == 1);
    input.event = 0;
    input.mutations = 0;
    result = run_probe(&input);
    assert(result.flag == 1 && result.event == 0 && result.argument == 0 && result.calls == 1);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("secondary-controller event helper tests passed");
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
