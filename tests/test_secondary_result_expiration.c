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
    uint32_t selection, first, second, expiration_selection, event, mutation, replacement, status;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t selection, first, second, expiration_selection, event, calls, chosen, removed, trace[8];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 8 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 16 * sizeof(uint32_t), "probe output layout");

extern void func_8002E2DC(void);
GroundedEventRecord *g_result_expiration_event;
int16_t g_result_expiration_selection;
static ResultCounterContext context;
static GroundedEventRecord event_record, replacement_event;
static ProbeInput current;
static ProbeOutput output;
static unsigned getters;

static void record(uint32_t value) {
    assert(output.calls < 8);
    output.trace[output.calls++] = value;
}

void *func_80026648(void *actor, void *unused) {
    (void)unused;
    assert((uintptr_t)actor == (getters == 0 ? 0U : 1U));
    record(1);
    if (getters++ == 1 && (current.mutation & 1U)) {
        context.selection = (int16_t)current.replacement;
    }
    return &context;
}

int32_t func_8004FF50(void *actor, void *value, GroundedEventRecord **event) {
    assert(actor == (void *)func_8002E2DC && (uintptr_t)value == 600);
    assert(event == &g_result_expiration_event && *event == NULL);
    record(2);
    *event = &event_record;
    return -1;
}

void func_8002BB64(int32_t value) {
    record(3);
    output.chosen = (uint32_t)value;
}

int32_t func_8004FFEC(GroundedEventRecord *event) {
    int32_t status;
    record(4);
    output.removed = event != NULL;
    if (current.mutation & 2U) g_result_expiration_event = &replacement_event;
    memcpy(&status, &current.status, sizeof(status));
    return status;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    current = *input;
    memset(&context, 0, sizeof(context));
    memset(&output, 0, sizeof(output));
    getters = 0;
    context.selection = (int16_t)input->selection;
    memcpy(&context.slots[0].value, &input->first, sizeof(input->first));
    memcpy(&context.slots[1].value, &input->second, sizeof(input->second));
    g_result_expiration_selection = (int16_t)input->expiration_selection;
    g_result_expiration_event = input->event ? &event_record : NULL;
    func_8002E2DC();
    output.selection = (uint16_t)context.selection;
    memcpy(&output.first, &context.slots[0].value, sizeof(output.first));
    memcpy(&output.second, &context.slots[1].value, sizeof(output.second));
    output.expiration_selection = (uint16_t)g_result_expiration_selection;
    output.event = g_result_expiration_event != NULL;
    return output;
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t read_count;
    if (argc == 1) {
        memset(&input, 0, sizeof(input));
        input.event = 1;
        input.mutation = 2;
        result = run_probe(&input);
        assert(result.second == 1 && result.removed == 1 && result.event == 0);
        assert(result.calls == 4 && result.chosen == 11);
        puts("secondary result expiration tests passed");
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
