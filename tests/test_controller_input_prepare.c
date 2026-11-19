#include "controller_input_prepare.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

typedef struct ProbeInput {
    uint32_t buttons, status_1, status_2, latched_1, latched_2, second_port, mirror_extra;
} ProbeInput;

typedef struct ProbeOutput {
    uint8_t record[28];
    uint32_t latched_1, latched_2;
} ProbeOutput;

_Static_assert(sizeof(GameTransitionEventRecord) == 28, "event record size");
_Static_assert(offsetof(GameTransitionEventRecord, payload.input.buttons) == 0x0C, "button offset");
_Static_assert(offsetof(GameTransitionEventRecord, payload.input.digital_pad_1) == 0x10, "port 1 offset");
_Static_assert(offsetof(GameTransitionEventRecord, payload.input.digital_pad_2) == 0x11, "port 2 offset");
_Static_assert(sizeof(ProbeInput) == 7 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 9 * sizeof(uint32_t), "probe output layout");

static ProbeOutput run_probe(const ProbeInput *input) {
    GameTransitionEventRecord event;
    ControllerInputPrepareState state;
    ProbeOutput output;

    memset(&event, 0xA5, sizeof(event));
    event.payload.input.buttons = input->buttons;
    event.payload.input.digital_pad_1 = (uint8_t)input->status_1;
    event.payload.input.digital_pad_2 = (uint8_t)input->status_2;
    state.port_1_latched = (uint8_t)input->latched_1;
    state.port_2_latched = (uint8_t)input->latched_2;
    state.second_port_enabled = (uint8_t)input->second_port;
    state.mirror_extra_buttons = (uint8_t)input->mirror_extra;
    controller_prepare_input_event(&event, &state);
    memcpy(output.record, &event, sizeof(output.record));
    output.latched_1 = state.port_1_latched;
    output.latched_2 = state.port_2_latched;
    return output;
}

static uint32_t output_buttons(const ProbeOutput *output) {
    uint32_t buttons;
    memcpy(&buttons, &output->record[0x0C], sizeof(buttons));
    return buttons;
}

static void self_test(void) {
    ProbeInput input = {0};
    ProbeOutput output;

    input.buttons = 0xFFFFFFFFU;
    input.latched_1 = 2;
    output = run_probe(&input);
    assert(output.latched_1 == 0 && output_buttons(&output) == ((1U << 11) | (1U << 27)));

    input.buttons = (1U << 24) | (1U << 27) | (1U << 28) | (1U << 30) | (1U << 22);
    input.status_1 = input.status_2 = 1;
    input.latched_1 = input.latched_2 = 0;
    input.second_port = input.mirror_extra = 1;
    output = run_probe(&input);
    assert(output.latched_1 == 1 && output.latched_2 == 1);
    assert(output_buttons(&output) == (input.buttons | (1U << 8) | (1U << 11) |
                                       (1U << 12) | (1U << 14) | (1U << 6)));

    input.buttons = (1U << 8) | (1U << 11);
    input.second_port = input.mirror_extra = 0;
    input.latched_2 = 7;
    output = run_probe(&input);
    assert(output.latched_2 == 7);
    assert(output_buttons(&output) == (input.buttons | (1U << 24) | (1U << 27)));
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("controller input-preparation tests passed");
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
        ProbeOutput output;
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
