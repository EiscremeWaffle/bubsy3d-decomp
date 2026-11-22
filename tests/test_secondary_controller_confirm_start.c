#include "secondary_controller_confirm.h"
#include "secondary_controller_helpers.h"
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

typedef struct ProbeInput {
    uint32_t selection, mutations, initial_flag, basis[8];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t outcome, selection, flag, marker, event, basis[8], count, calls[10];
} ProbeOutput;

volatile uint8_t g_secondary_controller_flag_505;
ComponentRotationMatrix g_secondary_controller_basis;
ControllerScreenPosition g_secondary_controller_confirm_position;
GroundedEventRecord *g_secondary_controller_confirm_event;
static ResultCounterContext context;
static ComponentRotationMatrix reference;
static ProbeInput current;
static ProbeOutput output;

static void log_call(uint32_t call) {
    assert(output.count < 10);
    output.calls[output.count++] = call;
}

void func_8002E348(int32_t delta) {
    assert(delta == -1);
    log_call(1);
    if (current.mutations & 1) context.selection = 0;
}

int32_t func_800516F8(void **result) {
    assert(g_secondary_controller_flag_505 == 1);
    log_call(2);
    *result = &reference;
    if (current.mutations & 2) context.selection = 1;
    return -1;
}

int32_t func_80055D94(ControllerScreenPosition *position, uint32_t x, uint32_t y) {
    assert(position == &g_secondary_controller_confirm_position && x == 256 && y == 240);
    assert(memcmp(&g_secondary_controller_basis, &reference, sizeof(reference)) == 0);
    log_call(3);
    if (current.mutations & 4) context.selection = 0;
    return -1;
}

void func_800493CC(void) {
    assert(0);
}

int32_t func_8004FF50(void *actor, void *value, GroundedEventRecord **record_out) {
    assert(actor == (void *)func_800493CC && value == (void *)(uintptr_t)60);
    assert(record_out == &g_secondary_controller_confirm_event);
    assert(context.unknown[0x0D] == 0);
    log_call(4);
    *record_out = (GroundedEventRecord *)(uintptr_t)0x1234;
    if (current.mutations & 8) g_secondary_controller_flag_505 = 0;
    return -1;
}

void func_8002C708(int16_t index) {
    assert(context.unknown[0x0D] == 1);
    log_call(0x100u + (uint16_t)index);
    if (index == 6 && (current.mutations & 16)) context.selection = 1;
}

void func_8002BAD4(int16_t index) {
    log_call(0x200u + (uint16_t)index);
    if (current.mutations & 32) context.selection = 0;
}

void func_8002CA88(int32_t index, int32_t duration) {
    assert(index == 5 && duration == 10);
    log_call(5);
}

int32_t func_8001C6BC(int32_t index, int32_t state, int32_t value) {
    assert(index == 6 && state == 2 && value == 1);
    log_call(6);
    if (current.mutations & 64) g_secondary_controller_flag_505 = 0;
    return -1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    memset(&context, 0, sizeof(context));
    memset(&output, 0, sizeof(output));
    memset(&g_secondary_controller_basis, 0, sizeof(g_secondary_controller_basis));
    current = *input;
    context.selection = (int16_t)input->selection;
    g_secondary_controller_flag_505 = (uint8_t)input->initial_flag;
    g_secondary_controller_confirm_event = 0;
    memcpy(&reference, input->basis, sizeof(reference));
    output.outcome = secondary_controller_confirm_start(&context);
    output.selection = (uint16_t)context.selection;
    output.flag = g_secondary_controller_flag_505;
    output.marker = context.unknown[0x0D];
    output.event = g_secondary_controller_confirm_event != 0;
    memcpy(output.basis, &g_secondary_controller_basis, sizeof(output.basis));
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
            ProbeOutput result;
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
        ProbeInput input = {0, 0, 0, {1, 2, 3, 4, 5, 6, 7, 8}};
        ProbeOutput result = run_probe(&input);
        assert(result.outcome == SECONDARY_CONFIRM_ACTIVE && result.count == 10);
        assert(result.calls[6] == 0x208 && result.calls[7] == 0x20B);
        input.selection = 1;
        input.mutations = 32 | 64;
        result = run_probe(&input);
        assert(result.outcome == SECONDARY_CONFIRM_EXIT);
        assert(result.calls[6] == 0x209 && result.calls[7] == 0x20A);
    }
    puts("secondary controller confirmation start tests passed");
    return 0;
}
