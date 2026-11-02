#include "grounded_event_records.h"

extern void func_80082EFC(void *record);
extern GroundedEventRecord *g_grounded_event_records;
extern uint8_t g_grounded_event_busy;

int32_t func_8004FFEC(GroundedEventRecord *record) {
    GroundedEventRecord *previous;
    GroundedEventRecord *current;

    if (record == 0) {
        return 1;
    }

    current = g_grounded_event_records;
    g_grounded_event_busy = 1;
    previous = 0;
    while (current != 0 && current != record) {
        previous = current;
        current = current->next;
    }

    if (current == 0) {
        g_grounded_event_busy = 0;
        return 1;
    }
    if (previous == 0) {
        g_grounded_event_records = current->next;
    } else {
        previous->next = current->next;
    }
    func_80082EFC(current);
    g_grounded_event_busy = 0;
    return 0;
}