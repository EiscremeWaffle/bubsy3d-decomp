#include "reset_entry_helpers.h"

extern void func_80042170(ResetTrackedNode *node);

void func_8004232C(int32_t tag) {
    ResetTrackedNode *node = g_reset_tracked_head;
    ProximityObject *reference = func_8005327C();

    if (node != 0) {
        int16_t wanted = (int16_t)tag;
        do {
            ResetTrackedNode *next = node->next;
            ResetTrackedObject *object = node->object;
            if (node->tag == wanted && object->reference != reference &&
                (object->flags & 0x200000u) == 0) {
                func_80042170(node);
            }
            node = next;
        } while (node != 0);
    }
}
