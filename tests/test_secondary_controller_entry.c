#include "secondary_controller_entry.h"
#include "component_records.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

typedef struct ProbeInput {
    uint32_t buttons, initialized, timer, duration, mode, flag, selection, mutations, seed;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t outcome, initialized, timer, flag, owner_status, blocked, mode, heading, context_ok;
    uint32_t call_count, calls[16], record[8], read_owner, write_owner;
} ProbeOutput;

uint8_t g_secondary_controller_initialized;
int16_t g_secondary_controller_timer;
uint8_t g_secondary_controller_flag_506;
void *g_extra_model_handle;
const char g_secondary_controller_timeout_text[] = "timeout";

static ProbeInput current;
static ProbeOutput output;
static ControllerGameContext context;
static SecondaryControllerOwner actor_owner;
static SecondaryControllerActor actor;
static ComponentRecordOwner first_owner, second_owner;

static void record(unsigned call) {
    assert(output.call_count < 16);
    output.calls[output.call_count++] = call;
}

void *func_80026648(void *actor_argument, void *context_argument) {
    assert((uintptr_t)actor_argument == current.buttons);
    assert(context_argument == 0);
    record(1);
    return &context;
}

void func_8002CA4C(int32_t index, int32_t duration) {
    assert(index == 17 && duration == 240);
    record(2);
    if (current.mutations & 1) g_secondary_controller_timer = 89;
}

int32_t func_80052094(ComponentRecordOwner *owner, uint32_t index, uint32_t *record_data) {
    unsigned i;
    assert(index == 0 && g_secondary_controller_initialized == 1);
    output.read_owner = owner == &first_owner ? 1 : 2;
    assert(owner == &first_owner || owner == &second_owner);
    record(3);
    for (i = 0; i < 8; ++i) record_data[i] = current.seed + i;
    if (current.mutations & 2) g_extra_model_handle = &second_owner;
    return -1;
}

int32_t func_800528A4(ComponentRecordOwner *owner, uint32_t index, const uint32_t *record_data) {
    assert(index == 0);
    output.write_owner = owner == &first_owner ? 1 : 2;
    assert(owner == &first_owner || owner == &second_owner);
    memcpy(output.record, record_data, sizeof(output.record));
    record(4);
    return -1;
}

void *func_80028764(int32_t index) { assert(index == 0); record(5); return &actor; }
uint32_t func_8004753C(void) { record(6); return current.selection; }
void func_8001B558(
    int32_t id, int32_t mode, uintptr_t flags, int32_t heading,
    void *a4, void *a5, void *a6, void *a7
) {
    assert(id == 23 && mode == 5 && flags == 0);
    assert(a4 == 0 && a5 == 0 && a6 == 0 && a7 == 0);
    assert(actor_owner.status_20 == 0);
    output.heading = (uint32_t)heading;
    record(7);
}
void func_8001BC28(void) { record(8); }
int32_t func_8002BD0C(int32_t index) {
    assert(index == 17);
    record(9);
    if (current.mutations & 4) g_secondary_controller_flag_506 = 1;
    return (int32_t)current.duration;
}
int32_t func_8003AA1C(void **result) {
    *result = &context;
    record(10);
    return 0;
}
void func_8002DF24(const char *text) {
    assert(text == g_secondary_controller_timeout_text);
    assert(context.blocked_2b == 1 && context.mode_6d == 2);
    assert(g_secondary_controller_timer == 0);
    record(11);
}
void func_8001BC08(void) { record(12); }
uint8_t func_8001BCE4(void) {
    record(13);
    if (current.mutations & 8) g_secondary_controller_flag_506 = 1;
    return (uint8_t)current.mode;
}
uint32_t func_80056678(uint32_t buttons, uint32_t bit) {
    assert(buttons == current.buttons && bit == 7);
    record(14);
    if (current.mutations & 16) g_secondary_controller_flag_506 = 2;
    return (buttons >> bit) & 1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    void *result_context = 0;
    current = *input;
    memset(&output, 0, sizeof(output));
    memset(&context, 0, sizeof(context));
    context.blocked_2b = 0x55;
    context.mode_6d = 0x66;
    actor_owner.status_20 = 0xabcdef01;
    actor.owner_1c = &actor_owner;
    g_extra_model_handle = &first_owner;
    g_secondary_controller_initialized = (uint8_t)current.initialized;
    g_secondary_controller_timer = (int16_t)(uint16_t)current.timer;
    g_secondary_controller_flag_506 = (uint8_t)current.flag;
    output.heading = UINT32_MAX;
    output.outcome = secondary_controller_entry(current.buttons, &result_context);
    output.context_ok = result_context == &context;
    output.initialized = g_secondary_controller_initialized;
    output.timer = (uint16_t)g_secondary_controller_timer;
    output.flag = g_secondary_controller_flag_506;
    output.owner_status = actor_owner.status_20;
    output.blocked = context.blocked_2b;
    output.mode = context.mode_6d;
    return output;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--binary") == 0) {
        ProbeInput input;
        size_t bytes_read;
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
            ProbeOutput result = run_probe(&input);
            if (fwrite(&result, sizeof(result), 1, stdout) != 1) {
                perror("write probe output");
                return 2;
            }
        }
        if (ferror(stdin)) {
            perror("read probe input");
            return 2;
        }
        return 0;
    }
    if (argc != 1) {
        fputs("expected --binary or no arguments\n", stderr);
        return 2;
    }
    {
        ProbeInput input = {0, 0, 89, 0, 0, 0, 42, 0, 10};
        ProbeOutput result = run_probe(&input);
        assert(result.outcome == SECONDARY_CONTROLLER_ENTRY_EXIT);
        assert(result.timer == 90 && result.owner_status == 0 && result.heading == 42);
        input.initialized = 1;
        input.timer = 0xffff;
        input.duration = 1;
        input.flag = 1;
        result = run_probe(&input);
        assert(result.outcome == SECONDARY_CONTROLLER_ENTRY_CONTINUE && result.flag == 0);
    }
    puts("secondary controller entry tests passed");
    return 0;
}
