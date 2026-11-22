#include "actor_context.h"
#include "controller_result.h"
#include "grounded_event_records.h"

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
    uint32_t delta, selection, first, second, event, mutation, replacement, status;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t selection, first, second, event, expiration_selection, calls, chosen, scheduled_selection;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 8 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 8 * sizeof(uint32_t), "probe output layout");

extern void func_8002E348(int32_t delta);
GroundedEventRecord *g_result_expiration_event;
int16_t g_result_expiration_selection;

static ProbeInput current;
static ProbeOutput output;
static union {
    ResultCounterContext value;
    uint8_t bytes[0x40];
} context;
static GroundedEventRecord event_record;

static void write16(unsigned offset, uint16_t value) {
    memcpy(context.bytes + offset, &value, sizeof(value));
}

static uint16_t read16(unsigned offset) {
    uint16_t value;
    memcpy(&value, context.bytes + offset, sizeof(value));
    return value;
}

static void write32(unsigned offset, uint32_t value) {
    memcpy(context.bytes + offset, &value, sizeof(value));
}

static uint32_t read32(unsigned offset) {
    uint32_t value;
    memcpy(&value, context.bytes + offset, sizeof(value));
    return value;
}

void *func_80026648(void *actor, void *unused) {
    (void)unused;
    assert((uint32_t)(uintptr_t)actor == current.delta);
    assert(output.calls++ == 0);
    return context.bytes;
}

void func_8002E2DC(void) {
    assert(0);
}

int32_t func_8004FF50(void *actor, void *value, GroundedEventRecord **event) {
    assert(output.calls++ == 1);
    assert(actor == (void *)func_8002E2DC && (uintptr_t)value == 600);
    assert(event == &g_result_expiration_event && *event == NULL);
    output.scheduled_selection = (uint16_t)g_result_expiration_selection;
    *event = &event_record;
    if (current.mutation) write16(0x14, (uint16_t)current.replacement);
    {
        int32_t status;
        memcpy(&status, &current.status, sizeof(status));
        return status;
    }
}

void func_8002BB64(int32_t chosen) {
    assert(output.calls == 1 || output.calls == 2);
    output.calls++;
    output.chosen = (uint32_t)chosen;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t delta;
    current = *input;
    memset(&output, 0, sizeof(output));
    memset(&context, 0, sizeof(context));
    write16(0x14, (uint16_t)input->selection);
    write32(0x28, input->first);
    write32(0x3C, input->second);
    g_result_expiration_event = input->event ? &event_record : NULL;
    g_result_expiration_selection = -1234;
    memcpy(&delta, &input->delta, sizeof(delta));
    func_8002E348(delta);
    output.selection = read16(0x14);
    output.first = read32(0x28);
    output.second = read32(0x3C);
    output.event = g_result_expiration_event != NULL;
    output.expiration_selection = (uint16_t)g_result_expiration_selection;
    return output;
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t read_count;
    if (argc == 1) {
        memset(&input, 0, sizeof(input));
        input.first = 5;
        input.second = 1;
        input.delta = 0xFFFFFFFFU;
        input.mutation = 1;
        input.replacement = 1;
        result = run_probe(&input);
        assert(result.second == 0 && result.first == 5 && result.calls == 3);
        assert(result.selection == 1 && result.chosen == 10 && result.event == 1);
        puts("secondary result counter tests passed");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) return 2;
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
        perror("set binary mode");
        return 2;
    }
#endif
    while ((read_count = fread(&input, 1, sizeof(input), stdin)) != 0) {
        if (read_count != sizeof(input)) {
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
