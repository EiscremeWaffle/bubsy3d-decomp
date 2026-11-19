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
    MUTATE_RESOURCE_EVENT = 1 << 0,
    MUTATE_CLEANUP_EVENT = 1 << 1,
    MUTATE_REMOVED_RECORD = 1 << 2
};

typedef struct ProbeInput {
    uint32_t index, resource, event, state_08, state_1D, state_10, mutations, replacement_event;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t event, resource, state_08, state_1D, state_10, call_count, calls[3];
    uint32_t resource_argument, cleanup_index, remove_argument;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 8 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 12 * sizeof(uint32_t), "probe output layout");

HeadingTimedRecord g_heading_timed_records[18];
static HeadingTimedRecord *observed;
static uint32_t mutations, replacement_event, calls[3], call_count;
static uint32_t resource_argument, cleanup_index, remove_argument;
static void *resource_pointer;

static void record_call(uint32_t call) {
    assert(call_count < 3);
    calls[call_count++] = call;
}

int32_t func_80050650(void *payload) {
    record_call(1);
    resource_pointer = payload;
    resource_argument = (uint32_t)(uintptr_t)payload;
    if (mutations & MUTATE_RESOURCE_EVENT) {
        observed->event_0C = (GroundedEventRecord *)(uintptr_t)replacement_event;
    }
    return 0;
}

void func_8002C708(int16_t index) {
    record_call(2);
    cleanup_index = (uint32_t)(int32_t)index;
    if (mutations & MUTATE_CLEANUP_EVENT) {
        observed->event_0C = (GroundedEventRecord *)(uintptr_t)replacement_event;
    }
}

int32_t func_8004FFEC(GroundedEventRecord *event) {
    record_call(3);
    remove_argument = (uint32_t)(uintptr_t)event;
    if (mutations & MUTATE_REMOVED_RECORD) {
        observed->event_0C = (GroundedEventRecord *)(uintptr_t)0x55555555U;
        observed->payload_24 = (void *)(uintptr_t)0x66666666U;
        observed->state_08 = 0x77;
        observed->state_1D = 0x88;
        observed->state_10 = 0x99999999U;
    }
    return 0;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeOutput output;
    uint16_t index = (uint16_t)input->index;

    assert(index < 18);
    memset(g_heading_timed_records, 0, sizeof(g_heading_timed_records));
    memset(&output, 0, sizeof(output));
    memset(calls, 0, sizeof(calls));
    call_count = resource_argument = cleanup_index = remove_argument = 0;
    resource_pointer = 0;
    observed = &g_heading_timed_records[index];
    observed->payload_24 = (void *)(uintptr_t)input->resource;
    observed->event_0C = (GroundedEventRecord *)(uintptr_t)input->event;
    observed->state_08 = (uint8_t)input->state_08;
    observed->state_1D = (uint8_t)input->state_1D;
    observed->state_10 = input->state_10;
    mutations = input->mutations;
    replacement_event = input->replacement_event;
    func_8002BAD4((int16_t)index);

    output.event = (uint32_t)(uintptr_t)observed->event_0C;
    output.resource = (uint32_t)(uintptr_t)observed->payload_24;
    output.state_08 = observed->state_08;
    output.state_1D = observed->state_1D;
    output.state_10 = observed->state_10;
    output.call_count = call_count;
    memcpy(output.calls, calls, sizeof(calls));
    output.resource_argument = resource_argument;
    output.cleanup_index = cleanup_index;
    output.remove_argument = remove_argument;
    return output;
}

static void self_test(void) {
    uint8_t live_payload;
    ProbeInput input = {5, 0x1234, 0, 7, 8, 9, MUTATE_CLEANUP_EVENT | MUTATE_REMOVED_RECORD, 0x4321};
    ProbeOutput output = run_probe(&input);

    assert(output.call_count == 3 && output.calls[0] == 1 && output.calls[1] == 2 && output.calls[2] == 3);
    assert(output.remove_argument == 0x4321 && output.event == 0);
    assert(output.resource == 0x66666666 && output.state_08 == 0 && output.state_1D == 0 && output.state_10 == 0);

    input.resource = 0;
    input.event = 0x4321;
    input.mutations = MUTATE_CLEANUP_EVENT;
    input.replacement_event = 0;
    output = run_probe(&input);
    assert(output.call_count == 1 && output.calls[0] == 2);
    assert(output.event == 0 && output.state_08 == 7 && output.state_1D == 8 && output.state_10 == 9);

    memset(g_heading_timed_records, 0, sizeof(g_heading_timed_records));
    observed = &g_heading_timed_records[5];
    observed->payload_24 = &live_payload;
    mutations = call_count = 0;
    resource_pointer = 0;
    func_8002BAD4(5);
    assert(resource_pointer == &live_payload && call_count == 2);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("heading timed-delete tests passed");
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
