#ifndef BUBSY3D_RESET_RESOURCES_H
#define BUBSY3D_RESET_RESOURCES_H

#include <stdint.h>

typedef union RuntimeResetEntry {
    uint32_t word;
    struct {
        int16_t first;
        uint16_t second;
    } halfwords;
} RuntimeResetEntry;

extern uint8_t g_reset_active;
extern uint8_t g_reset_secondary_flag;
extern uint32_t g_reset_entry_count;
extern RuntimeResetEntry g_reset_entries[];
extern int32_t g_reset_clear_count;
extern uint32_t g_reset_clear_words[];

void func_80045094(void);

#endif
