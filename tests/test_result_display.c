#include "result_display.h"

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
    MUTATE_CREATE_TEXT = 1 << 0,
    MUTATE_SCALE = 1 << 1,
    MUTATE_REMOVE_TEXT = 1 << 2,
    MUTATE_RELEASE_TEXT = 1 << 3,
    MUTATE_REMOVE_EVENT = 1 << 4,
    MUTATE_CREATE_EVENT = 1 << 5
};

typedef struct ProbeInput {
    uint32_t operation, phase, scale, tag, event, text, mutations;
    uint32_t replacement_scale, replacement_phase, replacement_tag, replacement_event;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t phase, scale, tag, event, count, calls[5], arguments[5][3];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 11 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 25 * sizeof(uint32_t), "probe output layout");

uint8_t g_result_display_phase;
int32_t g_result_display_scale;
int8_t g_result_display_tag;
GroundedEventRecord *g_result_display_event;
static ProbeInput current;
static ProbeOutput output;

static void record_call(uint32_t id, uint32_t first, uint32_t second, uint32_t third) {
    unsigned at = output.count++;
    assert(at < 5);
    output.calls[at] = id;
    output.arguments[at][0] = first;
    output.arguments[at][1] = second;
    output.arguments[at][2] = third;
}

void func_800486D0(void *context, int32_t first, int32_t second, int8_t *tag) {
    assert(tag == &g_result_display_tag);
    record_call(1, (uint32_t)(uintptr_t)context, (uint32_t)first, (uint32_t)second);
    if (current.mutations & MUTATE_CREATE_TEXT) {
        *tag = (int8_t)current.replacement_tag;
        g_result_display_phase = (uint8_t)current.replacement_phase;
        g_result_display_scale = (int32_t)current.replacement_scale;
    }
}

int32_t func_800481D4(int32_t tag, int32_t first, int32_t second) {
    record_call(2, (uint32_t)tag, (uint32_t)first, (uint32_t)second);
    if (current.mutations & MUTATE_SCALE) {
        g_result_display_tag = (int8_t)current.replacement_tag;
        g_result_display_phase = (uint8_t)current.replacement_phase;
        g_result_display_scale = (int32_t)current.replacement_scale;
    }
    return 0;
}

int32_t func_800483C0(int32_t tag) {
    record_call(3, (uint32_t)tag, 0, 0);
    return 0;
}

int32_t func_80048504(int32_t tag) {
    record_call(5, (uint32_t)tag, 0, 0);
    if (current.mutations & MUTATE_REMOVE_TEXT) {
        g_result_display_tag = (int8_t)(current.replacement_tag ^ 0x55U);
        g_result_display_event = (GroundedEventRecord *)(uintptr_t)current.replacement_event;
    }
    return 0;
}

int32_t func_80048C48(int32_t tag) {
    record_call(6, (uint32_t)tag, 0, 0);
    if (current.mutations & MUTATE_RELEASE_TEXT) {
        g_result_display_event = (GroundedEventRecord *)(uintptr_t)current.replacement_event;
    }
    return 0;
}

int32_t func_8004FF50(void *actor, void *value, GroundedEventRecord **event_out) {
    record_call(4, actor == (void *)func_8002DE34, (uint32_t)(uintptr_t)value,
                event_out == &g_result_display_event);
    if (current.mutations & MUTATE_CREATE_EVENT) {
        *event_out = (GroundedEventRecord *)(uintptr_t)current.replacement_event;
    }
    return 0;
}

int32_t func_8004FFEC(GroundedEventRecord *event) {
    record_call(7, (uint32_t)(uintptr_t)event, 0, 0);
    if (current.mutations & MUTATE_REMOVE_EVENT) {
        g_result_display_event = (GroundedEventRecord *)(uintptr_t)0xABCDEF01U;
        g_result_display_phase = (uint8_t)current.replacement_phase;
        g_result_display_scale = (int32_t)current.replacement_scale;
        g_result_display_tag = (int8_t)current.replacement_tag;
    }
    return 0;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    current = *input;
    memset(&output, 0, sizeof(output));
    g_result_display_phase = (uint8_t)input->phase;
    g_result_display_scale = (int32_t)input->scale;
    g_result_display_tag = (int8_t)input->tag;
    g_result_display_event = (GroundedEventRecord *)(uintptr_t)input->event;
    if (input->operation == 0) {
        func_8002DF24((const char *)(uintptr_t)input->text);
    } else {
        assert(input->operation == 1);
        func_8002DE34();
    }
    output.phase = g_result_display_phase;
    output.scale = (uint32_t)g_result_display_scale;
    output.tag = (uint32_t)(int32_t)g_result_display_tag;
    output.event = (uint32_t)(uintptr_t)g_result_display_event;
    return output;
}

static void self_test(void) {
    ProbeInput input = {0, 0, 99, 7, 0x1234, 0x8888, MUTATE_CREATE_TEXT | MUTATE_CREATE_EVENT,
                        123, 2, 0xFE, 0x5678};
    ProbeOutput result = run_probe(&input);

    assert(result.count == 4 && result.calls[0] == 1 && result.calls[3] == 4);
    assert(result.phase == 2 && result.scale == 123 && result.tag == 0xFFFFFFFEU);
    assert(result.event == 0x5678 && result.arguments[1][0] == 0xFFFFFFFEU);

    memset(&input, 0, sizeof(input));
    input.operation = 1;
    input.phase = 35;
    input.scale = 0x400;
    input.event = 0x1234;
    result = run_probe(&input);
    assert(result.count == 5 && result.calls[0] == 2 && result.calls[4] == 7);
    assert(result.phase == 0 && result.scale == 0x1000 && result.event == 0);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("result display tests passed");
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
