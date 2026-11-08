#include "level_records.h"

uint8_t func_80028C84(int32_t index, int32_t *list_id) {
    if (index >= g_level_record_count || index < 0) {
        return 0;
    }
    *list_id = g_level_records[index].list_id_18;
    return 1;
}
