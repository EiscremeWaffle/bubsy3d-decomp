#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
#define FUNC_8008E044_CHARACTER_TABLE ((volatile uint32_t *)0x801D7D44)
#else
extern volatile uint32_t g_character_state_table[];
#define FUNC_8008E044_CHARACTER_TABLE g_character_state_table
#endif

extern int32_t func_80094108(uint32_t selection);

int32_t func_8008E044(int32_t character_index, int32_t state_index) {
    uint8_t *state;
    uint32_t selection;
    int32_t index;

    character_index = (int16_t)character_index;
    state_index = (int16_t)state_index;
    state = (uint8_t *)(uintptr_t)FUNC_8008E044_CHARACTER_TABLE[character_index] +
            state_index * 44;

    *(volatile uint32_t *)(state + 0x90) &= ~0x02u;
    *(volatile uint32_t *)(state + 44 + 0x90) &= ~0x04u;
    *(volatile uint32_t *)(state + 88 + 0x90) &= ~0x08u;
    *(volatile uint32_t *)(state + 132 + 0x90) |= 0x04u;

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

    *(volatile uint32_t *)(state + 0x88) = *(volatile uint32_t *)(state + 0x7C);
    *(volatile uint32_t *)(state + 0x8C) = *(volatile uint32_t *)(state + 0x84);
    *(volatile uint16_t *)(state + 0x70) = *(volatile uint16_t *)(state + 0x72);
    *(volatile uint32_t *)(state + 0x04) = *(volatile uint32_t *)(state + 0x08);
    *(volatile uint32_t *)(state + 0x0C) = *(volatile uint32_t *)(state + 0x08);

    for (index = 0; index < 16; index++) {
        state[0x2C + index] = 0;
        state[0x17 + index] = 0x40;
        *(volatile uint16_t *)(state + 0x4E + index * 2) = 0x7F;
    }

    *(volatile uint16_t *)(state + 0x78) = 0x7F;
    *(volatile uint16_t *)(state + 0x7A) = 0x7F;
    return 0x7F;
}