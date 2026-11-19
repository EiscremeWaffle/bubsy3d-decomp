#include "heading_timed_records.h"

extern int32_t func_80050650(void *payload);

void func_8002C708(int16_t index) {
    HeadingTimedRecord *record = &g_heading_timed_records[index];
    int32_t element;

    if (record->elements_20 != 0) {
        for (element = 0; element < record->element_count_1E; ++element) {
            func_80050650(record->elements_20 + (uint32_t)element * 0x28U);
        }
        record->active_1C = 0;
        record->element_count_1E = 0;
    }
}
