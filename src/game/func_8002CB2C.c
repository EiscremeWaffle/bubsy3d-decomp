#include "heading_timed_records.h"

extern void func_8002C708(int16_t index);

void func_8002CB2C(int32_t index) {
    HeadingTimedRecord *record;
    GroundedEventRecord *event;

    if (index != 5 && index != 15) {
        return;
    }

    record = &g_heading_timed_records[index];
    event = record->event_0C;
    record->duration_30 = 0;
    if (event != 0) {
        func_8004FFEC(event);
        record->event_0C = 0;
    }
    func_8002C708((int16_t)index);
}
