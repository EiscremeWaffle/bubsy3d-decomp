#include "game_transition.h"
#include "input_events.h"
#include "proximity_list.h"
#include "reset_resources.h"
#include "texture_font_slots.h"
#include "timeout_resources.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

uint8_t g_event_input_gate;
uint8_t g_event_runtime_byte;
uint8_t g_unknown_800BE1C2;
uint32_t global_0x800BDE98;
uint8_t g_timeout_event_id;
volatile uint32_t g_timeout_event_flags;
volatile int32_t g_timeout_event_heading;
const char g_sequence_query_condition[] = "timeout condition";
const char g_transition_source_path[] = "transition source";

uint8_t g_texture_font_loaded;
TextureFontSlot g_texture_font_slots[TEXTURE_FONT_SLOT_COUNT];
uint32_t g_texture_font_count;
const char g_texture_font_error_condition[] = "slot condition";
const char g_texture_font_source_path[] = "slot source";

uint8_t g_timeout_resource_active;
int32_t g_timeout_resource_protected;
void *g_timeout_resource_a;
void *g_timeout_resource_b;
void *g_timeout_resource_c;
void *g_timeout_resource_d;
uint8_t g_timeout_resource_state[1];

uint8_t g_reset_active;
uint8_t g_reset_secondary_flag;
uint32_t g_reset_entry_count;
RuntimeResetEntry g_reset_entries[2];
int32_t g_reset_clear_count;
uint32_t g_reset_clear_words[2];

uint16_t g_proximity_count;
ProximityObject *g_proximity_objects[40];
const char g_proximity_limit_condition[] = "proximity condition";
const char g_proximity_source_path[] = "proximity source";

enum CallKind {
    SHUTDOWN_A, SHUTDOWN_B, SHUTDOWN_C, SHUTDOWN_D, FREE_A, FREE_B, RESOURCE_STATE,
    SLOT_0, SLOT_1, SLOT_2, ACTOR, CONTEXT, RESET_SETUP, ENTRY_WORD_0, ENTRY_HALF_0,
    ENTRY_WORD_1, ENTRY_HALF_1, RESET_TICK, RESET_FINALIZE, REFERENCE_ACTIVE,
    REFERENCE_INACTIVE, LIST, RESET_EVENT, TIMEOUT_EVENT
};

static enum CallKind calls[32];
static size_t call_count;
static uint8_t allocations[2];
static uint8_t actor[16];
static uint8_t context[0x6E];
static uint8_t replacement_id;
static unsigned int entry_index;
static ProximityPosition reference_position, object_position;
static ProximityObject reference_object, object;
static ProximityNode node;

static void record(enum CallKind kind) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = kind;
}

void func_80062750(void) {
    assert(g_timeout_resource_active == 1 && g_event_input_gate == 0x5A);
    record(SHUTDOWN_A);
}

void func_8008714C(void) {
    record(SHUTDOWN_B);
}

void func_800968BC(int32_t value) {
    assert(value == 0);
    record(SHUTDOWN_C);
}

void func_80096AEC(int32_t value) {
    assert(value == 0);
    record(SHUTDOWN_D);
}

void func_80082EFC(void *allocation) {
    if (allocation == &allocations[0]) {
        record(FREE_A);
    } else {
        assert(allocation == &allocations[1]);
        record(FREE_B);
    }
}

void func_800601D0(void *state) {
    assert(state == g_timeout_resource_state && g_timeout_resource_active == 1);
    record(RESOURCE_STATE);
}

int32_t func_80050650(void *payload) {
    size_t index;
    for (index = 0; index < 3; ++index) {
        if (payload == g_texture_font_slots[index].data.payload) {
            record((enum CallKind)(SLOT_0 + index));
            return 0;
        }
    }
    assert(0 && "unexpected slot payload");
    return -1;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(0 && "unexpected assertion callback");
}

void *func_80028764(int32_t index) {
    assert(index == 0 && g_event_input_gate == 0 && g_event_runtime_byte == 0x5A);
    record(ACTOR);
    return actor;
}

int32_t func_8003AA1C(void **output) {
    assert(g_event_runtime_byte == 0);
    record(CONTEXT);
    *output = context;
    return 1;
}

void func_8004380C(void) {
    assert(g_reset_active == 1 && global_0x800BDE98 == 0);
    record(RESET_SETUP);
}

uint8_t func_80044458(uint32_t value, uint32_t mode) {
    assert(g_reset_active == 1 && mode == 0 && entry_index < 2);
    assert(value == g_reset_entries[entry_index].word);
    record(entry_index == 0 ? ENTRY_WORD_0 : ENTRY_WORD_1);
    return 0;
}

void func_8004232C(int32_t value) {
    assert(value == (entry_index == 0 ? -1 : 128));
    record(entry_index == 0 ? ENTRY_HALF_0 : ENTRY_HALF_1);
    ++entry_index;
}

uint32_t func_800439EC(void) {
    assert(g_reset_active == 1 && g_reset_secondary_flag == 0);
    assert(g_reset_clear_words[0] == 0 && g_reset_clear_words[1] == 0);
    record(RESET_TICK);
    return 0;
}

void func_80044268(void) {
    record(RESET_FINALIZE);
}

ProximityObject *func_8005327C(void) {
    record(g_reset_active ? REFERENCE_ACTIVE : REFERENCE_INACTIVE);
    return &reference_object;
}

int32_t func_800529D0(ProximityNode **output) {
    assert(g_proximity_count == 0);
    record(LIST);
    *output = &node;
    return 0;
}

void func_8001B558(int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
                   void *argument_4, void *argument_5, void *argument_6, void *argument_7) {
    assert(argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    if (mode == 1) {
        record(RESET_EVENT);
        assert(event_id == 4 && flags == 0xFFFE && heading == 0xFFFE);
        assert((uintptr_t)argument_4 == (uintptr_t)actor + 0x0C);
        assert(g_reset_active == 0 && g_proximity_count == 1 && g_proximity_objects[0] == &object);
        g_timeout_event_id = replacement_id;
        g_timeout_event_flags = 0xAABBCCDDu;
        g_timeout_event_heading = INT32_MIN;
        g_event_input_gate = 0xA5;
    } else {
        record(TIMEOUT_EVENT);
        assert(mode == 0 && event_id == replacement_id);
        assert(flags == 0xAABBCCDDu && heading == INT32_MIN && argument_4 == 0);
    }
}

static void initialize(uint8_t id) {
    size_t index;
    replacement_id = id;
    call_count = 0;
    entry_index = 0;
    memset(context, 0, sizeof(context));
    g_event_input_gate = 0x5A;
    g_event_runtime_byte = 0x5A;
    g_unknown_800BE1C2 = 1;
    global_0x800BDE98 = UINT32_MAX;
    g_timeout_event_id = 7;
    g_timeout_event_flags = 0;
    g_timeout_event_heading = 0;
    g_timeout_resource_active = 1;
    g_timeout_resource_protected = 0;
    g_timeout_resource_a = &allocations[0];
    g_timeout_resource_b = &allocations[1];
    g_timeout_resource_c = 0;
    g_timeout_resource_d = 0;
    g_texture_font_loaded = 1;
    g_texture_font_count = 3;
    for (index = 0; index < TEXTURE_FONT_SLOT_COUNT; ++index) {
        g_texture_font_slots[index].tag = -1;
    }
    g_texture_font_slots[0].tag = 5;
    g_texture_font_slots[1].tag = 5;
    g_texture_font_slots[2].tag = 7;
    g_reset_active = 0;
    g_reset_secondary_flag = 1;
    g_reset_entry_count = 2;
    g_reset_entries[0].word = 0x1234FFFFu;
    g_reset_entries[1].word = 0xF00D0080u;
    g_reset_clear_count = 2;
    g_reset_clear_words[0] = 0xFFFFFFFFu;
    g_reset_clear_words[1] = 0xFFFFFFFFu;
    reference_position.x = 100;
    reference_position.z = -20;
    object_position.x = 101;
    object_position.z = -20;
    reference_object.position = &reference_position;
    object.position = &object_position;
    object.radius = 2;
    node.object = &object;
    node.next = 0;
    g_proximity_count = 0xFFFF;
    g_proximity_objects[0] = 0;
}

int main(void) {
    static const enum CallKind expected[] = {
        SHUTDOWN_A, SHUTDOWN_B, SHUTDOWN_C, SHUTDOWN_D, FREE_A, FREE_B, RESOURCE_STATE,
        SLOT_0, SLOT_1, SLOT_2, ACTOR, CONTEXT, RESET_SETUP, ENTRY_WORD_0, ENTRY_HALF_0,
        ENTRY_WORD_1, ENTRY_HALF_1, RESET_TICK, RESET_TICK, RESET_FINALIZE,
        REFERENCE_ACTIVE, LIST, REFERENCE_INACTIVE, LIST, RESET_EVENT
    };
    static const uint8_t ids[] = {5, 0xFE, 0xFF};
    size_t index, call;

    for (index = 0; index < sizeof(ids) / sizeof(ids[0]); ++index) {
        initialize(ids[index]);
        assert(func_8001C04C() == 0);
        assert(call_count == sizeof(expected) / sizeof(expected[0]) + (ids[index] < 0xFE));
        for (call = 0; call < sizeof(expected) / sizeof(expected[0]); ++call) {
            assert(calls[call] == expected[call]);
        }
        if (ids[index] < 0xFE) {
            assert(calls[call] == TIMEOUT_EVENT);
        }
        assert(g_timeout_resource_active == 0);
        assert(g_timeout_resource_a == 0 && g_timeout_resource_b == 0);
        assert(g_texture_font_count == 0);
        assert(g_texture_font_slots[0].tag == -1 && g_texture_font_slots[1].tag == -1);
        assert(g_texture_font_slots[2].tag == -1);
        assert(g_event_runtime_byte == 0 && g_event_input_gate == 0xA5);
        assert(global_0x800BDE98 == 0 && g_reset_active == 0);
        assert(g_proximity_count == 1 && g_proximity_objects[0] == &object);
    }
    puts("full timeout-reset integration tests passed");
    return 0;
}
