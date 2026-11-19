#ifndef BUBSY3D_COMPONENT_RECORDS_H
#define BUBSY3D_COMPONENT_RECORDS_H

#include <stddef.h>
#include <stdint.h>

typedef struct ComponentRecordHalf {
    uint32_t words[4];
} ComponentRecordHalf;

typedef struct ComponentRecord {
    ComponentRecordHalf first;
    ComponentRecordHalf second;
} ComponentRecord;

typedef struct ComponentRecordTable {
    ComponentRecord *volatile records_00;
} ComponentRecordTable;

typedef struct ComponentRecordOwner {
    uint8_t unknown_00[0x18];
    ComponentRecordTable *table_18;
} ComponentRecordOwner;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(ComponentRecord) == 0x20, "component record size");
_Static_assert(offsetof(ComponentRecord, second) == 0x10, "component record second half");
_Static_assert(offsetof(ComponentRecordTable, records_00) == 0, "component record array offset");
_Static_assert(offsetof(ComponentRecordOwner, table_18) == 0x18, "component record table offset");
#endif

#ifndef BUBSY3D_MATCH_ORIGINAL_L0
/* Each half is snapshotted separately when the source and destination overlap. */
static inline void component_record_copy_half(uint32_t *output, const uint32_t *input) {
    uint32_t a = input[0], b = input[1], c = input[2], d = input[3];

    output[0] = a;
    output[1] = b;
    output[2] = c;
    output[3] = d;
}
#endif

static __inline__ void component_record_copy(ComponentRecord *output, const ComponentRecord *input) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    output->first = input->first;
    output->second = input->second;
#else
    component_record_copy_half(output->first.words, input->first.words);
    component_record_copy_half(output->second.words, input->second.words);
#endif
}

int32_t func_80052094(ComponentRecordOwner *owner, uint32_t index, uint32_t *output);
int32_t func_800528A4(ComponentRecordOwner *owner, uint32_t index, const uint32_t *input);

#endif
