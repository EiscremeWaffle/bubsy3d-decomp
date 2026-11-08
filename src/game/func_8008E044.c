#include <stdint.h>

#include "sequence_notification.h"

/* Behavioral candidate; not yet byte-matched. */
extern volatile uintptr_t g_character_state_table[];

int32_t func_8008E044(int32_t character_index, int32_t state_index) {
    volatile uint8_t *state;
    volatile uint8_t *current_state;
    int32_t state_offset;
    uint32_t selection;
    uint32_t source_word_7c, source_word_84, source_word_08_first, source_word_08_second;
    uint16_t source_halfword_72;
    int32_t index;

    character_index = (int16_t)character_index;
    state_index = (int16_t)state_index;
    state_offset = state_index * 172;
    state = (volatile uint8_t *)(uintptr_t)g_character_state_table[character_index] + state_offset;

    *(volatile uint32_t *)(state + 0x90) &= ~0x01u;
    current_state = (volatile uint8_t *)(uintptr_t)g_character_state_table[character_index] + state_offset;
    *(volatile uint32_t *)(current_state + 0x90) &= ~0x02u;
    current_state = (volatile uint8_t *)(uintptr_t)g_character_state_table[character_index] + state_offset;
    *(volatile uint32_t *)(current_state + 0x90) &= ~0x08u;
    current_state = (volatile uint8_t *)(uintptr_t)g_character_state_table[character_index] + state_offset;
    *(volatile uint32_t *)(current_state + 0x90) |= 0x04u;

    selection = ((uint32_t)state_index << 8) | (uint32_t)character_index;
    func_80094108(selection);

    state[0x2B] = 0;
    *(volatile uint32_t *)(state + 0x80) = 0;
    state[0x27] = 0;
    state[0x13] = 0;
    state[0x14] = 0;
    state[0x29] = 0;
    state[0x15] = 0;
    state[0x16] = 0;
    state[0x2A] = 0;
    state[0x12] = 0;
    *(volatile uint16_t *)(state + 0x48) = 0;
    state[0x27] = 0;
    state[0x28] = 0;
    state[0x10] = 0;
    state[0x11] = 0;

    source_word_7c = *(volatile uint32_t *)(state + 0x7C);
    source_word_84 = *(volatile uint32_t *)(state + 0x84);
    source_halfword_72 = *(volatile uint16_t *)(state + 0x72);
    source_word_08_first = *(volatile uint32_t *)(state + 0x08);
    source_word_08_second = *(volatile uint32_t *)(state + 0x08);
    *(volatile uint32_t *)(state + 0x88) = source_word_7c;
    *(volatile uint32_t *)(state + 0x8C) = source_word_84;
    *(volatile uint16_t *)(state + 0x70) = source_halfword_72;
    *(volatile uint32_t *)(state + 0x04) = source_word_08_first;
    *(volatile uint32_t *)(state + 0x0C) = source_word_08_second;

    for (index = 0; index < 16; index++) {
        state[0x2C + index] = (uint8_t)index;
        state[0x17 + index] = 0x40;
        *(volatile uint16_t *)(state + 0x4E + index * 2) = 0x7F;
    }

    *(volatile uint16_t *)(state + 0x78) = 0x7F;
    *(volatile uint16_t *)(state + 0x7A) = 0x7F;
    return 0x7F;
}