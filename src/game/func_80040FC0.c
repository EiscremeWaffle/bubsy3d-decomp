#include "level_records.h"

void *func_80040FC0(int32_t index) {
    return &g_auxiliary_object_slots[(uint32_t)index - (uint32_t)func_80028D48()];
}