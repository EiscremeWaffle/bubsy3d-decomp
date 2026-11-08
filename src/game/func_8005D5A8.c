#include "sequence_limit.h"

int32_t func_8005D5A8(int32_t requested_key, int32_t value) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t key_copy __asm__("$3") = requested_key;
    register int32_t index __asm__("$6");
    register int32_t count __asm__("$7");
    register int32_t loop_key __asm__("$8");
    register int32_t offset __asm__("$4");
    register int32_t shifted_key __asm__("$2");
    uint8_t target_frame[8];
#else
    int32_t key_copy = requested_key;
    int32_t index;
    int32_t count;
    int32_t loop_key;
    int32_t offset;
#endif
    int32_t result;

    if (g_sequence_limit_scan_enabled == 0) {
        result = 0x1B;
        goto done;
    }

    count = g_sequence_limit_count;
    index = 0;
    if (count > 0) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        /* Preserve the original sign-extension register order. */
        shifted_key = (int32_t)((uint32_t)requested_key << 16);
        __asm__("" : "=r"(shifted_key) : "0"(shifted_key));
        loop_key = shifted_key >> 16;
#else
        loop_key = (int16_t)requested_key;
#endif
        offset = 0;
        do {
            if (loop_key == ((SequenceLimitEntry *)((uint8_t *)g_sequence_limit_entries + offset))->key) {
                goto matched;
            }
            ++index;
            offset += sizeof(SequenceLimitEntry);
        } while (index < count);
    }
    if ((int16_t)key_copy != g_sequence_limit_entries[index].key) {
        result = 1;
        goto done;
    }
matched:
    if ((int16_t)value < g_sequence_limit_entries[index].limit) {
        if ((int16_t)value >= 0) {
            result = 0;
            goto done;
        }
    }
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("");
#endif
    result = 1;
done:
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=m"(target_frame));
#endif
    return result;
}