#ifndef BUBSY3D_GROUNDED_EVENT_RECORDS_H
#define BUBSY3D_GROUNDED_EVENT_RECORDS_H

#include <stdint.h>

typedef struct GroundedEventRecord {
    void *actor;
    void *value_04;
    void *value_08;
    struct GroundedEventRecord *next;
} GroundedEventRecord;

int32_t func_8004FF50(void *actor, void *value, GroundedEventRecord **record_out);
int32_t func_8004FFEC(GroundedEventRecord *record);

#endif
