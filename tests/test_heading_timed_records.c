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
    MUTATE_START = 1 << 0,
    MUTATE_CREATE = 1 << 1,
    MUTATE_REMOVE = 1 << 2,
    MUTATE_STOP = 1 << 3
};

typedef struct ProbeInput {
    uint32_t operation, index, duration, stored_duration, event, owner, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t duration, event, owner, call_count, calls[4];
    uint32_t start_index, stop_index, create_owner, create_value, remove_event;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 7 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 13 * sizeof(uint32_t), "probe output layout");

HeadingTimedRecord g_heading_timed_records[18];
static HeadingTimedRecord *observed;
static uint32_t mutations, calls[4], call_count;
static uint32_t start_index, stop_index, create_owner, create_value, remove_event;

static void record_call(uint32_t call) {
    assert(call_count < 4);
    calls[call_count++] = call;
}

void func_8002C604(int16_t index) {
    record_call(1);
    start_index = (uint32_t)(int32_t)index;
    if (mutations & MUTATE_START) {
        observed->owner_04 = (void *)(uintptr_t)0x22222222U;
        observed->duration_30 = 0x33333333;
        observed->event_0C = (GroundedEventRecord *)(uintptr_t)0x44444444U;
    }
}

void func_8002C708(int16_t index) {
    record_call(4);
    stop_index = (uint32_t)(int32_t)index;
    if (mutations & MUTATE_STOP) {
        observed->owner_04 = (void *)(uintptr_t)0x88888888U;
        observed->duration_30 = 0x11111111;
    }
}

int32_t func_8004FF50(void *owner, void *value, GroundedEventRecord **event_out) {
    record_call(2);
    assert(event_out == &observed->event_0C);
    create_owner = (uint32_t)(uintptr_t)owner;
    create_value = (uint32_t)(uintptr_t)value;
    if (mutations & MUTATE_CREATE) {
        *event_out = (GroundedEventRecord *)(uintptr_t)0x55555555U;
    }
    return 3;
}

int32_t func_8004FFEC(GroundedEventRecord *event) {
    record_call(3);
    remove_event = (uint32_t)(uintptr_t)event;
    if (mutations & MUTATE_REMOVE) {
        observed->event_0C = (GroundedEventRecord *)(uintptr_t)0x66666666U;
        observed->duration_30 = 0x77777777;
    }
    return 1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    ProbeOutput output;
    int32_t index, duration;
    unsigned row = input->index < 18 ? input->index : 0;

    memset(g_heading_timed_records, 0, sizeof(g_heading_timed_records));
    memset(&output, 0, sizeof(output));
    memset(calls, 0, sizeof(calls));
    call_count = start_index = stop_index = create_owner = create_value = remove_event = 0;
    mutations = input->mutations;
    observed = &g_heading_timed_records[row];
    observed->event_0C = (GroundedEventRecord *)(uintptr_t)input->event;
    observed->owner_04 = (void *)(uintptr_t)input->owner;
    memcpy(&observed->duration_30, &input->stored_duration, sizeof(input->stored_duration));
    memcpy(&index, &input->index, sizeof(index));
    memcpy(&duration, &input->duration, sizeof(duration));

    if (input->operation == 0) {
        func_8002CA88(index, duration);
    } else if (input->operation == 1) {
        func_8002CB2C(index);
    } else {
        assert(input->operation == 2);
        func_8002CA4C(index, duration);
    }

    output.duration = (uint32_t)observed->duration_30;
    output.event = (uint32_t)(uintptr_t)observed->event_0C;
    output.owner = (uint32_t)(uintptr_t)observed->owner_04;
    output.call_count = call_count;
    memcpy(output.calls, calls, sizeof(calls));
    output.start_index = start_index;
    output.stop_index = stop_index;
    output.create_owner = create_owner;
    output.create_value = create_value;
    output.remove_event = remove_event;
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.index = 5;
    input.duration = 20;
    input.stored_duration = 0xFFFFFFFFU;
    input.owner = 0x12345678;
    input.mutations = MUTATE_START | MUTATE_CREATE;
    output = run_probe(&input);
    assert(output.duration == 0x33333333 && output.event == 0x55555555);
    assert(output.create_owner == 0x22222222 && output.create_value == 0x3C);
    assert(output.call_count == 2 && output.calls[0] == 1 && output.calls[1] == 2);

    input.operation = 1;
    input.event = 0x12345678;
    input.mutations = MUTATE_REMOVE;
    output = run_probe(&input);
    assert(output.duration == 0x77777777 && output.event == 0);
    assert(output.remove_event == 0x12345678 && output.stop_index == 5);

    input.operation = 0;
    input.index = 17;
    input.event = 0;
    input.duration = 0xFFFFFFFFU;
    input.stored_duration = 99;
    input.mutations = 0;
    output = run_probe(&input);
    assert(output.duration == 99 && output.call_count == 2 && output.start_index == 17);

    input.operation = 2;
    input.duration = 0x80000000U;
    output = run_probe(&input);
    assert(output.duration == 0x80000000U && output.call_count == 0);

    input.index = 16;
    output = run_probe(&input);
    assert(output.duration == 99 && output.call_count == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput output;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("heading timed-record tests passed");
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
