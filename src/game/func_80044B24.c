#include "reset_entry_helpers.h"

extern void *func_80028764(int32_t index);
extern int32_t func_80052BDC(void *object, int32_t mode);

int32_t func_80044B24(uint32_t index, uint32_t mode) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    uint8_t target_frame[8];
    register uint32_t offset __asm__("$2");
    register ResetEntryGroup *base __asm__("$3");
    register int32_t getter_arg __asm__("$4");
    register int32_t entry __asm__("$17");
    register ResetEntryGroup *group __asm__("$18");
    ResetSelectionActor *actor;
    register uint32_t mode_snapshot __asm__("$16") = mode;

    __asm__("" : "=r"(mode_snapshot) : "0"(mode_snapshot));
    offset = index << 4;
    base = g_reset_entry_groups;
    getter_arg = 0;
    entry = 0;
    /* Preserve the original saved-register and frame scheduling. */
    __asm__ volatile("" : : : "$19", "$20");
    group = (ResetEntryGroup *)(offset + (uintptr_t)base);
    actor = func_80028764(getter_arg);
#else
    int32_t entry = 0;
    ResetEntryGroup *group = &g_reset_entry_groups[index];
    ResetSelectionActor *actor = func_80028764(0);
#endif

    if (group->count > 0) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register uint32_t check_entry __asm__("$19") = (uint8_t)mode_snapshot;
        __asm__("" : "=r"(check_entry) : "0"(check_entry));
#else
        uint8_t check_entry = (uint8_t)mode;
#endif
        do {
            uint32_t object_index = group->indices[entry];
            void *object = g_reset_selected_objects[object_index];
            if (object != actor->object) {
                if (check_entry == 0 || (uint8_t)func_80044A88(object_index, group) == 0) {
                    func_80052BDC(object, 1);
                }
            }
            ++entry;
        } while (entry < group->count);
    }
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=m"(target_frame));
#endif
    return 1;
}
