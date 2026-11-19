#include "heading_timed_records.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

HeadingTimedRecord g_heading_timed_records[18];
const char g_heading_timed_format[] = "%ld";
static GroundedEventRecord event_record;
static uint8_t elements[4 * 0x28];
static uint8_t display_payload;
static int owner;
static unsigned calls[16], call_count, removed_count;

static void record_call(unsigned call) {
    assert(call_count < 16);
    calls[call_count++] = call;
}

int32_t func_8002BD0C(int32_t index) {
    assert(index == 5);
    record_call(1);
    return 123;
}

int32_t func_80082F1C(char *output, const char *format, ...) {
    va_list arguments;
    int32_t value;
    assert(format == g_heading_timed_format);
    va_start(arguments, format);
    value = va_arg(arguments, int32_t);
    va_end(arguments);
    assert(value == 123);
    memcpy(output, "123", 4);
    record_call(2);
    return 3;
}

int32_t func_8002BAA0(int32_t value) {
    assert(value == 123);
    record_call(3);
    return 3;
}

void func_8002BE68(int16_t index, int16_t *first, int16_t *second) {
    assert(index == 5);
    *first = 10;
    *second = -20;
    record_call(4);
}

void func_8002C4C8(int16_t index, int32_t value, int16_t first, int16_t second) {
    assert(index == 5 && value == 123 && first == 45 && second == -20);
    record_call(5);
}

int32_t func_8004FF50(void *actor, void *value, GroundedEventRecord **event_out) {
    assert(actor == &owner && (uintptr_t)value == 0x3C);
    *event_out = &event_record;
    record_call(6);
    return 0;
}

int32_t func_8004FFEC(GroundedEventRecord *event) {
    assert(event == &event_record);
    record_call(7);
    return 0;
}

int32_t func_80050650(void *payload) {
    if (payload == &display_payload) {
        record_call(9);
        return 0;
    }
    assert(removed_count < 2 && payload == &elements[removed_count * 0x28]);
    ++removed_count;
    record_call(8);
    return 0;
}

int main(void) {
    HeadingTimedRecord *record = &g_heading_timed_records[5];
    static const unsigned expected[] = {1, 2, 3, 4, 5, 6, 7, 8, 8};
    static const unsigned deleted[] = {1, 2, 3, 4, 5, 6, 9, 8, 8, 7};

    record->owner_04 = &owner;
    record->elements_20 = elements;
    record->maximum_34 = 2;
    record->payload_24 = (void *)(uintptr_t)1;
    func_8002CA88(5, 20);
    assert(record->duration_30 == 20 && record->active_1C == 1);
    assert(record->element_count_1E == 2 && record->event_0C == &event_record);
    func_8002CA88(5, 99);
    assert(record->duration_30 == 20 && call_count == 6);
    func_8002CB2C(5);
    assert(record->duration_30 == 0 && record->active_1C == 0);
    assert(record->element_count_1E == 0 && record->event_0C == 0);
    assert(call_count == sizeof(expected) / sizeof(expected[0]));
    assert(memcmp(calls, expected, sizeof(expected)) == 0);

    memset(g_heading_timed_records, 0, sizeof(g_heading_timed_records));
    call_count = removed_count = 0;
    record->owner_04 = &owner;
    record->elements_20 = elements;
    record->maximum_34 = 2;
    record->payload_24 = &display_payload;
    record->state_08 = 7;
    record->state_1D = 8;
    record->state_10 = 9;
    func_8002CA4C(5, 20);
    func_8002CA88(5, 20);
    func_8002BAD4(5);
    assert(record->duration_30 == 20 && record->active_1C == 0);
    assert(record->element_count_1E == 0 && record->event_0C == 0);
    assert(record->state_08 == 0 && record->state_1D == 0 && record->state_10 == 0);
    assert(call_count == sizeof(deleted) / sizeof(deleted[0]));
    assert(memcmp(calls, deleted, sizeof(deleted)) == 0);
    puts("heading timed-record flow tests passed");
    return 0;
}
