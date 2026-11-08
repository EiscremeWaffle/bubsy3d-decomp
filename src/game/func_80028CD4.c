#include "level_records.h"

/* Returns list `list_id` (one-based) and writes its entry count, or NULL/0 when out of range. */
int32_t *func_80028CD4(int32_t list_id, int32_t *count) {
    uintptr_t offset = (uint32_t)list_id - 1u;

    if (g_level_list_count < list_id || (offset *= sizeof(GameLevelList), list_id <= 0)) {
        *count = 0;
        return 0;
    }
    *count = ((GameLevelList *)(offset + (uintptr_t)g_level_lists))->count_02;
    offset += (uintptr_t)g_level_lists;
    return ((GameLevelList *)offset)->indices_04;
}
