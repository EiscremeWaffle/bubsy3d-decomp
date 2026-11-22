#include "controller_cycle_warp.h"
#include "game_transition.h"
#ifdef CYCLE_MODEL_MOVEMENT
#include "cycle_movement_setup.h"
#include "queue_update.h"
#endif
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

typedef struct ProbeInput {
    uint32_t position[4], direction[3], sample[3];
    uint32_t flags, lock, mode, state, state04, heading;
    uint32_t level, height_offset, motion_scale, motion_base, distance, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t position[4], state, state04, saved_position[3], lock, effect;
    uint32_t shared_position[3], vectors[12], direction[3], motion[3], adjusted_y;
    uint32_t count, calls[18];
    uint32_t actor_flags, event_parameter;
#ifdef CYCLE_MODEL_MOVEMENT
    uint32_t movement_step[3], movement_adjustment[3], movement_duration;
    uint32_t movement_active, movement_mode, movement_callback;
#endif
} ProbeOutput;

GameCyclicEntry g_cycle_initial_vector;
uint32_t g_cycle_saved_position[3];
uint8_t g_heading_lock_6479, g_cycle_mode_flag;
uint16_t g_cycle_saved_effect;
int16_t g_cycle_level_id;
uint32_t g_cycle_height_offset, g_cycle_motion_scale;
int32_t g_cycle_motion_base;
uint32_t g_event_parameters[8];
#ifdef CYCLE_MODEL_MOVEMENT
uint8_t g_unknown_800BE024, g_unknown_800BE020, g_unknown_800BE022;
uint8_t g_unknown_80186108, g_unknown_80186109;
uint32_t g_unknown_80186110, g_unknown_80186114;
uint32_t g_unknown_80186118[3], g_unknown_80186148[3];
void (*g_unknown_80186144)(void);
#endif
static ControllerCycleActor actor;
static ControllerCycleComponent component;
static ControllerCycleShared shared;
static int transform, reference;
#ifdef CYCLE_REAL_POSITION_UPDATE
static int position_component;
void *g_unknown_800BE528 = &position_component;
#endif
static ProbeInput current;
static ProbeOutput output;

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

static void record(uint32_t call) {
    assert(output.count < 18);
    output.calls[output.count++] = call;
}

void *func_80028764(int32_t index) {
    assert(index == 0);
    record(1);
    if (current.mutations & 4096) {
        g_cycle_initial_vector.words[0] = 0x12345678;
        g_cycle_initial_vector.words[3] = 0x87654321;
    }
    return &actor;
}

#ifndef CYCLE_REAL_LEAVES
int32_t func_800226C0(void *value, uint8_t flags) {
    assert(value == &actor && flags == 0x10 && actor.state_04 == 0);
    record(2);
    return 0;
}
#endif

int32_t func_8001C700(int32_t index, int32_t mode) {
    assert((index == 4 && mode == 3) || (index == 5 && mode == (g_cycle_mode_flag ? 5 : 8)));
    record(index == 4 ? 3 : 4);
    return 0;
}

void func_8001B558(int32_t event, int32_t mode, uintptr_t flags, int32_t heading,
                   void *a4, void *a5, void *a6, void *a7) {
    assert(mode == 4 && a4 == NULL && a5 == NULL && a6 == NULL && a7 == NULL);
    if (event == 0x2e) {
        assert(flags == 0 && heading == 0);
        record(5);
    } else {
        assert(event == 0x13 && (uint32_t)flags == (uint32_t)actor.heading_0c &&
               heading == actor.heading_0c);
        record(15);
    }
}

int32_t func_80054AF0(const int16_t *value, int32_t *direction) {
    assert((const void *)value == &transform);
    record(6);
    memcpy(direction, current.direction, sizeof(current.direction));
    if (current.mutations & 1) g_cycle_height_offset ^= 0x80808080u;
    return 0;
}

int32_t func_80054C7C(ComponentTransformView *value, const ComponentBasisView *basis,
                      const int32_t *position) {
#ifdef CYCLE_REAL_POSITION_UPDATE
    if ((void *)value == &position_component) {
        GameCyclicEntry *mutable_position = (GameCyclicEntry *)position;
        assert(basis == NULL);
        record(8);
        output.adjusted_y = mutable_position->words[1];
        if (current.mutations & 8) mutable_position->words[1] ^= 0x80000000u;
        if (current.mutations & 16) g_cycle_height_offset ^= 0x02020202u;
        if (current.mutations & 32) mutable_position->words[0] ^= 0x40404040u;
        return 0;
    }
#endif
    assert((void *)value == &transform && basis == NULL);
    record(7);
    if (current.mutations & 2) {
        uint32_t y;
        memcpy(&y, &position[1], sizeof(y));
        y ^= 0x10101010u;
        memcpy((int32_t *)&position[1], &y, sizeof(y));
    }
    if (current.mutations & 4) g_cycle_height_offset ^= 0x01010101u;
    return 0;
}

#ifndef CYCLE_REAL_POSITION_UPDATE
void func_8002E910(const int32_t position[3]) {
    uint32_t *words = (uint32_t *)position;
    record(8);
    output.adjusted_y = words[1];
    if (current.mutations & 8) words[1] ^= 0x80000000u;
    if (current.mutations & 16) g_cycle_height_offset ^= 0x02020202u;
    if (current.mutations & 32) words[0] ^= 0x40404040u;
}
#endif

#ifndef CYCLE_REAL_LEAVES
uint32_t func_80044C1C(int32_t index) {
    assert(index == 1);
    record(9);
    return 0x12345678;
}

void func_80044C04(int32_t index, uint32_t mode) {
    assert(index == 1 && mode == 12);
    record(10);
    g_event_parameters[index] = (uint32_t)mode;
}
#endif

int32_t func_800516F8(void **result) {
    record(11);
    *result = &reference;
    return 0;
}

int32_t func_80054B20(const ComponentTransformView *value, const ComponentBasisView *basis,
                      int32_t *position) {
#ifdef CYCLE_MODEL_MOVEMENT_REAL_TARGET
    if ((const void *)value == &position_component) {
        assert(basis == NULL);
        record(19);
        memcpy(position, current.sample, sizeof(current.sample));
        return 0;
    }
#endif
    assert((const void *)value == &reference && basis == NULL);
    record(12);
    memcpy(position, current.sample, sizeof(current.sample));
    return 0;
}

#ifndef CYCLE_REAL_DISTANCE
int32_t func_80022B5C(const int32_t position[3], const int32_t other[3]) {
    int32_t distance;
    assert(memcmp(other, current.sample, sizeof(current.sample)) == 0);
    record(13);
    if (current.mutations & 64) g_cycle_motion_scale ^= 0x80808080u;
    if (current.mutations & 128) g_cycle_level_id = 14;
    if (current.mutations & 256) actor.heading_0c ^= 0x20202020;
    if (current.mutations & 512) ((uint32_t *)position)[1] ^= 0x40404040u;
    memcpy(&distance, &current.distance, sizeof(distance));
    return distance;
}
#endif

void func_80038368(void) {}

#if defined(CYCLE_MODEL_MOVEMENT) && !defined(CYCLE_MODEL_MOVEMENT_REAL_TARGET)
void func_8002E858(
    const GroundedTransformVector *direction, int32_t distance,
    uint32_t height, GroundedTransformVector *destination
) {
    record(19);
    destination->coordinates.x = height;
    destination->coordinates.y = (uint32_t)distance;
    destination->coordinates.z = direction->coordinates.x;
}
#endif
#ifdef CYCLE_MODEL_MOVEMENT_REAL_TARGET
int32_t func_80053D00(const int32_t *direction, int32_t distance, int32_t *output_vector) {
    (void)distance;
    record(20);
    memcpy(output_vector, direction, 3 * sizeof(*direction));
    return 0;
}

int32_t func_800550A4(ComponentRotationMatrix *value, const ComponentRotationMatrix *basis) {
    (void)value;
    (void)basis;
    abort();
}
#endif

void func_8002EB70(GameCyclicEntry *direction, int32_t distance, uint32_t height, int32_t duration,
                   int32_t a4, void (*callback)(void), int32_t a6, int32_t a7) {
    assert(a4 == 0 && callback == func_80038368 && a6 == 0 && a7 == 0);
    record(14);
    memcpy(output.direction, direction->words, sizeof(output.direction));
    output.motion[0] = (uint32_t)distance;
    output.motion[1] = height;
    output.motion[2] = (uint32_t)duration;
#ifdef CYCLE_MODEL_MOVEMENT
    {
        GroundedTransformVector vector;
        memcpy(&vector, direction->words, sizeof(vector));
        cycle_movement_setup(&vector, distance, height, duration, callback);
    }
#else
    if (current.mutations & 1024) actor.heading_0c ^= 0x10101010;
    if (current.mutations & 2048) g_cycle_mode_flag ^= 1;
#endif
}

void func_80028FC8(void) { record(16); }

void func_8001C8A0(int32_t a, int32_t b) {
    assert(a == 0 && b == 0);
    record(17);
}

int32_t func_8001C6BC(int32_t index, int32_t mode, int32_t value) {
    assert(index == 4 && mode == (g_cycle_mode_flag ? 14 : 15) && value == -1);
    record(18);
    return 0;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    GameCyclicEntry position;
    memset(&actor, 0, sizeof(actor));
    memset(&shared, 0xA5, sizeof(shared));
    memset(&output, 0, sizeof(output));
    memset(&g_cycle_initial_vector, 0, sizeof(g_cycle_initial_vector));
    memset(g_event_parameters, 0xA5, sizeof(g_event_parameters));
    g_event_parameters[1] = 0x12345678u;
    current = *input;
    memcpy(position.words, input->position, sizeof(position.words));
    actor.flags_10 = input->flags;
    actor.state_00 = (uint8_t)input->state;
    memcpy(&actor.state_04, &input->state04, sizeof(actor.state_04));
    memcpy(&actor.heading_0c, &input->heading, sizeof(actor.heading_0c));
    component.transform_14 = &transform;
    actor.component_1c = &component;
    actor.shared_28 = &shared;
    memset(g_cycle_saved_position, 0xA5, sizeof(g_cycle_saved_position));
    g_heading_lock_6479 = (uint8_t)input->lock;
    g_cycle_mode_flag = (uint8_t)input->mode;
    g_cycle_saved_effect = 0xA5A5;
    {
        uint16_t level = (uint16_t)input->level;
        memcpy(&g_cycle_level_id, &level, sizeof(level));
    }
    g_cycle_height_offset = input->height_offset;
    g_cycle_motion_scale = input->motion_scale;
    memcpy(&g_cycle_motion_base, &input->motion_base, sizeof(g_cycle_motion_base));
#ifdef CYCLE_MODEL_MOVEMENT
    g_unknown_800BE024 = g_unknown_800BE020 = 1;
    g_unknown_800BE022 = 9;
    g_unknown_80186108 = g_unknown_80186109 = 0;
    g_unknown_80186110 = 0x11111111;
    g_unknown_80186114 = 0x22222222;
    g_unknown_80186144 = NULL;
    for (int i = 0; i < 3; i++) {
        g_unknown_80186118[i] = 0x33333333 + i;
        g_unknown_80186148[i] = 0x44444444 + i;
    }
#endif
    func_80038430(&position);
    memcpy(output.position, position.words, sizeof(output.position));
    output.state = actor.state_00;
    output.state04 = (uint32_t)actor.state_04;
    memcpy(output.saved_position, g_cycle_saved_position, sizeof(output.saved_position));
    output.lock = g_heading_lock_6479;
    output.effect = g_cycle_saved_effect;
    memcpy(output.shared_position, shared.position_68, sizeof(output.shared_position));
    memcpy(output.vectors, shared.vector_38.words, 16);
    memcpy(output.vectors + 4, shared.vector_78.words, 16);
    memcpy(output.vectors + 8, shared.vector_98.words, 16);
    output.actor_flags = actor.flags_10;
    output.event_parameter = g_event_parameters[1];
#ifdef CYCLE_MODEL_MOVEMENT
    memcpy(output.movement_step, g_unknown_80186118, sizeof(output.movement_step));
    memcpy(output.movement_adjustment, g_unknown_80186148, sizeof(output.movement_adjustment));
    output.movement_duration = g_unknown_80186110;
    output.movement_active = g_unknown_80186108;
    output.movement_mode = g_unknown_80186109;
    output.movement_callback = g_unknown_80186144 == func_80038368;
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
        ProbeInput input = {{1, 2, 3, 4}, {1, 2, 3}, {4, 5, 6}, 0, 3, 0, 9, 8, 7,
                            11, 5, 10, 12, 320, 0};
        ProbeOutput result = run_probe(&input);
        assert(result.effect == 0x5678 && result.motion[1] == 22);
#ifdef CYCLE_MODEL_MOVEMENT
#ifdef CYCLE_MODEL_MOVEMENT_REAL_TARGET
        assert(result.motion[2] == 1 && result.count == 15);
#else
        assert(result.motion[2] == 1 && result.count == 14);
#endif
        assert(result.movement_active == 1 && result.movement_callback == 1);
#elif defined(CYCLE_REAL_DISTANCE) && defined(CYCLE_REAL_LEAVES)
        assert(result.motion[2] == 1 && result.count == 11);
#elif defined(CYCLE_REAL_DISTANCE)
        assert(result.motion[2] == 1 && result.count == 13);
#elif defined(CYCLE_REAL_LEAVES)
        assert(result.motion[2] == 2 && result.count == 12);
#else
        assert(result.motion[2] == 2 && result.count == 14);
#endif
        assert(result.direction[0] == 0xFFFFFFFFu);
    }
    puts("controller cycle warp tests passed");
    return 0;
}
