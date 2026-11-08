#ifndef BUBSY3D_RESET_ENTRY_HELPERS_H
#define BUBSY3D_RESET_ENTRY_HELPERS_H

#include "proximity_list.h"

typedef struct ResetTrackedObject {
    uint8_t reserved_00[0x10];
    uint32_t flags;
    uint8_t reserved_14[8];
    ProximityObject *reference;
} ResetTrackedObject;

typedef struct ResetTrackedNode {
    ResetTrackedObject *object;
    int16_t tag;
    uint8_t reserved_06[6];
    struct ResetTrackedNode *next;
} ResetTrackedNode;

typedef struct ResetEntryGroup {
    uint32_t *indices;
    uint32_t reserved_04;
    int32_t count;
    uint32_t reserved_0C;
} ResetEntryGroup;

typedef struct ResetSelectionActor {
    uint8_t reserved_00[8];
    void *object;
} ResetSelectionActor;

extern ResetTrackedNode *g_reset_tracked_head;
extern void **g_reset_entry_table;
extern ResetEntryGroup *g_reset_entry_groups;
extern void **g_reset_selected_objects;
extern uint32_t g_reset_active_group_count;
extern uint32_t g_reset_active_group_indices[];

void func_8004232C(int32_t tag);
uint8_t func_80044458(uint32_t index, uint32_t mode);
int32_t func_80044A88(uint32_t object_index, ResetEntryGroup *excluded_group);
int32_t func_80044B24(uint32_t index, uint32_t mode);

#endif
