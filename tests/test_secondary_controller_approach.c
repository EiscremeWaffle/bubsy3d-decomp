#include "secondary_controller_approach.h"
#include "secondary_controller_entry.h"
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
    uint32_t position[4], direction[3], sample[3], reset, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t position[4], offset[3], before_sample[3], component, reset, direction[3], handle;
    uint32_t actor_handle, info, iteration, branch, count, calls[8];
} ProbeOutput;

#ifdef SECONDARY_APPROACH_STEP_PROBE
typedef struct StepInput {
    int32_t base[4], position[4], step[3], iteration;
    uint32_t mutations;
} StepInput;

typedef struct StepOutput {
    int32_t base[4], position[4], step[3];
    uint32_t count, calls[2];
} StepOutput;

static StepInput *active_step;
static StepOutput step_output;
static ComponentRotationMatrix anchor;
#endif

SecondaryControllerApproachActor g_secondary_controller_approach_actor;
MovePreparationInfo g_secondary_controller_approach_move_info;
void *g_extra_model_handle;
static MovePreparationComponent handles[2];
static ComponentRotationMatrix components[2];
static MovePreparationInfo alternate_info;
static int32_t direction[3];
static int32_t *saved_offset;
static SecondaryControllerApproach *active_approach;
static ProbeInput current;
static ProbeOutput output;
static unsigned multiplies;

static void xor_word(int32_t *value, uint32_t mask) {
    uint32_t bits;
    memcpy(&bits, value, sizeof(bits));
    bits ^= mask;
    memcpy(value, &bits, sizeof(bits));
}

#ifndef SECONDARY_APPROACH_REAL_MULTIPLY
int32_t func_8001007C(int32_t left, int32_t right, int32_t *result) {
    uint32_t bits = (uint32_t)left ^ (uint32_t)right ^ (0x12340000u + multiplies);
    assert(multiplies < 3 && right == 672);
    if (multiplies == 0) assert(g_secondary_controller_approach_actor.reset == 0);
    output.calls[output.count++] = 1 + multiplies;
    memcpy(result, &bits, sizeof(bits));
    if (multiplies == 2) saved_offset = result;
    if (multiplies++ == 0) {
        if (current.mutations & 1) xor_word(&direction[1], 0x01010101u);
        if (current.mutations & 2) g_extra_model_handle = &handles[1];
        if (current.mutations & 16) g_secondary_controller_approach_actor.reset = 0x12345678;
    }
    return -1;
}
#endif

int32_t func_80054C7C(
    ComponentRotationMatrix *component, const ComponentRotationMatrix *basis, const int32_t position[3]
) {
#ifdef SECONDARY_APPROACH_STEP_PROBE
    if (active_step != NULL) {
        assert(basis == NULL && step_output.count < 2);
        if (component == &anchor) {
            assert(position == active_step->base);
            step_output.calls[step_output.count++] = 1;
            if (active_step->mutations & 1) xor_word(&active_step->step[1], 0x80000000u);
            if (active_step->mutations & 2) xor_word(&active_step->position[0], 0x10101010u);
            if (active_step->mutations & 4) xor_word(&active_step->base[1], 0x04040404u);
        } else {
            assert(component == &components[0] && position == active_step->position);
            step_output.calls[step_output.count++] = 2;
            if (active_step->mutations & 8) xor_word(&active_step->position[2], 0x80808080u);
        }
        return -1;
    }
#endif
    assert(component == &components[0] && basis == NULL);
    output.calls[output.count++] = 4;
    if (current.mutations & 4) {
        xor_word((int32_t *)&position[1], 0x02020202u);
#ifdef SECONDARY_APPROACH_REAL_MULTIPLY
        saved_offset = &active_approach->offset[2];
#endif
        xor_word(saved_offset, 0x04040404u);
    }
    return -1;
}

int32_t func_8005536C(
    ComponentRotationMatrix *component, const ComponentRotationMatrix *basis, const int32_t position[3]
) {
    assert(component == &components[0] && basis == NULL);
    output.calls[output.count++] = 5;
    memcpy(output.before_sample, position, sizeof(output.before_sample));
    if (current.mutations & 8) {
        xor_word((int32_t *)&position[0], 0x08080808u);
        g_secondary_controller_approach_actor.reset ^= 0xA5A5A5A5u;
    }
    return -1;
}

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t result[3]
) {
    assert((const void *)component == &components[0] && basis == NULL);
    output.calls[output.count++] = 6;
    memcpy(result, current.sample, sizeof(current.sample));
    return -1;
}

int32_t func_80052BDC(void *handle, int32_t mode) {
    assert(handle == g_extra_model_handle && mode == 0);
    output.calls[output.count++] = 7;
    if (current.mutations & 32) g_extra_model_handle = &handles[1];
    return -1;
}

uint8_t func_800222EC(MovePreparationInfo **info, const MovePreparationVector *position) {
    assert(info == &g_secondary_controller_approach_actor.info_24);
    assert(*info == &g_secondary_controller_approach_move_info);
    assert(g_secondary_controller_approach_actor.handle_18 == g_extra_model_handle);
    output.calls[output.count++] = 8;
    if (current.mutations & 64) xor_word((int32_t *)&position->y, 0x80000000u);
    if (current.mutations & 128) *info = &alternate_info;
    return 0;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t position[4];
    SecondaryControllerApproach approach;
    memset(&output, 0, sizeof(output));
    memset(&approach, 0, sizeof(approach));
    memset(handles, 0, sizeof(handles));
    current = *input;
    multiplies = 0;
    memcpy(position, input->position, sizeof(position));
    memcpy(direction, input->direction, sizeof(direction));
    handles[0].transform_14 = &components[0];
    handles[1].transform_14 = &components[1];
    g_extra_model_handle = &handles[0];
    memset(&g_secondary_controller_approach_actor, 0, sizeof(g_secondary_controller_approach_actor));
    g_secondary_controller_approach_actor.reset = input->reset;
    active_approach = &approach;
    output.branch = (uint32_t)secondary_controller_approach(position, direction, &approach);
    memcpy(output.position, approach.position.words, sizeof(output.position));
    memcpy(output.offset, approach.offset, sizeof(output.offset));
    memcpy(output.direction, direction, sizeof(output.direction));
    output.component = approach.component == &components[0] ? 1 : approach.component == &components[1] ? 2 : 0;
    output.handle = g_extra_model_handle == &handles[0] ? 1 : g_extra_model_handle == &handles[1] ? 2 : 0;
    output.reset = g_secondary_controller_approach_actor.reset;
    output.actor_handle = g_secondary_controller_approach_actor.handle_18 == &handles[0] ? 1 :
                          g_secondary_controller_approach_actor.handle_18 == &handles[1] ? 2 : 0;
    output.info = g_secondary_controller_approach_actor.info_24 == &g_secondary_controller_approach_move_info ? 1 :
                  g_secondary_controller_approach_actor.info_24 == &alternate_info ? 2 : 0;
    output.iteration = approach.iteration;
    return output;
}

#ifdef SECONDARY_APPROACH_STEP_PROBE
static StepOutput run_step_probe(const StepInput *input) {
    StepInput state = *input;
    memset(&step_output, 0, sizeof(step_output));
    active_step = &state;
    secondary_controller_approach_step(state.iteration, state.base, state.position, state.step,
                                       &anchor, &components[0]);
    active_step = NULL;
    memcpy(step_output.base, state.base, sizeof(state.base));
    memcpy(step_output.position, state.position, sizeof(state.position));
    memcpy(step_output.step, state.step, sizeof(state.step));
    return step_output;
}
#endif

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--binary") == 0) {
#ifdef SECONDARY_APPROACH_STEP_PROBE
        StepInput input;
#else
        ProbeInput input;
#endif
        size_t bytes_read;
#ifdef _WIN32
        if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
            perror("set binary mode");
            return 2;
        }
#endif
        while ((bytes_read = fread(&input, 1, sizeof(input), stdin)) != 0) {
#ifdef SECONDARY_APPROACH_STEP_PROBE
            StepOutput result;
#else
            ProbeOutput result;
#endif
            if (bytes_read != sizeof(input)) {
                fputs("incomplete probe input\n", stderr);
                return 2;
            }
#ifdef SECONDARY_APPROACH_STEP_PROBE
            result = run_step_probe(&input);
#else
            result = run_probe(&input);
#endif
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
#ifdef SECONDARY_APPROACH_STEP_PROBE
    {
        StepInput input = {{0, 159, 0, 77}, {0, 0, 0, 88}, {1, 0, 3}, 19, 1};
        StepOutput result = run_step_probe(&input);
        assert(result.count == 2 && result.calls[0] == 1 && result.calls[1] == 2);
        assert(result.base[1] == 160 && result.position[0] == 1);
        assert((uint32_t)result.position[1] == 0x80000000u && result.position[3] == 88);
    }
#endif
    {
        ProbeInput input = {{0xFFFFFFFFu, 0x7FFFFFFFu, 0, 0x12345678u}, {0, 1, 2}, {7, 8, 9}, 9, 2};
#ifdef SECONDARY_APPROACH_REAL_MULTIPLY
        input.mutations = 32;
#endif
        ProbeOutput result = run_probe(&input);
        assert(result.component == 1 && result.handle == 2 && result.actor_handle == 2);
        assert(result.reset == 0 && result.branch == 1 && result.iteration == 0);
#ifdef SECONDARY_APPROACH_REAL_MULTIPLY
        assert(result.count == 5 && result.before_sample[0] == 0xFFFFFFFFu);
#else
        assert(result.count == 8 && result.before_sample[0] == 0x1234029Fu);
#endif
        assert(result.position[3] == 0x12345678u);
    }
    puts("secondary controller approach tests passed");
    return 0;
}
