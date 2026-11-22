#include "secondary_controller_confirm.h"
#include "controller_bitmap_gate.h"
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
    uint32_t buttons, flag, selection, slots[2], mode, response, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t outcome, flag, selection, slots[2], mode, count, calls[3];
} ProbeOutput;

volatile uint8_t g_secondary_controller_flag_505;
static ResultCounterContext context;
static ControllerGameContext game;
static ProbeInput current;
static ProbeOutput output;

uint32_t func_80056678(uint32_t value, uint32_t selector) {
    assert(value == current.buttons && selector == 7);
    output.calls[output.count++] = 1;
    if (current.mutations & 1) g_secondary_controller_flag_505 ^= 1;
    return current.response;
}

int32_t func_8003AA1C(void **result) {
    output.calls[output.count++] = 2;
    if (!(current.mutations & 16)) *result = &game;
    if (current.mutations & 2) context.selection = 0;
    if (current.mutations & 4) context.slots[current.selection == 0].value ^= 1;
    if (current.mutations & 8) game.unknown_00[0x11] ^= 1;
    return -1;
}

int32_t func_8001C6BC(int32_t index, int32_t state, int32_t value) {
    assert(index == 4 && state == 5 && value == 1);
    output.calls[output.count++] = 3;
    return -1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    memset(&context, 0, sizeof(context));
    memset(&game, 0, sizeof(game));
    memset(&output, 0, sizeof(output));
    current = *input;
    g_secondary_controller_flag_505 = (uint8_t)input->flag;
    context.selection = (int16_t)input->selection;
    memcpy(&context.slots[0].value, &input->slots[0], sizeof(input->slots[0]));
    memcpy(&context.slots[1].value, &input->slots[1], sizeof(input->slots[1]));
    game.unknown_00[0x11] = (uint8_t)input->mode;
    output.outcome = (uint32_t)secondary_controller_confirm_gate(input->buttons, &context);
    output.flag = g_secondary_controller_flag_505;
    output.selection = (uint16_t)context.selection;
    memcpy(&output.slots[0], &context.slots[0].value, sizeof(output.slots[0]));
    memcpy(&output.slots[1], &context.slots[1].value, sizeof(output.slots[1]));
    output.mode = game.unknown_00[0x11];
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
        ProbeInput input = {0x80, 0, 0, {0, 1}, 4, 1, 0};
        ProbeOutput result = run_probe(&input);
        assert(result.outcome == SECONDARY_CONFIRM_START && result.count == 2);
        input.mutations = 4;
        result = run_probe(&input);
        assert(result.outcome == SECONDARY_CONFIRM_EXIT && result.count == 3);
        input.flag = 1;
        result = run_probe(&input);
        assert(result.outcome == SECONDARY_CONFIRM_ACTIVE && result.count == 1);
        input.response = 0x100;
        input.mutations = 1;
        result = run_probe(&input);
        assert(result.outcome == SECONDARY_CONFIRM_EXIT && result.count == 1);
        input.flag = 0;
        input.response = 1;
        input.slots[1] = 0;
        input.mutations = 16;
        result = run_probe(&input);
        assert(result.outcome == SECONDARY_CONFIRM_EXIT && result.count == 3);
    }
    puts("secondary controller confirmation tests passed");
    return 0;
}
