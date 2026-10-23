#include <stdint.h>

typedef struct GroundedEventRecord {
    void *actor;
    void *value_04;
    void *value_08;
    struct GroundedEventRecord *next;
} GroundedEventRecord;

extern void func_80082EFC(void *record);

int32_t func_8004FFEC(GroundedEventRecord *record) {
    register volatile uint8_t *runtime_globals __asm__("$28");
    GroundedEventRecord *previous = 0;
    GroundedEventRecord *current;

    if (record == 0) {
        return 1;
    }

    current = *(GroundedEventRecord * volatile *)(runtime_globals + 0x85C);
    runtime_globals[0x860] = 1;
    while (current != 0 && current != record) {
        previous = current;
        current = current->next;
    }

    if (current == 0) {
        runtime_globals[0x860] = 0;
        return 1;
    }
    if (previous == 0) {
        *(GroundedEventRecord * volatile *)(runtime_globals + 0x85C) = current->next;
    } else {
        previous->next = current->next;
    }
    func_80082EFC(current);
    runtime_globals[0x860] = 0;
    return 0;
}