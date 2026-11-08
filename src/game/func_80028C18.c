#include "level_records.h"

uint8_t func_80028C18(int32_t index, int32_t *count, const MovePreparationEntry **table) {
    if (index >= g_level_record_count || index < 0) {
        return 0;
    }
    *table = g_level_records[index].table_14;
    *count = g_level_records[index].table_count_10;
    return 1;
}
