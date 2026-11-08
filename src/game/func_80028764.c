#include "level_records.h"

extern void *func_80040FC0(int32_t index);

void *func_80028764(int32_t index) {
    if (index < 0) {
        return 0;
    }
    if (index < g_level_record_count) {
        return g_level_records[index].object_1C;
    }
    return func_80040FC0(index);
}
