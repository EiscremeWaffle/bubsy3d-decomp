#include "grounded_event_records.h"

extern void *func_80082EEC(uint32_t size);
extern GroundedEventRecord *g_grounded_event_records;
extern uint8_t g_grounded_event_busy;

int32_t func_8004FF50(
    void *actor,
    void *value,
    GroundedEventRecord **record_out
) {
    GroundedEventRecord *record;
    GroundedEventRecord *previous;

    if (actor == 0 || value == 0) {
        return 1;
    }

    g_grounded_event_busy = 1;
    record = func_80082EEC(sizeof(*record));
    if (record == 0 || record == (GroundedEventRecord *)(uint32_t)0xFFFFFFFFu) {
        g_grounded_event_busy = 0;
        return 3;
    }

    previous = g_grounded_event_records;
    record->actor = actor;
    record->value_04 = value;
    record->value_08 = value;
    g_grounded_event_records = record;
    record->next = previous;
    *record_out = record;
    g_grounded_event_busy = 0;
    return 0;
}