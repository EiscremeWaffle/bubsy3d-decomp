#include "component_rotation.h"
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
    uint32_t tick;
    uint32_t tick_result;
    uint32_t flag;
    uint32_t result;
    uint32_t mutations;
    uint32_t argument;
    uint32_t reference[8];
    uint32_t basis[8];
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t tick;
    uint32_t tick_result;
    uint32_t flag;
    uint32_t call_count;
    uint32_t calls[12];
    uint32_t reference[8];
    uint32_t basis[8];
    uint32_t argument;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 22 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 33 * sizeof(uint32_t), "probe output layout");

uint32_t g_secondary_controller_tick;
uint8_t g_secondary_controller_tick_result;
volatile uint8_t g_secondary_controller_flag_505;
volatile uint8_t g_secondary_controller_flag_507;
ComponentRotationMatrix g_secondary_controller_basis;
uint8_t g_secondary_controller_display[4];

static ComponentRotationMatrix reference_matrix;
static unsigned char arguments[2][4];
static ProbeInput current;
static ProbeOutput output;
static uint32_t queue_calls;
static uint32_t dispatch_calls;

static void record(uint32_t call) {
    assert(output.call_count < sizeof(output.calls) / sizeof(output.calls[0]));
    output.calls[output.call_count++] = call;
}

static void mutate_first_reference_word(uint32_t mask) {
    uint32_t word;
    memcpy(&word, &reference_matrix, sizeof(word));
    word ^= mask;
    memcpy(&reference_matrix, &word, sizeof(word));
}

static void *argument_for(uint32_t id) {
    assert(id <= 2);
    return id == 0 ? 0 : arguments[id - 1];
}

static uint32_t argument_id(void *argument) {
    if (argument == 0) return 0;
    if (argument == arguments[0]) return 1;
    assert(argument == arguments[1]);
    return 2;
}

int32_t func_800516F8(void **destination) {
    record(1);
    *destination = &reference_matrix;
    return -1;
}

int32_t func_80032D7C(void) {
    record(2);
    queue_calls++;
    if (queue_calls == 1 && (current.mutations & 1U)) {
        mutate_first_reference_word(0x01020304U);
    }
    if (queue_calls == 2 && (current.mutations & 64U)) {
        mutate_first_reference_word(0xA5A55A5AU);
    }
    return -1;
}

uint32_t func_800439EC(void) {
    record(3);
    if (current.mutations & 2U) g_secondary_controller_flag_507 = 0x22;
    return current.result;
}

void func_80044268(void) {
    record(4);
}

void func_8001B0A0(void) {
    record(5);
    if (current.mutations & 4U) g_secondary_controller_flag_507 = 1;
}

int32_t func_80050534(void *payload) {
    record(6);
    assert(payload == g_secondary_controller_display);
    if (current.mutations & 8U) g_secondary_controller_flag_507 = 0;
    return -1;
}

int32_t func_80032C8C(int32_t mode) {
    record(7);
    assert(mode == 0);
    dispatch_calls++;
    if (dispatch_calls == 1 && (current.mutations & 16U)) {
        g_secondary_controller_flag_507 = 1;
    }
    if (dispatch_calls == 2 && (current.mutations & 128U)) {
        g_secondary_controller_flag_507 = 0x77;
    }
    return -1;
}

int32_t func_80051678(void) {
    record(8);
    return -1;
}

int32_t func_800509A4(int32_t mode, void *argument, int32_t flag) {
    record(9);
    assert(mode == 1 && flag == 1);
    output.argument = argument_id(argument);
    if (current.mutations & 32U) g_secondary_controller_flag_507 = 1;
    return -1;
}

int32_t func_80050650(void *payload) {
    record(10);
    assert(payload == g_secondary_controller_display);
    return -1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    current = *input;
    memset(&output, 0, sizeof(output));
    queue_calls = 0;
    dispatch_calls = 0;
    g_secondary_controller_tick = input->tick;
    g_secondary_controller_tick_result = (uint8_t)input->tick_result;
    g_secondary_controller_flag_507 = (uint8_t)input->flag;
    memcpy(&reference_matrix, input->reference, sizeof(reference_matrix));
    memcpy(&g_secondary_controller_basis, input->basis, sizeof(g_secondary_controller_basis));
    func_80049158(argument_for(input->argument));
    output.tick = g_secondary_controller_tick;
    output.tick_result = g_secondary_controller_tick_result;
    output.flag = g_secondary_controller_flag_507;
    memcpy(output.reference, &reference_matrix, sizeof(reference_matrix));
    memcpy(output.basis, &g_secondary_controller_basis, sizeof(g_secondary_controller_basis));
    return output;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput result;

    input.tick = 3;
    input.flag = 1;
    input.argument = 1;
    input.mutations = 255;
    input.result = 0x1234;
    input.reference[0] = 0x11223344;
    input.basis[0] = 0x55667788;
    result = run_probe(&input);
    assert(result.tick == 4 && result.tick_result == 0);
    assert(result.call_count == 11 && result.calls[0] == 1 && result.calls[10] == 7);
    assert(result.reference[0] == (input.reference[0] ^ 0x01020304U));
    assert(result.argument == 1);
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("secondary-controller basis swap tests passed");
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
