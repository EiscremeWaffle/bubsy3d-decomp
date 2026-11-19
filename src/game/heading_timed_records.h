#ifndef BUBSY3D_HEADING_TIMED_RECORDS_H
#define BUBSY3D_HEADING_TIMED_RECORDS_H

#include "grounded_event_records.h"
#include <stddef.h>
#include <stdint.h>

typedef struct HeadingTimedRecord {
    uint8_t unknown_00[4];
    void *owner_04;
    uint8_t state_08;
    uint8_t unknown_09[3];
    GroundedEventRecord *event_0C;
    uint32_t state_10;
    uint8_t unknown_14[8];
    uint8_t active_1C;
    uint8_t state_1D;
    int16_t element_count_1E;
    uint8_t *elements_20;
    void *payload_24;
    uint8_t unknown_28[8];
    int32_t duration_30;
    uint8_t maximum_34;
    uint8_t unknown_35[3];
} HeadingTimedRecord;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && UINTPTR_MAX == 0xFFFFFFFFu
_Static_assert(offsetof(HeadingTimedRecord, owner_04) == 0x04, "timed record owner offset");
_Static_assert(offsetof(HeadingTimedRecord, state_08) == 0x08, "timed record state offset");
_Static_assert(offsetof(HeadingTimedRecord, event_0C) == 0x0C, "timed record event offset");
_Static_assert(offsetof(HeadingTimedRecord, state_10) == 0x10, "timed record state offset");
_Static_assert(offsetof(HeadingTimedRecord, active_1C) == 0x1C, "timed record active offset");
_Static_assert(offsetof(HeadingTimedRecord, state_1D) == 0x1D, "timed record state offset");
_Static_assert(offsetof(HeadingTimedRecord, element_count_1E) == 0x1E, "timed record count offset");
_Static_assert(offsetof(HeadingTimedRecord, elements_20) == 0x20, "timed record elements offset");
_Static_assert(offsetof(HeadingTimedRecord, payload_24) == 0x24, "timed record payload offset");
_Static_assert(offsetof(HeadingTimedRecord, duration_30) == 0x30, "timed record duration offset");
_Static_assert(offsetof(HeadingTimedRecord, maximum_34) == 0x34, "timed record maximum offset");
_Static_assert(sizeof(HeadingTimedRecord) == 0x38, "timed record size");
#endif

extern HeadingTimedRecord g_heading_timed_records[18];
extern const char g_heading_timed_format[];

void func_8002C604(int16_t index);
void func_8002BAD4(int16_t index);
void func_8002CA4C(int32_t index, int32_t duration);
void func_8002CA88(int32_t index, int32_t duration);
void func_8002CB2C(int32_t index);
void func_8002C708(int16_t index);

#endif
