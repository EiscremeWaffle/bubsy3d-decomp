#include "heading_timed_records.h"

extern int32_t func_8002BD0C(int32_t index);
extern int32_t func_80082F1C(char *output, const char *format, ...);
extern int32_t func_8002BAA0(int32_t value);
extern void func_8002BE68(int16_t index, int16_t *first, int16_t *second);
extern void func_8002C4C8(int16_t index, int32_t value, int16_t first, int16_t second);

void func_8002C604(int16_t index) {
    HeadingTimedRecord *record = &g_heading_timed_records[index];
    int32_t value;
    char formatted[16];
    int16_t first, second;

    if (record->active_1C == 1 || record->elements_20 == 0) {
        return;
    }

    value = func_8002BD0C(index);
    func_80082F1C(formatted, g_heading_timed_format, value);
    record->element_count_1E = (int16_t)func_8002BAA0(value);
    if (record->maximum_34 < record->element_count_1E) {
        record->element_count_1E = record->maximum_34;
    }
    func_8002BE68(index, &first, &second);
    if (record->payload_24 != 0) {
        first = (int16_t)((uint16_t)first + 0x23U);
    }
    func_8002C4C8(index, value, first, second);
    record->active_1C = 1;
}
