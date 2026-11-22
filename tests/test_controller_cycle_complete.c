#include "controller_cycle_warp.h"
#include "game_transition.h"
#include "heading_common_gates.h"
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
    uint32_t flags, state, lock, mode, stop, unknown, deceleration, active, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t flags, state, lock, mode, stop, unknown, deceleration, active, count, calls[6];
    uint32_t parameters[8];
} ProbeOutput;

static ControllerCycleActor actor;
static ProbeInput current;
static ProbeOutput output;
uint8_t g_heading_lock_6479, g_cycle_mode_flag;
uint8_t g_deceleration_stop_649C, g_cycle_unknown_6460;
uint8_t g_deceleration_state_6464, g_deceleration_active_64B8;
#ifdef CYCLE_REAL_RESET
static const uint32_t initial_parameters[8] = {0, 1, 2, 3, 0x80000000u, 5, 6, 0xFFFFFFFFu};
const uint32_t *g_level_event_parameters = initial_parameters;
uint32_t g_event_parameters[8];
#endif

extern void func_80038368(void);

static void record(uint32_t call) {
    assert(output.count < 6);
    output.calls[output.count++] = call;
}

void *func_80028764(int32_t index) {
    assert(index == 0);
    record(1);
    return &actor;
}

#ifndef CYCLE_REAL_RESET
void func_80028E80(void) {
    record(2);
    if (current.mutations & 1) g_cycle_mode_flag ^= 1;
}
#endif

int32_t func_8001C700(int32_t index, int32_t state) {
    assert(index == 4 && state == (g_cycle_mode_flag ? 14 : 15));
    record(3);
    if (current.mutations & 2) g_heading_lock_6479 = 99;
    return 0;
}

void func_8001B558(int32_t event, int32_t mode, uintptr_t flags, int32_t heading,
                   void *a4, void *a5, void *a6, void *a7) {
    assert(event == 0x2e && mode == 4 && flags == 0 && heading == 0);
    assert(a4 == NULL && a5 == NULL && a6 == NULL && a7 == NULL && g_heading_lock_6479 == 2);
    record(4);
    if (current.mutations & 4) {
        actor.flags_10 ^= 0x80808080u;
        g_deceleration_stop_649C ^= 1;
    }
}

int32_t func_8002269C(void *value, uint8_t selector) {
    assert(value == &actor && selector == 0x10);
    record(5);
    if (current.mutations & 8) {
        actor.unknown_01[0] ^= 0x80;
        g_cycle_unknown_6460 ^= 1;
    }
    return 0;
}

void func_8003539C(void *value, int32_t mode) {
    assert(value == &actor && mode == 1 && actor.flags_10 == 0 && actor.unknown_01[0] == 2);
    assert(g_deceleration_stop_649C == 0 && g_cycle_unknown_6460 == 0 &&
           g_deceleration_state_6464 == 0 && g_deceleration_active_64B8 == 0);
    record(6);
    if (current.mutations & 16) {
        actor.flags_10 = 0x12345678;
        g_deceleration_active_64B8 = 7;
    }
}

static ProbeOutput run_probe(const ProbeInput *input) {
    memset(&actor, 0, sizeof(actor));
    memset(&output, 0, sizeof(output));
    current = *input;
    actor.flags_10 = input->flags;
    actor.unknown_01[0] = (uint8_t)input->state;
    g_heading_lock_6479 = (uint8_t)input->lock;
    g_cycle_mode_flag = (uint8_t)input->mode;
    g_deceleration_stop_649C = (uint8_t)input->stop;
    g_cycle_unknown_6460 = (uint8_t)input->unknown;
    g_deceleration_state_6464 = (uint8_t)input->deceleration;
    g_deceleration_active_64B8 = (uint8_t)input->active;
#ifdef CYCLE_REAL_RESET
    memset(g_event_parameters, 0xA5, sizeof(g_event_parameters));
#endif
    func_80038368();
    output.flags = actor.flags_10;
    output.state = actor.unknown_01[0];
    output.lock = g_heading_lock_6479;
    output.mode = g_cycle_mode_flag;
    output.stop = g_deceleration_stop_649C;
    output.unknown = g_cycle_unknown_6460;
    output.deceleration = g_deceleration_state_6464;
    output.active = g_deceleration_active_64B8;
#ifdef CYCLE_REAL_RESET
    memcpy(output.parameters, g_event_parameters, sizeof(output.parameters));
#endif
    return output;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--binary") == 0) {
        ProbeInput input;
        size_t bytes;
#ifdef _WIN32
        if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
            perror("set binary mode");
            return 2;
        }
#endif
        while ((bytes = fread(&input, 1, sizeof(input), stdin)) != 0) {
            ProbeOutput result;
            if (bytes != sizeof(input)) {
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
    if (argc != 1) return 2;
    {
        ProbeInput input = {123, 4, 3, 0, 1, 1, 1, 1, 16};
        ProbeOutput result = run_probe(&input);
        assert(result.flags == 0x12345678 && result.state == 2 && result.active == 7);
        assert(result.lock == 2);
#ifdef CYCLE_REAL_RESET
        assert(result.count == 5 && result.parameters[4] == 0x80000000u);
#else
        assert(result.count == 6);
#endif
    }
    puts("controller cycle completion tests passed");
    return 0;
}
