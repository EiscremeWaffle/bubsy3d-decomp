#include "heading_timed_records.h"

extern void func_8002C604(int16_t index);

void func_8002CA88(int32_t index, int32_t duration) {
    HeadingTimedRecord *record;

    if (index != 5 && index != 15 && index != 17) {
        return;
    }

    record = &g_heading_timed_records[index];
    if (record->event_0C != 0) {
        return;
    }
    if (duration != -1) {
        if (record->duration_30 < 0) {
            record->duration_30 = 0;
        } else {
            record->duration_30 = duration;
        }
    }
    func_8002C604((int16_t)index);
    func_8004FF50(record->owner_04, (void *)(uintptr_t)0x3C, &record->event_0C);
}
