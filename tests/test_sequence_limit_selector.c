#include "sequence_limit.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

_Static_assert(sizeof(SequenceLimitEntry) == 72, "sequence-limit entry size");
_Static_assert(offsetof(SequenceLimitEntry, limit) == 2, "sequence-limit offset");

uint8_t g_sequence_limit_scan_enabled;
int16_t g_sequence_limit_count;
SequenceLimitEntry g_sequence_limit_entries[4];

static unsigned int initializer_calls;
static int32_t initializer_key, initializer_value;
static uint32_t random_state = 0x5D5A8u;

int32_t func_8008E1DC(int32_t key, int32_t value) {
    ++initializer_calls;
    initializer_key = key;
    initializer_value = value;
    return 123;
}

static uint32_t next_random(void) {
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}

static int32_t signed_word(uint32_t value) {
    int32_t magnitude = (int32_t)(value & 0x7FFFFFFFu);
    return (value & 0x80000000u) ? -1 - (INT32_MAX - magnitude) : magnitude;
}

static int16_t signed_halfword(uint32_t value) {
    int32_t narrowed = (int32_t)(value & 0xFFFFu);
    return (int16_t)(narrowed < 0x8000 ? narrowed : narrowed - 0x10000);
}

static int32_t expected_result(int32_t requested_key, int32_t value) {
    int32_t key = signed_halfword((uint32_t)requested_key);
    int32_t narrowed_value = signed_halfword((uint32_t)value);
    int32_t index = 0;

    if (g_sequence_limit_scan_enabled == 0) {
        return 0x1B;
    }
    while (index < g_sequence_limit_count && g_sequence_limit_entries[index].key != key) {
        ++index;
    }
    if (g_sequence_limit_entries[index].key != key) {
        return 1;
    }
    return !(narrowed_value >= 0 && narrowed_value < g_sequence_limit_entries[index].limit);
}

static void test_flags_and_stopping_index(void) {
    unsigned int flag;

    g_sequence_limit_count = 1;
    g_sequence_limit_entries[0].key = 7;
    g_sequence_limit_entries[0].limit = 3;
    for (flag = 0; flag < 256; ++flag) {
        g_sequence_limit_scan_enabled = (uint8_t)flag;
        assert(func_8005D5A8(0x10007, 0) == (flag == 0 ? 0x1B : 0));
    }
    g_sequence_limit_scan_enabled = 1;
    assert(func_8005D5A8(7, -1) == 1);
    assert(func_8005D5A8(7, 2) == 0);
    assert(func_8005D5A8(7, 3) == 1);
    g_sequence_limit_count = 0;
    assert(func_8005D5A8(7, 0) == 0);
    g_sequence_limit_count = INT16_MIN;
    assert(func_8005D5A8(7, 0) == 0);

    g_sequence_limit_count = 3;
    g_sequence_limit_entries[0].key = 1;
    g_sequence_limit_entries[1].key = 2;
    g_sequence_limit_entries[2].key = 3;
    g_sequence_limit_entries[3].key = 7;
    g_sequence_limit_entries[3].limit = 3;
    assert(func_8005D5A8(7, 2) == 0);
    assert(func_8005D5A8(8, 2) == 1);
    g_sequence_limit_entries[0].key = 7;
    g_sequence_limit_entries[0].limit = 0;
    assert(func_8005D5A8(7, 2) == 1);
}

static void test_randomized_inputs(void) {
    static const int16_t counts[] = {INT16_MIN, -1, 0, 1, 2, 3};
    static const int16_t boundaries[] = {INT16_MIN, -1, 0, 1, 2, 3, INT16_MAX};
    unsigned int sample;

    for (sample = 0; sample < 4096; ++sample) {
        uint32_t raw_key, raw_value;
        int32_t key, value;
        size_t index;
        g_sequence_limit_scan_enabled = (uint8_t)next_random();
        g_sequence_limit_count = counts[next_random() % (sizeof(counts) / sizeof(counts[0]))];
        for (index = 0; index < 4; ++index) {
            g_sequence_limit_entries[index].key = boundaries[next_random() % (sizeof(boundaries) / sizeof(boundaries[0]))];
            g_sequence_limit_entries[index].limit = boundaries[next_random() % (sizeof(boundaries) / sizeof(boundaries[0]))];
        }
        raw_key = next_random();
        if ((sample & 3u) != 0) {
            raw_key = (raw_key & 0xFFFF0000u) |
                (uint16_t)g_sequence_limit_entries[next_random() % 4].key;
        }
        raw_value = (next_random() & 0xFFFF0000u) |
            (uint16_t)boundaries[next_random() % (sizeof(boundaries) / sizeof(boundaries[0]))];
        key = signed_word(raw_key);
        value = signed_word(raw_value);
        assert(func_8005D5A8(key, value) == expected_result(key, value));
    }
}

static void test_real_wrapper(void) {
    g_sequence_limit_count = 1;
    g_sequence_limit_entries[0].key = 7;
    g_sequence_limit_entries[0].limit = 3;
    g_sequence_limit_scan_enabled = 0;
    initializer_calls = 0;
    assert(func_8005E7B8(0x10007, 0x10002) == 0x1B);
    assert(initializer_calls == 0);

    g_sequence_limit_scan_enabled = 1;
    assert(func_8005E7B8(0x10007, 0x10002) == 0);
    assert(initializer_calls == 1 && initializer_key == 7 && initializer_value == 2);
    assert(func_8005E7B8(0x10007, 0x10003) == 1);
    assert(func_8005E7B8(0x10007, -1) == 1);
    assert(initializer_calls == 1);
}

int main(void) {
    test_flags_and_stopping_index();
    test_randomized_inputs();
    test_real_wrapper();
    puts("sequence-limit selector and real-wrapper tests passed");
    return 0;
}
