#include "actor_context.h"
#include "controller_result.h"
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
    uint32_t argument0, argument1, flag_505, flag_507, context_byte, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t flag_505, flag_507, context_byte, argument0, argument1, call_count;
    uint32_t calls[6];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 6 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 12 * sizeof(uint32_t), "probe output layout");

volatile uint8_t g_secondary_controller_flag_505;
volatile uint8_t g_secondary_controller_flag_507;
uint8_t g_secondary_controller_display[4];
static ControllerResultContext context;
static uint32_t arguments[2];
static ProbeInput current;
static ProbeOutput output;

static void *argument_for(uint32_t value) {
    assert(value <= 2);
    return value == 0 ? 0 : &arguments[value - 1];
}

static uint32_t argument_id(void *value) {
    if (value == 0) {
        return 0;
    }
    if (value == &arguments[0]) {
        return 1;
    }
    assert(value == &arguments[1]);
    return 2;
}

static void record_call(uint32_t call) {
    assert(output.call_count < 6);
    output.calls[output.call_count++] = call;
}

void *func_80026648(void *argument0, void *argument1) {
    record_call(1);
    assert(argument_id(argument0) == current.argument0);
    assert(argument_id(argument1) == current.argument1);
    return &context;
}

void func_8002C370(void *argument0, void *argument1) {
    record_call(2);
    output.argument0 = argument_id(argument0);
    output.argument1 = argument_id(argument1);
    if (current.mutations & 1U) {
        g_secondary_controller_flag_505 = 0x11;
        g_secondary_controller_flag_507 = 0x22;
        context.unknown_0D[0] = 0x33;
    }
}

int32_t func_80050650(void *display) {
    record_call(3);
    assert(display == g_secondary_controller_display);
    if (current.mutations & 2U) {
        g_secondary_controller_flag_505 = 0x44;
        g_secondary_controller_flag_507 = 0x55;
        context.unknown_0D[0] = 0x66;
    }
    return -1;
}

void func_80028EA8(void) {
    record_call(4);
    assert(g_secondary_controller_flag_505 == 0 && g_secondary_controller_flag_507 == 0);
    assert(context.unknown_0D[0] == 0);
    if (current.mutations & 4U) {
        g_secondary_controller_flag_505 = 0xA1;
        g_secondary_controller_flag_507 = 0xA2;
        context.unknown_0D[0] = 0xA3;
    }
}

void func_8002CB2C(int32_t index) {
    record_call(5);
    assert(index == 5);
    if (current.mutations & 8U) {
        g_secondary_controller_flag_505 ^= 8U;
    }
}

void func_8002BAD4(int16_t index) {
    record_call(6);
    assert(index == 5);
    if (current.mutations & 16U) {
        g_secondary_controller_flag_507 ^= 16U;
        context.unknown_0D[0] ^= 16U;
    }
}

static ProbeOutput run_probe(const ProbeInput *input) {
    current = *input;
    memset(&context, 0, sizeof(context));
    memset(&output, 0, sizeof(output));
    g_secondary_controller_flag_505 = (uint8_t)input->flag_505;
    g_secondary_controller_flag_507 = (uint8_t)input->flag_507;
    context.unknown_0D[0] = (uint8_t)input->context_byte;
    func_800493F8(argument_for(input->argument0), argument_for(input->argument1));
    output.flag_505 = g_secondary_controller_flag_505;
    output.flag_507 = g_secondary_controller_flag_507;
    output.context_byte = context.unknown_0D[0];
    return output;
}

static void self_test(void) {
    ProbeInput input = {1, 2, 255, 255, 255, 31};
    ProbeOutput result = run_probe(&input);

    assert(result.argument0 == 1 && result.argument1 == 2 && result.call_count == 6);
    assert(result.flag_505 == (0xA1U ^ 8U));
    assert(result.flag_507 == (0xA2U ^ 16U) && result.context_byte == (0xA3U ^ 16U));
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("secondary-controller cleanup tests passed");
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
