#include "cyclic_entries.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

GameCyclicEntry g_cyclic_entries[7];

static void reset_entries(void) {
    unsigned row, word;

    for (row = 0; row < 7; row++) {
        for (word = 0; word < 4; word++) {
            g_cyclic_entries[row].words[word] = 0x12340000U + row * 16U + word;
        }
    }
}

static void test_invalid_indices(void) {
    static const int32_t invalid[] = {INT32_MIN, -1, 7, INT32_MAX};
    GameCyclicEntry output, before;
    unsigned i;

    memset(&before, 0xA5, sizeof(before));
    for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        int32_t index = invalid[i];
        output = before;
        assert(func_8001A280(&index, &output) == 0);
        assert(index == invalid[i]);
        assert(memcmp(&output, &before, sizeof(output)) == 0);
        assert(func_8001A280(&index, NULL) == 0);
    }
}

static void test_two_complete_cycles(void) {
    int32_t index = 0;
    unsigned call;

    reset_entries();
    for (call = 0; call < 14; call++) {
        GameCyclicEntry output;
        unsigned selected = call % 7;
        assert(func_8001A280(&index, &output) == 1);
        assert(memcmp(&output, &g_cyclic_entries[selected], sizeof(output)) == 0);
        assert(index == (int32_t)((selected + 1U) % 7U));
    }
}

static void test_output_aliases_table(void) {
    GameCyclicEntry selected;
    int32_t index;

    reset_entries();
    selected = g_cyclic_entries[2];
    index = 2;
    assert(func_8001A280(&index, &g_cyclic_entries[4]) == 1);
    assert(index == 3);
    assert(memcmp(&g_cyclic_entries[4], &selected, sizeof(selected)) == 0);
    assert(memcmp(&g_cyclic_entries[2], &selected, sizeof(selected)) == 0);

    index = 2;
    assert(func_8001A280(&index, &g_cyclic_entries[2]) == 1);
    assert(index == 3);
    assert(memcmp(&g_cyclic_entries[2], &selected, sizeof(selected)) == 0);
}

static void test_output_overwrites_index(void) {
    static const uint32_t replacement[] = {
        0, 5, 6, 7, UINT32_MAX, 0x80000000U, 0x7FFFFFFFU
    };
    union {
        GameCyclicEntry entry;
        int32_t index[4];
    } overlap;
    unsigned slot, i;

    for (slot = 0; slot < 4; slot++) {
        for (i = 0; i < sizeof(replacement) / sizeof(replacement[0]); i++) {
            GameCyclicEntry expected;
            uint32_t next = replacement[i] + 1U;
            reset_entries();
            g_cyclic_entries[2].words[slot] = replacement[i];
            expected = g_cyclic_entries[2];
            expected.words[slot] = next >= 7 && next < 0x80000000U ? 0 : next;
            memset(&overlap, 0, sizeof(overlap));
            overlap.index[slot] = 2;
            assert(func_8001A280(&overlap.index[slot], &overlap.entry) == 1);
            assert(memcmp(&overlap.entry, &expected, sizeof(expected)) == 0);
        }
    }
}

int main(void) {
    test_invalid_indices();
    test_two_complete_cycles();
    test_output_aliases_table();
    test_output_overwrites_index();
    puts("cyclic entry tests passed");
    return 0;
}
