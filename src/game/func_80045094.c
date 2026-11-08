#include "reset_resources.h"
#include "reset_entry_helpers.h"

extern void func_8004380C(void);
extern uint32_t func_800439EC(void);
extern void func_80044268(void);
extern void func_80069758(void);

void func_80045094(void) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t index __asm__("$16") = 0;
    register RuntimeResetEntry *entry __asm__("$17");
    register int32_t clear_count __asm__("$4");
    register uint32_t *clear_word __asm__("$3");
    uint8_t target_frame[16];
#else
    uint32_t index = 0;
    RuntimeResetEntry *entry;
    int32_t clear_count;
    uint32_t *clear_word;
#endif

    g_reset_active = 1;
    func_8004380C();
    if (g_reset_entry_count != 0) {
        entry = g_reset_entries;
        do {
            func_80044458(entry->word, 0);
            func_8004232C(entry->halfwords.first);
            ++index;
            ++entry;
        } while (index < g_reset_entry_count);
    }

    clear_count = g_reset_clear_count;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    /* Reserve the original local frame without emitting instructions. */
    __asm__ volatile("" : "=m"(target_frame));
#endif
    index = 0;
    if (clear_count > 0) {
        clear_word = g_reset_clear_words;
        do {
            *clear_word++ = 0;
            ++index;
        } while ((int32_t)index < clear_count);
    }
    g_reset_secondary_flag = 0;
    func_800439EC();
    func_800439EC();
    func_80044268();
    func_80069758();
    g_reset_active = 0;
}
