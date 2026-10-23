#include <stdint.h>

typedef struct GroundedEventRecord {
    void *actor;
    void *value_04;
    void *value_08;
    struct GroundedEventRecord *next;
} GroundedEventRecord;

extern void *func_80082EEC(uint32_t size);

int32_t func_8004FF50(
    void *actor,
    void *value,
    GroundedEventRecord **record_out
) {
    register volatile uint8_t *runtime_globals __asm__("$28");
    GroundedEventRecord *record;

    if (actor == 0 || value == 0) {
        return 1;
    }

    runtime_globals[0x860] = 1;
    record = func_80082EEC(sizeof(*record));
    if (record == 0 || record == (GroundedEventRecord *)(uint32_t)0xFFFFFFFFu) {
        runtime_globals[0x860] = 0;
        return 3;
    }

    record->actor = actor;
    record->value_04 = value;
    record->value_08 = value;
    record->next = *(GroundedEventRecord * volatile *)(runtime_globals + 0x85C);
    *(GroundedEventRecord * volatile *)(runtime_globals + 0x85C) = record;
    *record_out = record;
    runtime_globals[0x860] = 0;
    return 0;
}