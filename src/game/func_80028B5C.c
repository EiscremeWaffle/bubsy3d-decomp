#include "level_records.h"

int32_t func_80028B5C(int32_t index, GameLevelPosition *output) {
    if (index >= g_level_record_count || index < 0) {
        return 0;
    }
    *output = *g_level_records[index].position_0C;
    return 1;
}
