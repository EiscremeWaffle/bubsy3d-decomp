#include <stdint.h>

typedef struct SequenceLimitEntry {
    int16_t key;
    int16_t limit;
    uint8_t padding[68];
} SequenceLimitEntry;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
register volatile uint8_t *func_8005D5A8_gp __asm__("$28");
#define FUNC_8005D5A8_GLOBALS func_8005D5A8_gp
#define FUNC_8005D5A8_SEQUENCE_LIMITS ((volatile SequenceLimitEntry *)(uintptr_t)0x801CCE14)
#else
extern volatile uint8_t g_l0_runtime_globals[];
extern volatile SequenceLimitEntry g_sequence_limit_entries[];
#define FUNC_8005D5A8_GLOBALS g_l0_runtime_globals
#define FUNC_8005D5A8_SEQUENCE_LIMITS g_sequence_limit_entries
#endif

int32_t func_8005D5A8(int16_t requested_key, int16_t value) {
    int32_t index;
    int16_t count;
    volatile SequenceLimitEntry *entry;

    if (FUNC_8005D5A8_GLOBALS[0x624] != 0) {
        return 0x1B;
    }

    count = *(volatile int16_t *)(FUNC_8005D5A8_GLOBALS + 0x62C);
    index = 0;
    while (index < count &&
           FUNC_8005D5A8_SEQUENCE_LIMITS[index].key != requested_key) {
        index++;
    }

    entry = &FUNC_8005D5A8_SEQUENCE_LIMITS[index];
    if (entry->key != requested_key) {
        return 1;
    }

    return value < entry->limit && value < 0;
}