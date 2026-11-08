#include "reset_resources.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

_Static_assert(sizeof(RuntimeResetEntry) == 4, "reset entry size");
_Static_assert(offsetof(RuntimeResetEntry, halfwords.first) == 0, "first halfword offset");

uint8_t g_reset_active;
uint8_t g_reset_secondary_flag;
uint32_t g_reset_entry_count;
RuntimeResetEntry g_reset_entries[4];
int32_t g_reset_clear_count;
uint32_t g_reset_clear_words[6];

enum CallKind { SETUP, ENTRY_WORD, ENTRY_HALF, TICK, FINALIZE, REFRESH };
typedef struct Call {
    enum CallKind kind;
    uint32_t value;
} Call;

static Call calls[16];
static size_t call_count;
static int mutation;
static int entry_calls;

static void record(enum CallKind kind, uint32_t value) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    assert(g_reset_active == 1);
    calls[call_count].kind = kind;
    calls[call_count].value = value;
    ++call_count;
}

void func_8004380C(void) {
    record(SETUP, 0);
    if (mutation == 1) {
        g_reset_entry_count = 2;
        g_reset_clear_count = 2;
    }
}

uint8_t func_80044458(uint32_t value, uint32_t mode) {
    record(ENTRY_WORD, value);
    assert(mode == 0);
    if (mutation == 1 && entry_calls == 0) {
        g_reset_entries[0].word = 0x1234FF80u;
        g_reset_entry_count = 3;
    }
    ++entry_calls;
    return 0;
}

void func_8004232C(int32_t value) {
    record(ENTRY_HALF, (uint32_t)value);
    if (mutation == 2 && entry_calls == 1) {
        g_reset_entry_count = 1;
    }
}

uint32_t func_800439EC(void) {
    assert(g_reset_secondary_flag == 0);
    record(TICK, 0);
    return 0xFFFFFFFFu;
}

void func_80044268(void) {
    assert(g_reset_secondary_flag == 0);
    record(FINALIZE, 0);
}

void func_80069758(void) {
    assert(g_reset_secondary_flag == 0);
    record(REFRESH, 0);
}

static void initialize(uint32_t count, int32_t clear_count, int mutation_mode) {
    static const uint32_t words[] = {0x1234FFFFu, 0xF00D8000u, 0xC0FF007Fu, 0x87654321u};
    size_t index;
    for (index = 0; index < sizeof(words) / sizeof(words[0]); ++index) {
        g_reset_entries[index].word = words[index];
    }
    for (index = 0; index < sizeof(g_reset_clear_words) / sizeof(g_reset_clear_words[0]); ++index) {
        g_reset_clear_words[index] = 0xA5A50000u + (uint32_t)index;
    }
    g_reset_active = 0x5A;
    g_reset_secondary_flag = 0xA5;
    g_reset_entry_count = count;
    g_reset_clear_count = clear_count;
    mutation = mutation_mode;
    entry_calls = 0;
    call_count = 0;
}

static void expect_call(size_t *next, enum CallKind kind, uint32_t value) {
    assert(*next < call_count);
    assert(calls[*next].kind == kind && calls[*next].value == value);
    ++*next;
}

static void expect_tail(size_t *next, int32_t clear_count) {
    size_t index;
    expect_call(next, TICK, 0);
    expect_call(next, TICK, 0);
    expect_call(next, FINALIZE, 0);
    expect_call(next, REFRESH, 0);
    assert(call_count == *next);
    assert(g_reset_active == 0 && g_reset_secondary_flag == 0);
    for (index = 0; index < sizeof(g_reset_clear_words) / sizeof(g_reset_clear_words[0]); ++index) {
        uint32_t expected = 0xA5A50000u + (uint32_t)index;
        if (clear_count > 0 && index < (size_t)clear_count) {
            expected = 0;
        }
        assert(g_reset_clear_words[index] == expected);
    }
}

static void test_regular_resets(void) {
    static const uint32_t counts[] = {0, 1, 3};
    static const int32_t clear_counts[] = {INT32_MIN, -1, 0, 1, 4};
    static const uint32_t words[] = {0x1234FFFFu, 0xF00D8000u, 0xC0FF007Fu};
    static const uint32_t halves[] = {0xFFFFFFFFu, 0xFFFF8000u, 0x7Fu};
    size_t count_index, clear_index;

    for (count_index = 0; count_index < sizeof(counts) / sizeof(counts[0]); ++count_index) {
        for (clear_index = 0; clear_index < sizeof(clear_counts) / sizeof(clear_counts[0]); ++clear_index) {
            size_t next = 0;
            uint32_t index;
            initialize(counts[count_index], clear_counts[clear_index], 0);
            func_80045094();
            expect_call(&next, SETUP, 0);
            for (index = 0; index < counts[count_index]; ++index) {
                expect_call(&next, ENTRY_WORD, words[index]);
                expect_call(&next, ENTRY_HALF, halves[index]);
            }
            expect_tail(&next, clear_counts[clear_index]);
        }
    }
}

static void test_live_mutations(void) {
    size_t next = 0;
    initialize(1, 0, 1);
    func_80045094();
    expect_call(&next, SETUP, 0);
    expect_call(&next, ENTRY_WORD, 0x1234FFFFu);
    expect_call(&next, ENTRY_HALF, 0xFFFFFF80u);
    expect_call(&next, ENTRY_WORD, 0xF00D8000u);
    expect_call(&next, ENTRY_HALF, 0xFFFF8000u);
    expect_call(&next, ENTRY_WORD, 0xC0FF007Fu);
    expect_call(&next, ENTRY_HALF, 0x7Fu);
    assert(g_reset_entry_count == 3);
    expect_tail(&next, 2);

    next = 0;
    initialize(3, 0, 2);
    func_80045094();
    expect_call(&next, SETUP, 0);
    expect_call(&next, ENTRY_WORD, 0x1234FFFFu);
    expect_call(&next, ENTRY_HALF, 0xFFFFFFFFu);
    assert(g_reset_entry_count == 1);
    expect_tail(&next, 0);
}

int main(void) {
    test_regular_resets();
    test_live_mutations();
    puts("reset-resource backend tests passed");
    return 0;
}
