#include "reset_entry_helpers.h"

int32_t func_80044A88(uint32_t object_index, ResetEntryGroup *excluded_group) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t active_input __asm__("$2") = g_reset_active_group_count;
    register uint32_t active_index __asm__("$8");
    register uint32_t active_count __asm__("$10");
    register uint32_t *active_ids __asm__("$9");
    register ResetEntryGroup *group_base __asm__("$11");
    uint8_t target_frame[16];

    /* Preserve load and loop-index scheduling without emitting instructions. */
    __asm__ volatile("" : "=r"(active_input) : "0"(active_input) : "$8");
#else
    uint32_t active_input = g_reset_active_group_count;
    uint32_t active_index;
    uint32_t active_count;
    uint32_t *active_ids;
    ResetEntryGroup *group_base;
#endif

    active_index = 0;
    if (active_input != 0) {
        group_base = g_reset_entry_groups;
        active_count = active_input;
        active_ids = g_reset_active_group_indices;
        do {
            ResetEntryGroup *group;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            group = (ResetEntryGroup *)(*active_ids * sizeof(*group_base) + (uintptr_t)group_base);
#else
            group = &group_base[*active_ids];
#endif
            if (group != excluded_group) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                register int32_t count_input __asm__("$3") = group->count;
                register int32_t entry __asm__("$6");
                register int32_t count __asm__("$7");
                register uint32_t *indices __asm__("$3");

                __asm__ volatile("" : "=r"(count_input) : "0"(count_input) : "$6");
#else
                int32_t count_input = group->count;
                int32_t entry;
                int32_t count;
                uint32_t *indices;
#endif
                entry = 0;
                if (count_input > 0) {
                    count = count_input;
                    indices = group->indices;
                    do {
                        if (*indices == object_index) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                            __asm__ volatile("");
                            __asm__ volatile("" : "=m"(target_frame));
#endif
                            return 1;
                        }
                        ++entry;
                        ++indices;
                    } while (entry < count);
                }
            }
            ++active_index;
            ++active_ids;
        } while (active_index < active_count);
    }
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=m"(target_frame));
#endif
    return 0;
}
