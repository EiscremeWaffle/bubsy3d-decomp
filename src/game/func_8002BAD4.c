#include "heading_timed_records.h"

extern int32_t func_80050650(void *payload);

void func_8002BAD4(int16_t index) {
    HeadingTimedRecord *record = &g_heading_timed_records[index];

    if (record->payload_24 != 0) {
        func_80050650(record->payload_24);
    }
    func_8002C708(index);
    if (record->event_0C != 0) {
        func_8004FFEC(record->event_0C);
        record->event_0C = 0;
        record->state_08 = 0;
        record->state_1D = 0;
        record->state_10 = 0;
    }
}
