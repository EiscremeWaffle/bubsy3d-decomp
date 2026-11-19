#include "game_transition.h"

extern GameTransitionQueue g_transition_queue_input;
extern GameTransitionQueue g_transition_queue_snapshot;

void func_8001B9C8(void) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t slot __asm__("$4") = 0;
    register uintptr_t offset __asm__("$3") = 0;
#else
    int32_t slot = 0;
    uintptr_t offset = 0;
#endif

    /* A separate byte offset preserves the original 28-byte record scan. */
    do {
        ((GameTransitionQueueRecord *)&((uint8_t *)g_transition_queue_input.records)[offset])->event.event_id = 0;
        ((GameTransitionQueueRecord *)&((uint8_t *)g_transition_queue_snapshot.records)[offset])->event.event_id = 0;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(slot) : "0"(slot) : "memory");
#endif
        slot++;
        offset += sizeof(GameTransitionQueueRecord);
    } while (slot < 100);
    g_transition_queue_input.count = 0;
    g_transition_queue_snapshot.count = 0;
}
