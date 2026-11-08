#include "level_records.h"

uint8_t func_800286F8(int32_t index, uint8_t *selector) {
    if (index < 0) {
        return 0;
    }
    *selector = g_level_records[index].selector_04;
    return 1;
}
