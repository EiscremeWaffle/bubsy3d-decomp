#include "game_transition.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint32_t g_queue_counter_144;
uint32_t g_queue_counter_148;
int32_t g_queue_counter_14C;
uint32_t g_queue_value_150;
uint8_t g_queue_mode;
int16_t g_queue_transition_state;
const char g_queue_message[] = "queue timeout";
GameTransitionQueue g_transition_queue_input;
GameTransitionQueue g_transition_queue_snapshot;
GameTransitionStaticHandlers g_transition_static_handlers[256];
const GameTransitionModeHandlers *g_transition_mode_handlers;
const GameTransitionHeadingTable *g_transition_heading_handlers;

typedef struct TestObject {
    uint8_t unused_00_to_0F[0x10];
    int32_t pending_value;
    uint8_t unused_14_to_1F[0x0C];
    void *state;
} TestObject;

typedef struct TestObjectState {
    uint8_t unused_00_to_1F[0x20];
    int32_t pending_value;
} TestObjectState;

typedef struct TestHeadingTable {
    uint32_t reserved[4];
    GameTransitionHeadingEntry entries[4];
} TestHeadingTable;

void func_8001B1E4(void *actor, void *context);

static TestObject test_object;
static TestObjectState test_object_state;
static uint8_t runtime_record[0x30];
static uint8_t actor_context[0x10];
static GameTransitionModeHandlers mode_handlers[256];
static TestHeadingTable heading_table;
static unsigned int record_dispatches;
static unsigned int payload_dispatches;
static unsigned int fallback_dispatches;
static unsigned int queue_update_calls;
static unsigned int finish_calls;
static unsigned int tail_gate_calls;
static unsigned int reset_pending_on_dispatch;
static int tail_gate_mode;
static GameTransitionEventRecord *last_dispatched_record;

static void capture_record(GameTransitionEventRecord *record) {
    ++record_dispatches;
    last_dispatched_record = record;
    if (reset_pending_on_dispatch) {
        g_transition_queue_snapshot.count = 0;
    }
}

static void capture_payload(GameTransitionPayload *payload) {
    ++payload_dispatches;
    last_dispatched_record = (GameTransitionEventRecord *)payload;
    if (reset_pending_on_dispatch) {
        g_transition_queue_snapshot.count = 0;
    }
}

void *func_80026648(void *actor, void *context) {
    (void)actor;
    return context;
}

void *func_80028764(int32_t index) {
    assert(index == 0);
    return &test_object;
}

int32_t func_8003AA1C(void **output) {
    *output = runtime_record;
    return 0;
}

void func_800443F8(void) {
}

void func_8003539C(void *actor, int32_t update) {
    (void)actor;
    (void)update;
}

void func_80055680(const char *message) {
    assert(message == g_queue_message);
}

void func_80032D7C(void) {
    ++queue_update_calls;
}

void func_8001B8B8(GameTransitionQueue *input, GameTransitionQueue *output) {
    memcpy(output, input, sizeof(*output));
}

void func_8001B044(void) {
    ++fallback_dispatches;
}

void func_8006BB84(int32_t mode) {
    assert(mode == 1);
}

void func_800250CC(void) {
}

uint32_t func_800439EC(void) {
    return 0;
}

void func_80044268(void) {
}

void func_8001B0A0(void) {
}

void func_80029164(void) {
    ++finish_calls;
}

int32_t func_80032C8C(int32_t mode) {
    ++tail_gate_calls;
    tail_gate_mode = mode;
    return 1;
}

static void reset_state(void) {
    memset(&g_transition_queue_input, 0, sizeof(g_transition_queue_input));
    memset(&g_transition_queue_snapshot, 0, sizeof(g_transition_queue_snapshot));
    memset(g_transition_static_handlers, 0, sizeof(g_transition_static_handlers));
    memset(mode_handlers, 0, sizeof(mode_handlers));
    memset(&heading_table, 0, sizeof(heading_table));
    memset(&test_object, 0, sizeof(test_object));
    memset(&test_object_state, 0, sizeof(test_object_state));
    memset(runtime_record, 0, sizeof(runtime_record));
    memset(actor_context, 0, sizeof(actor_context));
    test_object.state = &test_object_state;
    actor_context[0x0D] = 1;
    runtime_record[0x29] = 2;
    g_queue_counter_144 = 0;
    g_queue_counter_148 = 0;
    g_queue_counter_14C = 0;
    g_queue_value_150 = 0;
    g_queue_mode = 1;
    g_queue_transition_state = 0;
    g_transition_mode_handlers = mode_handlers;
    g_transition_heading_handlers =
        (const GameTransitionHeadingTable *)&heading_table;
    record_dispatches = 0;
    payload_dispatches = 0;
    fallback_dispatches = 0;
    queue_update_calls = 0;
    finish_calls = 0;
    tail_gate_calls = 0;
    reset_pending_on_dispatch = 0;
    tail_gate_mode = -1;
}

static GameTransitionEventRecord *set_record(
    unsigned int index,
    uint16_t event_id,
    uint16_t mode,
    int32_t heading
) {
    GameTransitionEventRecord *record =
        &g_transition_queue_input.records[index].event;
    record->event_id = event_id;
    record->mode = mode;
    record->heading = heading;
    return record;
}

static void run_processor(void) {
    func_8001B1E4(0, actor_context);
    assert(queue_update_calls == 1);
    assert(finish_calls == 1);
    assert(tail_gate_calls == 1);
}

static void test_heading_dispatch_and_counter_update(void) {
    GameTransitionEventRecord *record;

    reset_state();
    g_queue_counter_144 = 0x1234;
    g_queue_counter_148 = 0x1220;
    g_transition_queue_input.count = 1;
    record = set_record(0, 0x31, 0, 0);
    heading_table.entries[0].handler = capture_payload;

    run_processor();

    assert(payload_dispatches == 1);
    assert(record_dispatches == 0);
    assert(last_dispatched_record == &g_transition_queue_snapshot.records[0].event);
    assert(g_queue_counter_148 == 0x1234);
    assert(g_queue_mode == 1);
    assert(record->event_id == 0x31);
    assert(tail_gate_mode == 1);
}

static void test_static_and_mode_dispatch(void) {
    reset_state();
    g_transition_queue_input.count = 1;
    set_record(0, 7, 0, 0xFFFF);
    g_transition_static_handlers[7].mode_5_heading_ffff = capture_record;
    run_processor();
    assert(record_dispatches == 1);
    assert(last_dispatched_record == &g_transition_queue_snapshot.records[0].event);

    reset_state();
    g_transition_queue_input.count = 1;
    set_record(0, 9, 0, 0xFFFE);
    mode_handlers[9].heading.heading_fffe = capture_record;
    run_processor();
    assert(record_dispatches == 1);
    assert(last_dispatched_record == &g_transition_queue_snapshot.records[0].event);
}

static void test_heading_parity_fallback_and_empty_count(void) {
    reset_state();
    g_queue_mode = 3;
    g_transition_queue_input.count = 1;
    set_record(0, 0x22, 2, 2);
    heading_table.entries[2].handler = capture_payload;
    g_queue_transition_state = 1;
    run_processor();
    assert(payload_dispatches == 1);
    assert(fallback_dispatches == 0);

    reset_state();
    g_queue_mode = 3;
    g_transition_queue_input.count = 1;
    set_record(0, 0x22, 2, 2);
    g_queue_transition_state = 0;
    run_processor();
    assert(payload_dispatches == 0);
    assert(fallback_dispatches == 1);

    reset_state();
    g_queue_mode = 3;
    g_transition_queue_input.count = 2;
    set_record(0, 1, 0, 0);
    set_record(1, 2, 0, 0);
    heading_table.entries[0].handler = capture_payload;
    reset_pending_on_dispatch = 1;
    run_processor();
    assert(payload_dispatches == 1);
    assert(last_dispatched_record == &g_transition_queue_snapshot.records[0].event);
}

static void test_generated_event_suppresses_default_action(void) {
    reset_state();
    g_queue_mode = 0;
    g_transition_queue_input.count = 1;
    set_record(0, 4, 0, 0)->payload.opaque[0] = 0;
    heading_table.entries[0].handler = capture_payload;
    run_processor();
    assert(payload_dispatches == 1);
    assert(g_queue_counter_144 == 1);
    assert(g_queue_counter_148 == 1);
    assert(tail_gate_mode == 0);
}

int main(void) {
    test_heading_dispatch_and_counter_update();
    test_static_and_mode_dispatch();
    test_heading_parity_fallback_and_empty_count();
    test_generated_event_suppresses_default_action();
    puts("Queue processor: snapshot dispatch, callback routing, state gates and tail calls passed.");
    return 0;
}
