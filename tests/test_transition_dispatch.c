#include "game_transition.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(sizeof(GameTransitionEventRecord) == 28, "event record stride");
_Static_assert(offsetof(GameTransitionEventRecord, flags) == 4, "event flags offset");
_Static_assert(offsetof(GameTransitionEventRecord, heading) == 8, "event heading offset");
_Static_assert(offsetof(GameTransitionEventRecord, payload.input.buttons) == 12, "input bits offset");
_Static_assert(offsetof(GameTransitionEventRecord, payload.input.digital_pad_1) == 16, "first input byte offset");
_Static_assert(offsetof(GameTransitionEventRecord, payload.input.digital_pad_2) == 17, "second input byte offset");
_Static_assert(offsetof(GameTransitionHeadingTable, entries) == 16, "heading table header");

GameTransitionEventRecord g_transition_event_records[100];
int32_t g_transition_event_count;
GameTransitionStaticHandlers g_transition_static_handlers[256];
const GameTransitionModeHandlers *g_transition_mode_handlers;
const GameTransitionHeadingTable *g_transition_heading_handlers;
const GameTransitionHeadingTable *g_level_heading_table;
const uint32_t *g_level_event_parameters;
uint32_t g_event_parameters[16];
const char g_transition_overflow_condition[] = "transition event slots";
const char g_transition_source_path[] = "../f/event.c";

static unsigned int assertion_calls;
static unsigned int argument_calls;
static unsigned int record_calls;
static unsigned int payload_calls;
static GameTransitionEventRecord *last_record;
static GameTransitionPayload last_payload;
static void *last_arguments[4];
static jmp_buf assertion_return;
static int stop_on_assertion;
static int fill_queue_on_argument;

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 1);
    assert(condition == g_transition_overflow_condition);
    assert(source == g_transition_source_path);
    assert(line == 0x1F9);
    ++assertion_calls;
    if (stop_on_assertion) {
        longjmp(assertion_return, 1);
    }
}

static void argument_handler(
    GameTransitionEventRecord *record,
    void *argument_4,
    void *argument_5,
    void *argument_6,
    void *argument_7
) {
    ++argument_calls;
    last_record = record;
    last_arguments[0] = argument_4;
    last_arguments[1] = argument_5;
    last_arguments[2] = argument_6;
    last_arguments[3] = argument_7;
    if (fill_queue_on_argument) {
        g_transition_event_count = 100;
    }
}

static void record_handler(GameTransitionEventRecord *record) {
    ++record_calls;
    last_record = record;
}

const GameTransitionModeHandlers g_default_transition_mode_handlers[256] = {
    [3] = {0, {record_handler}, func_80023A0C}
};

static void payload_handler(GameTransitionPayload *payload) {
    ++payload_calls;
    last_payload.event_id = payload->event_id;
    last_payload.mode = payload->mode;
    last_payload.flags = payload->flags;
    last_payload.heading = payload->heading;
}

static GameTransitionModeHandlers mode_handlers[256];
static GameTransitionHeadingTable *heading_table;

static void reset_state(void) {
    memset(g_transition_event_records, 0, sizeof(g_transition_event_records));
    memset(g_transition_static_handlers, 0, sizeof(g_transition_static_handlers));
    memset(mode_handlers, 0, sizeof(mode_handlers));
    memset(heading_table, 0, sizeof(*heading_table) + 8 * sizeof(heading_table->entries[0]));
    g_transition_mode_handlers = mode_handlers;
    g_transition_heading_handlers = heading_table;
    g_transition_event_count = 0;
    assertion_calls = 0;
    argument_calls = 0;
    record_calls = 0;
    payload_calls = 0;
    stop_on_assertion = 0;
    fill_queue_on_argument = 0;
    last_record = 0;
    memset(last_arguments, 0, sizeof(last_arguments));
    memset(&last_payload, 0, sizeof(last_payload));
}

static void test_input_event_payload(void) {
    uint32_t bits = 0x11223344;
    uint8_t value_10 = 0xA5;
    uint8_t value_11 = 0x5A;
    GameTransitionEventRecord *record;
    size_t i;

    reset_state();
    record = &g_transition_event_records[0];
    memset(&record->payload, 0xCC, sizeof(record->payload));
    g_transition_static_handlers[10].flags_ffff = func_80023A0C;
    func_8001B558(10, 4, 0xFFFF, 0xFFFF, &bits, &value_10, &value_11, 0);
    assert(g_transition_event_count == 1);
    assert(record->event_id == 10 && record->mode == 4);
    assert(record->flags == 0xFFFF && record->heading == 0xFFFF);
    assert(record->payload.input.buttons == bits);
    assert(record->payload.input.digital_pad_1 == value_10);
    assert(record->payload.input.digital_pad_2 == value_11);
    for (i = 0; i < sizeof(record->payload.input.reserved); ++i) {
        assert(record->payload.input.reserved[i] == 0xCC);
    }

    reset_state();
    record = &g_transition_event_records[0];
    func_80023A0C(record, &bits, &record->payload.input.buttons,
                  &record->payload.input.digital_pad_1, 0);
    assert(record->payload.input.buttons == bits);
    assert(record->payload.input.digital_pad_1 == *(const uint8_t *)&bits);
    assert(record->payload.input.digital_pad_2 == *(const uint8_t *)&bits);
}

static void test_controller_event(void) {
    uint32_t bits = 1u << 13;
    uint8_t port_1 = 1;
    uint8_t port_2 = 0;
    GameTransitionEventRecord *record;

    reset_state();
    func_8002278C();
    assert(g_transition_mode_handlers == g_default_transition_mode_handlers);
    func_8001B558(3, 0, 0xFFFE, 0xFFFE, &bits, &port_1, &port_2, 0);
    record = &g_transition_event_records[0];
    assert(g_transition_event_count == 1);
    assert(record->event_id == 3 && record->mode == 0);
    assert(record->flags == 0xFFFE && record->heading == 0xFFFE);
    assert(record->payload.input.buttons == bits);
    assert(record->payload.input.digital_pad_1 == 1 && record->payload.input.digital_pad_2 == 0);

    reset_state();
    func_8002278C();
    func_8001B558(3, 5, 0xFFFE, 0xFFFE, &bits, &port_1, &port_2, 0);
    assert(record_calls == 1 && last_record == &g_transition_event_records[0]);
    assert(last_record->payload.input.buttons == bits);
    assert(g_transition_event_count == 0);
}

static void test_heading_table_activation(void) {
    uint32_t expected[16];
    size_t i;
    size_t shift;

    reset_state();
    heading_table->entries[0].handler = payload_handler;
    g_transition_heading_handlers = 0;
    g_level_heading_table = heading_table;
    for (i = 0; i < 16; ++i) {
        g_event_parameters[i] = (uint32_t)(0x1000 + i);
    }
    g_level_event_parameters = &g_event_parameters[3];
    memcpy(expected, g_event_parameters, sizeof(expected));
    for (i = 0; i < 8; ++i) {
        expected[i] = expected[i + 3];
    }
    func_80027F08();
    assert(g_transition_heading_handlers == heading_table);
    assert(memcmp(g_event_parameters, expected, sizeof(expected)) == 0);
    func_8001B558(8, 5, 0xFFFE, 0, 0, 0, 0, 0);
    assert(payload_calls == 1);
    assert(last_payload.event_id == 8 && last_payload.heading == 0);

    for (shift = 0; shift <= 8; ++shift) {
        for (i = 0; i < 16; ++i) {
            g_event_parameters[i] = (uint32_t)(0xAB00 + i);
        }
        memcpy(expected, g_event_parameters, sizeof(expected));
        for (i = 0; i < 8; ++i) {
            expected[i] = expected[i + shift];
        }
        func_80044C34(&g_event_parameters[shift]);
        assert(memcmp(g_event_parameters, expected, sizeof(expected)) == 0);
    }
}

int main(void) {
    int args[4];

    heading_table = malloc(sizeof(*heading_table) + 8 * sizeof(heading_table->entries[0]));
    assert(heading_table != 0);

    test_input_event_payload();
    test_controller_event();
    test_heading_table_activation();

    reset_state();
    g_transition_event_records[0].event_id = 1;
    g_transition_static_handlers[0x1A].flags_ffff = argument_handler;
    func_8001B558(0x11A, 4, 0xFFFF, 3, &args[0], &args[1], &args[2], &args[3]);
    assert(argument_calls == 1);
    assert(last_record == &g_transition_event_records[1]);
    assert(last_arguments[0] == &args[0]);
    assert(last_arguments[1] == &args[1]);
    assert(last_arguments[2] == &args[2]);
    assert(last_arguments[3] == &args[3]);
    assert(g_transition_event_count == 1);
    assert(g_transition_event_records[1].event_id == 0x1A);
    assert(g_transition_event_records[1].mode == 4);
    assert(g_transition_event_records[1].flags == 0xFFFF);
    assert(g_transition_event_records[1].heading == 3);

    reset_state();
    g_transition_static_handlers[1].flags_ffff = argument_handler;
    fill_queue_on_argument = 1;
    func_8001B558(1, 4, 0xFFFF, 3, 0, 0, 0, 0);
    assert(argument_calls == 1);
    assert(g_transition_event_count == 100);
    assert(g_transition_event_records[0].event_id == 0);

    reset_state();
    g_transition_static_handlers[7].mode_5_heading_ffff = record_handler;
    func_8001B558(7, 5, 0, 0xFFFF, &args[0], &args[1], &args[2], &args[3]);
    assert(record_calls == 1);
    assert(last_record == &g_transition_event_records[0]);
    assert(g_transition_event_count == 0);

    reset_state();
    mode_handlers[9].flags_fffe = argument_handler;
    func_8001B558(9, 1, 0xFFFE, 4, &args[0], &args[1], &args[2], &args[3]);
    assert(argument_calls == 1);
    assert(last_record == &g_transition_event_records[0]);
    assert(last_arguments[0] == &args[0]);
    assert(last_arguments[1] == &args[1]);
    assert(last_arguments[2] == &args[2]);
    assert(last_arguments[3] == &args[3]);
    assert(g_transition_event_count == 1);

    reset_state();
    mode_handlers[13].heading.heading_fffd = payload_handler;
    func_8001B558(13, 0x105, 0x12345678, 0xFFFD, 0, 0, 0, 0);
    assert(payload_calls == 1);
    assert(last_payload.event_id == 13);
    assert(last_payload.mode == 5);
    assert(last_payload.flags == 0x12345678);
    assert(last_payload.heading == 0xFFFD);
    assert(g_transition_event_count == 0);

    reset_state();
    mode_handlers[9].heading.heading_fffe = record_handler;
    func_8001B558(9, 5, 0, 0xFFFE, 0, 0, 0, 0);
    assert(record_calls == 1);
    assert(last_record == &g_transition_event_records[0]);
    assert(g_transition_event_count == 0);

    reset_state();
    heading_table->entries[3].handler = payload_handler;
    func_8001B558(2, 5, 5, 3, 0, 0, 0, 0);
    assert(payload_calls == 1);
    assert(last_payload.event_id == 2);
    assert(last_payload.heading == 3);
    assert(g_transition_event_count == 0);

    reset_state();
    heading_table->entries[3].handler = payload_handler;
    func_8001B558(2, 4, 5, 3, 0, 0, 0, 0);
    assert(payload_calls == 0);
    assert(g_transition_event_count == 1);

    reset_state();
    func_8001B558(2, 5, 5, 3, 0, 0, 0, 0);
    assert(payload_calls == 0);
    assert(g_transition_event_count == 0);

    reset_state();
    g_transition_static_handlers[2].flags_ffff = argument_handler;
    func_8001B558(2, 4, -1, 3, 0, 0, 0, 0);
    assert(argument_calls == 0);
    assert(g_transition_event_count == 1);

    reset_state();
    g_transition_event_count = 2;
    g_transition_event_records[0].event_id = 0;
    g_transition_event_records[1].event_id = 2;
    g_transition_event_records[1].flags = 5;
    g_transition_event_records[1].heading = 3;
    func_8001B558(2, 4, 5, 3, 0, 0, 0, 0);
    assert(g_transition_event_count == 2);
    assert(g_transition_event_records[2].event_id == 0);

    reset_state();
    g_transition_event_count = 2;
    g_transition_event_records[1].event_id = 7;
    g_transition_event_records[2].event_id = 2;
    g_transition_event_records[2].flags = 5;
    g_transition_event_records[2].heading = 3;
    func_8001B558(2, 4, 5, 3, 0, 0, 0, 0);
    assert(g_transition_event_count == 2);
    assert(g_transition_event_records[3].event_id == 0);

    reset_state();
    mode_handlers[9].flags_fffe = argument_handler;
    func_8001B558(9, 4, 0xFFFE, 0xFFFE, 0, 0, 0, 0);
    func_8001B558(9, 4, 0xFFFE, 0xFFFE, 0, 0, 0, 0);
    assert(argument_calls == 2);
    assert(g_transition_event_count == 2);
    assert(g_transition_event_records[1].event_id == 9);

    reset_state();
    g_transition_event_count = 1;
    g_transition_event_records[3].event_id = 9;
    g_transition_event_records[3].flags = 0xFFFE;
    g_transition_event_records[3].heading = 0xFFFE;
    func_8001B558(9, 4, 0xFFFE, 0xFFFE, 0, 0, 0, 0);
    assert(g_transition_event_count == 2);
    assert(g_transition_event_records[0].event_id == 9);

    reset_state();
    g_transition_event_records[0].payload.opaque[2] = 0x12345678;
    func_8001B558(0x100, 4, (uintptr_t)&args[0], -1, 0, 0, 0, 0);
    assert(g_transition_event_count == 1);
    assert(g_transition_event_records[0].event_id == 0);
    assert(g_transition_event_records[0].flags == (int32_t)(uintptr_t)&args[0]);
    assert(g_transition_event_records[0].heading == -1);
    assert(g_transition_event_records[0].payload.opaque[2] == 0x12345678);

    reset_state();
    g_transition_event_count = -1;
    func_8001B558(1, 2, 3, 4, 0, 0, 0, 0);
    assert(g_transition_event_count == 0);
    assert(g_transition_event_records[0].event_id == 1);

    reset_state();
    g_transition_event_count = 100;
    func_8001B558(1, 2, 3, 4, 0, 0, 0, 0);
    assert(g_transition_event_count == 100);
    assert(g_transition_event_records[0].event_id == 0);

    reset_state();
    for (size_t i = 0; i < 100; ++i) {
        g_transition_event_records[i].event_id = 1;
    }
    stop_on_assertion = 1;
    if (setjmp(assertion_return) == 0) {
        func_8001B558(1, 2, 3, 4, 0, 0, 0, 0);
        assert(0);
    }
    assert(assertion_calls == 1);
    assert(g_transition_event_count == 0);
    assert(g_transition_event_records[0].event_id == 1);

    free(heading_table);
    return 0;
}
